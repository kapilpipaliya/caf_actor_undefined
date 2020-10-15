/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright 2011-2018 Dominik Charousset                                     *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include <condition_variable>
#include <mutex>
#include <set>
#include <sstream>
#include <stdexcept>

#include "caf/all.hpp"
#include "caf/deserializer.hpp"
#include "caf/event_based_actor.hpp"
#include "caf/group.hpp"
#include "caf/group_manager.hpp"
#include "caf/locks.hpp"
#include "caf/message.hpp"
#include "caf/sec.hpp"
#include "caf/serializer.hpp"

namespace caf {

namespace {

using exclusive_guard = unique_lock<detail::shared_spinlock>;
using shared_guard = shared_lock<detail::shared_spinlock>;
using upgrade_guard = upgrade_lock<detail::shared_spinlock>;
using upgrade_to_unique_guard = upgrade_to_unique_lock<detail::shared_spinlock>;

class local_dispatcher;
class local_group_module;

void await_all_locals_down(actor_system& sys, std::initializer_list<actor> xs) {
  CAF_LOG_TRACE("");
  scoped_actor self{sys, true};
  std::vector<actor> ys;
  for (auto& x : xs)
    if (x.node() == sys.node()) {
      self->send_exit(x, exit_reason::kill);
      ys.push_back(x);
    }
  // Don't block when using the test coordinator.
  if (auto policy = get_if<std::string>(&sys.config(), "caf.scheduler.policy");
      policy && *policy != "testing")
    self->wait_for(ys);
}

class local_group : public abstract_group {
public:
  void send_all_subscribers(const strong_actor_ptr& sender, const message& msg,
                            execution_unit* host) {
    CAF_LOG_TRACE(CAF_ARG(sender) << CAF_ARG(msg));
    shared_guard guard(mtx_);
    for (auto& s : subscribers_)
      s->enqueue(sender, make_message_id(), msg, host);
  }

  void enqueue(strong_actor_ptr sender, message_id, message msg,
               execution_unit* host) override {
    CAF_LOG_TRACE(CAF_ARG(sender) << CAF_ARG(msg));
    send_all_subscribers(sender, msg, host);
    dispatcher_->enqueue(sender, make_message_id(), msg, host);
  }

  std::pair<bool, size_t> add_subscriber(strong_actor_ptr who) {
    CAF_LOG_TRACE(CAF_ARG(who));
    if (!who)
      return {false, subscribers_.size()};
    exclusive_guard guard(mtx_);
    auto res = subscribers_.emplace(std::move(who)).second;
    return {res, subscribers_.size()};
  }

  std::pair<bool, size_t> erase_subscriber(const actor_control_block* who) {
    CAF_LOG_TRACE(""); // serializing who would cause a deadlock
    exclusive_guard guard(mtx_);
    auto e = subscribers_.end();
    auto cmp = [&](const strong_actor_ptr& lhs) { return lhs.get() == who; };
    auto i = std::find_if(subscribers_.begin(), e, cmp);
    if (i == e)
      return {false, subscribers_.size()};
    subscribers_.erase(i);
    return {true, subscribers_.size()};
  }

  bool subscribe(strong_actor_ptr who) override {
    CAF_LOG_TRACE(CAF_ARG(who));
    return add_subscriber(std::move(who)).first;
  }

  void unsubscribe(const actor_control_block* who) override {
    CAF_LOG_TRACE(CAF_ARG(who));
    erase_subscriber(who);
  }

  void stop() override {
    CAF_LOG_TRACE("");
    await_all_locals_down(system(), {dispatcher_});
  }

  local_group(local_group_module& mod, std::string id, node_id nid,
              optional<actor> lb);

  ~local_group() override;

protected:
  detail::shared_spinlock mtx_;
#if __cplusplus > 201103L
  std::set<strong_actor_ptr, std::less<>> subscribers_;
#else
  std::set<strong_actor_ptr> subscribers_;
#endif
  actor dispatcher_;
};

using local_group_ptr = intrusive_ptr<local_group>;

class local_dispatcher : public event_based_actor {
public:
  explicit local_dispatcher(actor_config& cfg, local_group_ptr g)
    : event_based_actor(cfg), group_(std::move(g)) {
    // nop
  }

  void on_exit() override {
    acquaintances_.clear();
    group_.reset();
  }

  const char* name() const override {
    return "local_dispatcher";
  }

  behavior make_behavior() override {
    CAF_LOG_TRACE("");
    // instead of dropping "unexpected" messages,
    // we simply forward them to our acquaintances
    auto fwd = [=](scheduled_actor*, message& msg) -> skippable_result {
      send_to_acquaintances(std::move(msg));
      return delegated<message>{};
    };
    set_default_handler(fwd);
    set_down_handler([=](down_msg& dm) {
      CAF_LOG_TRACE(CAF_ARG(dm));
      auto first = acquaintances_.begin();
      auto last = acquaintances_.end();
      auto i = std::find_if(first, last,
                            [&](const actor& a) { return a == dm.source; });
      if (i != last)
        acquaintances_.erase(i);
    });
    // return behavior
    return {[=](join_atom, const actor& other) {
              CAF_LOG_TRACE(CAF_ARG(other));
              if (acquaintances_.insert(other).second) {
                monitor(other);
              }
            },
            [=](leave_atom, const actor& other) {
              CAF_LOG_TRACE(CAF_ARG(other));
              acquaintances_.erase(other);
              if (acquaintances_.erase(other) > 0)
                demonitor(other);
            },
            [=](forward_atom, const message& what) {
              CAF_LOG_TRACE(CAF_ARG(what));
              // local forwarding
              group_->send_all_subscribers(current_element_->sender, what,
                                           context());
              // forward to all acquaintances
              send_to_acquaintances(what);
            }};
  }

private:
  void send_to_acquaintances(const message& what) {
    // send to all remote subscribers
    auto src = current_element_->sender;
    CAF_LOG_DEBUG(CAF_ARG(acquaintances_.size())
                  << CAF_ARG(src) << CAF_ARG(what));
    for (auto& acquaintance : acquaintances_)
      acquaintance->enqueue(src, make_message_id(), what, context());
  }

  local_group_ptr group_;
  std::set<actor> acquaintances_;
};

// Send a join message to the original group if a proxy
// has local subscriptions and a "LEAVE" message to the original group
// if there's no subscription left.

class local_group_proxy;

using local_group_proxy_ptr = intrusive_ptr<local_group_proxy>;

class proxy_dispatcher : public event_based_actor {
public:
  proxy_dispatcher(actor_config& cfg, local_group_proxy_ptr grp)
    : event_based_actor(cfg), group_(std::move(grp)) {
    CAF_LOG_TRACE("");
  }

  behavior make_behavior() override;

  void on_exit() override {
    group_.reset();
  }

private:
  local_group_proxy_ptr group_;
};

class local_group_proxy : public local_group {
public:
  local_group_proxy(actor_system& sys, actor remote_dispatcher,
                    local_group_module& mod, std::string id, node_id nid)
    : local_group(mod, std::move(id), std::move(nid),
                  std::move(remote_dispatcher)),
      proxy_dispatcher_{sys.spawn<proxy_dispatcher, hidden>(this)},
      monitor_{sys.spawn<hidden>(dispatcher_monitor_actor, this)} {
    // nop
  }

  bool subscribe(strong_actor_ptr who) override {
    CAF_LOG_TRACE(CAF_ARG(who));
    auto res = add_subscriber(std::move(who));
    if (res.first) {
      // join remote source
      if (res.second == 1)
        anon_send(dispatcher_, join_atom_v, proxy_dispatcher_);
      return true;
    }
    CAF_LOG_WARNING("actor already joined group");
    return false;
  }

  void unsubscribe(const actor_control_block* who) override {
    CAF_LOG_TRACE(CAF_ARG(who));
    auto res = erase_subscriber(who);
    if (res.first && res.second == 0) {
      // leave the remote source,
      // because there's no more subscriber on this node
      anon_send(dispatcher_, leave_atom_v, proxy_dispatcher_);
    }
  }

  void enqueue(strong_actor_ptr sender, message_id mid, message msg,
               execution_unit* eu) override {
    CAF_LOG_TRACE(CAF_ARG(sender) << CAF_ARG(mid) << CAF_ARG(msg));
    // forward message to the dispatcher
    dispatcher_->enqueue(std::move(sender), mid,
                         make_message(forward_atom_v, std::move(msg)), eu);
  }

  void stop() override {
    CAF_LOG_TRACE("");
    await_all_locals_down(system_, {monitor_, proxy_dispatcher_, dispatcher_});
  }

private:
  static behavior dispatcher_monitor_actor(event_based_actor* self,
                                           local_group_proxy* grp) {
    CAF_LOG_TRACE("");
    self->monitor(grp->dispatcher_);
    self->set_down_handler([=](down_msg& down) {
      CAF_LOG_TRACE(CAF_ARG(down));
      auto msg = make_message(group_down_msg{group(grp)});
      grp->send_all_subscribers(self->ctrl(), std::move(msg), self->context());
      self->quit(down.reason);
    });
    return {[] {
      // nop
    }};
  }

  actor proxy_dispatcher_;
  actor monitor_;
};

behavior proxy_dispatcher::make_behavior() {
  CAF_LOG_TRACE("");
  // instead of dropping "unexpected" messages,
  // we simply forward them to our acquaintances
  auto fwd = [=](local_actor*, message& msg) -> skippable_result {
    group_->send_all_subscribers(current_element_->sender, std::move(msg),
                                 context());
    return message{};
  };
  set_default_handler(fwd);
  // return dummy behavior
  return {[] {
    // nop
  }};
}

class local_group_module : public group_module {
public:
  local_group_module(actor_system& sys) : group_module(sys, "local") {
    CAF_LOG_TRACE("");
  }

  expected<group> get(const std::string& identifier) override {
    CAF_LOG_TRACE(CAF_ARG(identifier));
    upgrade_guard guard(instances_mtx_);
    auto i = instances_.find(identifier);
    if (i != instances_.end())
      return group{i->second};
    auto tmp = make_counted<local_group>(*this, identifier, system().node(),
                                         none);
    upgrade_to_unique_guard uguard(guard);
    auto p = instances_.emplace(identifier, tmp);
    auto result = p.first->second;
    uguard.unlock();
    // someone might preempt us
    if (result != tmp)
      tmp->stop();
    return group{result};
  }

  expected<group> get(const std::string& identifier,
                      const caf::actor& dispatcher) override {
    CAF_LOG_DEBUG(CAF_ARG(identifier) << CAF_ARG(dispatcher));
    if (!dispatcher)
      return make_error(sec::invalid_argument, "dispatcher == nullptr");
    if (dispatcher->node() == system().node())
      return get(identifier);
    upgrade_guard guard(proxies_mtx_);
    auto i = proxies_.find(dispatcher);
    if (i != proxies_.end())
      return group{i->second};
    local_group_ptr tmp = make_counted<local_group_proxy>(system(), dispatcher,
                                                          *this, identifier,
                                                          dispatcher->node());
    upgrade_to_unique_guard uguard(guard);
    auto p = proxies_.emplace(dispatcher, tmp);
    // someone might preempt us
    return group{p.first->second};
  }

  void stop() override {
    CAF_LOG_TRACE("");
    std::map<std::string, local_group_ptr> imap;
    std::map<actor, local_group_ptr> pmap;
    { // critical section
      exclusive_guard guard1{instances_mtx_};
      exclusive_guard guard2{proxies_mtx_};
      imap.swap(instances_);
      pmap.swap(proxies_);
    }
    for (auto& kvp : imap)
      kvp.second->stop();
    for (auto& kvp : pmap)
      kvp.second->stop();
  }

private:
  detail::shared_spinlock instances_mtx_;
  std::map<std::string, local_group_ptr> instances_;
  detail::shared_spinlock proxies_mtx_;
  std::map<actor, local_group_ptr> proxies_;
};

local_group::local_group(local_group_module& mod, std::string id, node_id nid,
                         optional<actor> lb)
  : abstract_group(mod, std::move(id), std::move(nid)) {
  CAF_LOG_TRACE(CAF_ARG(id) << CAF_ARG(nid));
  dispatcher_ = lb ? *lb : mod.system().spawn<local_dispatcher, hidden>(this);
}

local_group::~local_group() {
  // nop
}

std::atomic<size_t> s_ad_hoc_id;

} // namespace

void group_manager::init(actor_system_config& cfg) {
  CAF_LOG_TRACE("");
  using ptr_type = std::unique_ptr<group_module>;
  mmap_.emplace("local", ptr_type{new local_group_module(system_)});
  for (auto& fac : cfg.group_module_factories) {
    ptr_type ptr{fac()};
    std::string name = ptr->name();
    mmap_.emplace(std::move(name), std::move(ptr));
  }
}

void group_manager::start() {
  CAF_LOG_TRACE("");
}

void group_manager::stop() {
  CAF_LOG_TRACE("");
  for (auto& kvp : mmap_)
    kvp.second->stop();
}

group_manager::~group_manager() {
  // nop
}

group_manager::group_manager(actor_system& sys) : system_(sys) {
  // nop
}

group group_manager::anonymous() const {
  CAF_LOG_TRACE("");
  std::string id = "__#";
  id += std::to_string(++s_ad_hoc_id);
  // local module is guaranteed to not return an error
  return *get_module("local")->get(id);
}

expected<group> group_manager::get(std::string group_uri) const {
  CAF_LOG_TRACE(CAF_ARG(group_uri));
  // URI parsing is pretty much a brute-force approach, no actual validation yet
  auto p = group_uri.find(':');
  if (p == std::string::npos)
    return sec::invalid_argument;
  auto group_id = group_uri.substr(p + 1);
  // erase all but the scheme part from the URI and use that as module name
  group_uri.erase(p);
  return get(group_uri, group_id);
}

expected<group> group_manager::get(const std::string& module_name,
                                   const std::string& group_identifier) const {
  CAF_LOG_TRACE(CAF_ARG(module_name) << CAF_ARG(group_identifier));
  if (auto mod = get_module(module_name))
    return mod->get(group_identifier);
  std::string error_msg = R"(no module named ")";
  error_msg += module_name;
  error_msg += R"(" found)";
  return make_error(sec::no_such_group_module, std::move(error_msg));
}

expected<group> group_manager::get(const std::string& module_name,
                                   const std::string& group_identifier,
                                   const actor& dispatcher) const {
  CAF_LOG_TRACE(CAF_ARG(module_name) << CAF_ARG(group_identifier));
  if (auto mod = get_module(module_name))
    return mod->get(group_identifier, dispatcher);
  std::string error_msg = R"(no module named ")";
  error_msg += module_name;
  error_msg += R"(" found)";
  return make_error(sec::no_such_group_module, std::move(error_msg));
}

optional<group_module&> group_manager::get_module(const std::string& x) const {
  auto i = mmap_.find(x);
  if (i != mmap_.end())
    return *(i->second);
  return none;
}

group group_manager::get_local(const std::string& group_identifier) const {
  // guaranteed to never return an error
  return *get("local", group_identifier);
}

} // namespace caf
