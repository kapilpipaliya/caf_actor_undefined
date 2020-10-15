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

#define CAF_SUITE stateful_actor

#include "caf/stateful_actor.hpp"

#include "core-test.hpp"

#include "caf/event_based_actor.hpp"

using namespace caf;

using namespace std::string_literals;

namespace {

using typed_adder_actor
  = typed_actor<reacts_to<add_atom, int>, replies_to<get_atom>::with<int>>;

struct counter {
  int value = 0;
};

behavior adder(stateful_actor<counter>* self) {
  return {
    [=](add_atom, int x) { self->state.value += x; },
    [=](get_atom) { return self->state.value; },
  };
}

class adder_class : public stateful_actor<counter> {
public:
  adder_class(actor_config& cfg) : stateful_actor<counter>(cfg) {
    // nop
  }

  behavior make_behavior() override {
    return adder(this);
  }
};

typed_adder_actor::behavior_type
typed_adder(typed_adder_actor::stateful_pointer<counter> self) {
  return {
    [=](add_atom, int x) { self->state.value += x; },
    [=](get_atom) { return self->state.value; },
  };
}

class typed_adder_class : public typed_adder_actor::stateful_base<counter> {
public:
  using super = typed_adder_actor::stateful_base<counter>;

  typed_adder_class(actor_config& cfg) : super(cfg) {
    // nop
  }

  behavior_type make_behavior() override {
    return typed_adder(this);
  }
};

struct fixture : test_coordinator_fixture<> {
  fixture() {
    // nop
  }

  template <class ActorUnderTest>
  void test_adder(ActorUnderTest aut) {
    inject((add_atom, int), from(self).to(aut).with(add_atom_v, 7));
    inject((add_atom, int), from(self).to(aut).with(add_atom_v, 4));
    inject((add_atom, int), from(self).to(aut).with(add_atom_v, 9));
    inject((get_atom), from(self).to(aut).with(get_atom_v));
    expect((int), from(aut).to(self).with(20));
  }

  template <class State>
  void test_name(const char* expected) {
    auto aut = sys.spawn([](stateful_actor<State>* self) -> behavior {
      return {
        [=](get_atom) {
          self->quit();
          return self->name();
        },
      };
    });
    inject((get_atom), from(self).to(aut).with(get_atom_v));
    expect((std::string), from(aut).to(self).with(expected));
  }
};

} // namespace

CAF_TEST_FIXTURE_SCOPE(dynamic_stateful_actor_tests, fixture)

CAF_TEST(stateful actors can be dynamically typed) {
  test_adder(sys.spawn(adder));
  test_adder(sys.spawn<typed_adder_class>());
}

CAF_TEST(stateful actors can be statically typed) {
  test_adder(sys.spawn(typed_adder));
  test_adder(sys.spawn<adder_class>());
}

CAF_TEST(stateful actors without explicit name use the name of the parent) {
  struct state {
    // empty
  };
  test_name<state>("user.scheduled-actor");
}

namespace {

struct named_state {
  static inline const char* name = "testee";
};

} // namespace

CAF_TEST(states with static C string names override the default name) {
  test_name<named_state>("testee");
}

CAF_TEST(states can accept constructor arguments and provide a behavior) {
  struct state_type {
    int x;
    int y;
    state_type(int x, int y) : x(x), y(y) {
      // nop
    }
    behavior make_behavior() {
      return {
        [=](int x, int y) {
          this->x = x;
          this->y = y;
        },
      };
    }
  };
  using actor_type = stateful_actor<state_type>;
  auto testee = sys.spawn<actor_type>(10, 20);
  auto& state = deref<actor_type>(testee).state;
  CAF_CHECK_EQUAL(state.x, 10);
  CAF_CHECK_EQUAL(state.y, 20);
  inject((int, int), to(testee).with(1, 2));
  CAF_CHECK_EQUAL(state.x, 1);
  CAF_CHECK_EQUAL(state.y, 2);
}

CAF_TEST(states optionally take the self pointer as first argument) {
  struct state_type : named_state {
    event_based_actor* self;
    int x;
    state_type(event_based_actor* self, int x) : self(self), x(x) {
      // nop
    }
    behavior make_behavior() {
      return {
        [=](get_atom) { return self->name(); },
      };
    }
  };
  using actor_type = stateful_actor<state_type>;
  auto testee = sys.spawn<actor_type>(10);
  auto& state = deref<actor_type>(testee).state;
  CAF_CHECK(state.self == &deref<actor_type>(testee));
  CAF_CHECK_EQUAL(state.x, 10);
  inject((get_atom), from(self).to(testee).with(get_atom_v));
  expect((std::string), from(testee).to(self).with("testee"s));
}

CAF_TEST(typed actors can use typed_actor_pointer as self pointer) {
  struct state_type : named_state {
    using self_pointer = typed_adder_actor::pointer_view;
    self_pointer self;
    int value;
    state_type(self_pointer self, int x) : self(self), value(x) {
      // nop
    }
    auto make_behavior() {
      return make_typed_behavior([=](add_atom, int x) { value += x; },
                                 [=](get_atom) { return value; });
    }
  };
  using actor_type = typed_adder_actor::stateful_base<state_type>;
  auto testee = sys.spawn<actor_type>(10);
  auto& state = deref<actor_type>(testee).state;
  CAF_CHECK(state.self == &deref<actor_type>(testee));
  CAF_CHECK_EQUAL(state.value, 10);
  inject((add_atom, int), from(self).to(testee).with(add_atom_v, 1));
  inject((get_atom), from(self).to(testee).with(get_atom_v));
  expect((int), from(testee).to(self).with(11));
}

CAF_TEST_FIXTURE_SCOPE_END()
