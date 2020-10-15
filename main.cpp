#include "CAF.hpp"
#include "MainActor.hpp"
#include <filesystem>
namespace {
void initialiseMainActor() {
  caf::init_global_meta_objects<caf::id_block::okproject>();
  caf::core::init_global_meta_objects();
  auto cfg = caf::actor_system_config();
  if (auto err = cfg.parse(0, nullptr)) {
    std::cerr << to_string(err) << std::endl;
  }
  auto actorSystem = caf::actor_system(cfg);
  auto mainActor = actorSystem.spawn(ok::mainActor);
}
} // namespace
int main(int argc, char *argv[]) {
  initialiseMainActor();
  return EXIT_SUCCESS;
}
