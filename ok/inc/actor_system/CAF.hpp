#pragma once
#include "caf/all.hpp"
CAF_BEGIN_TYPE_ID_BLOCK(okproject, first_custom_type_id)
CAF_ADD_ATOM(okproject, , shutdown_atom, "shutdown");
CAF_ADD_ATOM(okproject, , spawn_and_monitor_atom, "spawnmonit");
using main_actor_int = caf::typed_actor<caf::reacts_to<spawn_and_monitor_atom>,
                                        caf::reacts_to<shutdown_atom>>;
CAF_ADD_TYPE_ID(okproject, (main_actor_int))
CAF_END_TYPE_ID_BLOCK(okproject)
