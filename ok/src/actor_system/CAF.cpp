#include "CAF.hpp"
#include "MainActor.hpp"
namespace ok::actr {
namespace supervisor {
void initialiseMainActor() noexcept {
  // Initialze the global type information before anything else.
  // caf::init_global_meta_objects<...>();
  caf::init_global_meta_objects<caf::id_block::okproject>();
  // caf::io::middleman::init_global_meta_objects();
  caf::core::init_global_meta_objects();
  cfg = std::make_unique<caf::actor_system_config>();
  if (auto err = cfg->parse(0, nullptr)) {
    std::cerr << to_string(err) << std::endl;
  }
  actorSystem = std::make_unique<caf::actor_system>(*cfg);
  // self = std::make_unique<caf::scoped_actor>(*actorSystem);
  mainActor = actorSystem->spawn(ok::actr::supervisor::MainActor);
  // or
  // main_actor = self->spawn<ok::actr::supervisor::MainActor>();
  CAF_LOG_DEBUG("initialized caf system");
}
std::string getReasonString(caf::error &err) noexcept {
  return caf::to_string(err);
}
using exception_handler =
    std::function<caf::error(caf::scheduled_actor *, std::exception_ptr &)>;
exception_handler default_exception_handler(std::string const &msg) {
  return [=](caf::scheduled_actor *, std::exception_ptr &eptr) -> caf::error {
    try {
      if (eptr) {
        std::rethrow_exception(eptr);
      }
    } catch (std::exception const &e) {
      std::cout << "Exception Error : " << e.what();
    }
    return caf::make_error(caf::pec::success); // This will not resume actor.
  };
}
} // namespace supervisor
} // namespace ok::actr
