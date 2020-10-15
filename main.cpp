#include "CAF.hpp"
#include <filesystem>
int main(int argc, char *argv[]) {
  ok::actr::supervisor::initialiseMainActor();
  caf::anon_send(ok::actr::supervisor::mainActor, spawn_and_monitor_atom_v);
  return EXIT_SUCCESS;
}
