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

#pragma once

#include <functional>
#include <string>
#include <utility>

#include "caf/abstract_group.hpp"
#include "caf/detail/comparable.hpp"
#include "caf/detail/core_export.hpp"
#include "caf/detail/type_traits.hpp"
#include "caf/fwd.hpp"
#include "caf/group_module.hpp"
#include "caf/intrusive_ptr.hpp"
#include "caf/none.hpp"

namespace caf {

struct invalid_group_t {
  constexpr invalid_group_t() = default;
};

/// Identifies an invalid {@link group}.
/// @relates group
constexpr invalid_group_t invalid_group = invalid_group_t{};

class CAF_CORE_EXPORT group : detail::comparable<group>,
                              detail::comparable<group, invalid_group_t> {
public:
  using signatures = none_t;

  group() = default;

  group(group&&) = default;

  group(const group&) = default;

  group(const invalid_group_t&);

  group& operator=(group&&) = default;

  group& operator=(const group&) = default;

  group& operator=(const invalid_group_t&);

  group(abstract_group*);

  group(intrusive_ptr<abstract_group> gptr);

  explicit operator bool() const noexcept {
    return static_cast<bool>(ptr_);
  }

  bool operator!() const noexcept {
    return !ptr_;
  }

  static intptr_t compare(const abstract_group* lhs, const abstract_group* rhs);

  intptr_t compare(const group& other) const noexcept;

  intptr_t compare(const invalid_group_t&) const noexcept {
    return ptr_ ? 1 : 0;
  }

  abstract_group* get() const noexcept {
    return ptr_.get();
  }

  /// @cond PRIVATE

  actor_system& system() const {
    return ptr_->system();
  }

  template <class... Ts>
  void eq_impl(message_id mid, strong_actor_ptr sender, execution_unit* ctx,
               Ts&&... xs) const {
    CAF_ASSERT(!mid.is_request());
    if (ptr_)
      ptr_->enqueue(std::move(sender), mid,
                    make_message(std::forward<Ts>(xs)...), ctx);
  }

  bool subscribe(strong_actor_ptr who) const {
    if (!ptr_)
      return false;
    return ptr_->subscribe(std::move(who));
  }

  void unsubscribe(const actor_control_block* who) const {
    if (ptr_)
      ptr_->unsubscribe(who);
  }

  const group* operator->() const noexcept {
    return this;
  }

  template <class Inspector>
  friend bool inspect(Inspector& f, group& x) {
    std::string module_name;
    std::string group_name;
    actor dispatcher;
    if constexpr (!Inspector::is_loading) {
      if (x) {
        module_name = x.get()->module().name();
        group_name = x.get()->identifier();
        dispatcher = x.get()->dispatcher();
      }
    }
    auto load_cb = [&] {
      if constexpr (detail::has_context<Inspector>::value) {
        auto ctx = f.context();
        if (ctx != nullptr) {
          if (auto grp = ctx->system().groups().get(module_name, group_name,
                                                    dispatcher)) {
            x = std::move(*grp);
            return true;
          } else {
            f.set_error(std::move(grp.error()));
            return false;
          }
        }
      }
      f.emplace_error(sec::no_context);
      return false;
    };
    return f.object(x)
      .on_load(load_cb) //
      .fields(f.field("module_name", module_name),
              f.field("group_name", group_name),
              f.field("dispatcher", dispatcher));
  }

  /// @endcond

private:
  abstract_group* release() noexcept {
    return ptr_.release();
  }

  group(abstract_group*, bool);

  abstract_group_ptr ptr_;
};

/// @relates group
CAF_CORE_EXPORT std::string to_string(const group& x);

} // namespace caf

namespace std {

template <>
struct hash<caf::group> {
  size_t operator()(const caf::group& x) const {
    // Groups are singleton objects. Hence, we can simply use the address as
    // hash value.
    return !x ? 0 : reinterpret_cast<size_t>(x.get());
  }
};

} // namespace std
