#pragma once
#include "CAF.hpp"
#include "caf/all.hpp"
namespace ok {
struct mainActorState {};
main_actor_int::behavior_type
mainActor(main_actor_int::stateful_pointer<mainActorState> self);
} // namespace ok
