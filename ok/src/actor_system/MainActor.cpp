#include "MainActor.hpp"
#include <chrono>
namespace ok {
main_actor_int::behavior_type
mainActor(main_actor_int::stateful_pointer<mainActorState> self) {
  return {
      [=](spawn_and_monitor_atom) {},
      [=](shutdown_atom) {},
  };
}
} // namespace ok
