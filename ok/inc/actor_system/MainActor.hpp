#pragma once
#include "caf/all.hpp"
#include "CAF.hpp"
namespace ok::actr
{
namespace supervisor
{
struct subscribeActor
{
};
struct mainActorState
{
  static inline constexpr char const *name = "main-actor";
  caf::actor apiCount;
  std::vector<subscribeActor> subscribed_actors;
};
main_actor_int::behavior_type MainActor(main_actor_int::stateful_pointer<mainActorState> self);
void spawnAndMonitorSuperActor(main_actor_int::stateful_pointer<mainActorState> act, mainActorState &state) noexcept;
// clang-format off
void shutdownNow(main_actor_int::stateful_pointer<mainActorState> act, mainActorState &state) noexcept;
namespace impl
{
}
}  // namespace supervisor
}  // namespace ok::actr
