#include "MainActor.hpp"
#include <chrono>
namespace ok::actr {
namespace supervisor {
main_actor_int::behavior_type
MainActor(main_actor_int::stateful_pointer<mainActorState> self) {
  self->set_error_handler([](caf::error &err) {
    std::cout << "Main Actor Error :";
    std::cout << ok::actr::supervisor::getReasonString(err);
  });
  self->set_down_handler(
      [](caf::scheduled_actor *act, caf::down_msg &msg) noexcept {
        std::cout << "Monitored Actor Error Down Error :" << act->name();
        std::cout << ok::actr::supervisor::getReasonString(msg.reason);
      });
  // If self exception error occur: server freeze.
  self->set_exception_handler(
      [](caf::scheduled_actor *,
         std::exception_ptr &eptr) noexcept -> caf::error {
        try {
          if (eptr) {
            std::rethrow_exception(eptr);
          }
        } catch (std::exception const &e) {
          std::cout << "Main Actor Exception Error : " << e.what();
        }
        return caf::make_error(
            caf::pec::success); // self will not resume actor.
      });
  self->set_default_handler([](caf::scheduled_actor *ptr, caf::message &msg) {
    CAF_LOG_WARNING("unexpected message:" << msg);
    std::cout << "*** unexpected message [id: " << ptr->id()
              << ", name: " << ptr->name() << "]: " << to_string(msg);
    return make_error(caf::sec::unexpected_message);
  });
  // api_count =
  // Myok::actr::supervisor::communicateWithActors()->spawn(api_count_fun);
  return {
      [=](spawn_and_monitor_atom) {
        spawnAndMonitorSuperActor(self, self->state);
      },
      [=](shutdown_atom) { shutdownNow(self, self->state); },
  };
}

void shutdownNow(main_actor_int::stateful_pointer<mainActorState> act,
                 mainActorState &state) noexcept {
  std::cout << "shutdown main actor";
}
} // namespace supervisor
} // namespace ok::actr
