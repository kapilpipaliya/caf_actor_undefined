#pragma once
#include "caf/all.hpp"
//#include "caf/io/middleman.hpp"
CAF_BEGIN_TYPE_ID_BLOCK(okproject, first_custom_type_id)
CAF_ADD_TYPE_ID(okproject, (std::vector<std::string>))
CAF_ADD_TYPE_ID(okproject, (std::unordered_set<std::string>))
CAF_ADD_ATOM(okproject, , conn_exit_atom, "connexit");
CAF_ADD_ATOM(okproject, , shutdown_atom, "shutdown");
CAF_ADD_ATOM(okproject, , schema_changed_atom, "schemachng");
CAF_ADD_ATOM(okproject, , super_list_atom, "superlist");
CAF_ADD_ATOM(okproject, , user_list_atom, "userlist");
CAF_ADD_ATOM(okproject, , logout_atom, "logout");
CAF_ADD_ATOM(okproject, , super_mutate_atom, "supermut");
CAF_ADD_ATOM(okproject, , user_mutate_atom, "usermutate");
CAF_ADD_ATOM(okproject, , erase_atom, "eraseatom");
CAF_ADD_ATOM(okproject, , send_email_atom, "sendemail");
CAF_ADD_ATOM(okproject, , spawn_and_monitor_atom, "spawnmonit");
CAF_ADD_ATOM(okproject, , save_new_wsconnptr_atom, "savewsconn");
CAF_ADD_ATOM(okproject, , pass_to_ws_connection_atom, "passtowsac");
CAF_ADD_ATOM(okproject, , forward_to_all_ws_connection_atom, "sendtoall");
CAF_ADD_ATOM(okproject, , subscribe_to_total_ws_connections_atom, "subtotalws");
CAF_ADD_ATOM(okproject, , unsubscribe_to_total_ws_connections_atom, "unstotalws");
CAF_ADD_ATOM(okproject, , backup_atom, "backup");
CAF_ADD_ATOM(okproject, , db_health_check_atom, "dbhealth");
CAF_ADD_ATOM(okproject, , session_clean_atom, "sessioncln");
CAF_ADD_ATOM(okproject, , remove_atom, "removeatom");
CAF_ADD_ATOM(okproject, , send_to_same_browser_tab_atom, "sametab");
CAF_ADD_ATOM(okproject, , set_context_atom, "setcontext");
CAF_ADD_ATOM(okproject, , dispatch_atom, "dispatch");
CAF_ADD_ATOM(okproject, , send_to_one_atom, "oneatom");
CAF_ADD_ATOM(okproject, , send_to_one_database_actors_atom, "oneatom");
CAF_ADD_ATOM(okproject, , table_dispatch_atom, "table_dis");
CAF_ADD_ATOM(okproject, , table_erase_atom, "table_era");
CAF_ADD_ATOM(okproject, , file_notify_atom, "filenotify");
CAF_ADD_ATOM(okproject, , insert_atom, "insert");
CAF_ADD_ATOM(okproject, , sub_atom, "sub");
CAF_ADD_ATOM(okproject, , get_session_atom, "getsession");
using file_notify_actor_int = caf::typed_actor<caf::reacts_to<file_notify_atom>, caf::reacts_to<conn_exit_atom>>;
using backup_db_actor = caf::typed_actor<caf::reacts_to<backup_atom>, caf::reacts_to<conn_exit_atom>>;
using db_health_check_actor_int = caf::typed_actor<caf::reacts_to<db_health_check_atom>, caf::reacts_to<conn_exit_atom>>;
using session_clean_actor_int = caf::typed_actor<caf::reacts_to<session_clean_atom>, caf::reacts_to<conn_exit_atom>>;
using email_actor_int = caf::typed_actor<caf::reacts_to<send_email_atom, std::string, std::string, std::string, std::string>, caf::reacts_to<conn_exit_atom>>;

using main_actor_int = caf::typed_actor<caf::reacts_to<spawn_and_monitor_atom>,
                                        caf::reacts_to<shutdown_atom>>;

CAF_ADD_TYPE_ID(okproject, (file_notify_actor_int))
CAF_ADD_TYPE_ID(okproject, (backup_db_actor))
CAF_ADD_TYPE_ID(okproject, (db_health_check_actor_int))
CAF_ADD_TYPE_ID(okproject, (session_clean_actor_int))
CAF_ADD_TYPE_ID(okproject, (email_actor_int))
CAF_ADD_TYPE_ID(okproject, (main_actor_int))
CAF_END_TYPE_ID_BLOCK(okproject)
#define CONN_EXIT                            \
  [=](conn_exit_atom) {                      \
    LOG_DEBUG << "exiting " << self->name(); \
    self->unbecome();                        \
  }
namespace ok::actr
{
namespace supervisor
{
inline std::unique_ptr<caf::actor_system_config> cfg;
inline std::unique_ptr<caf::actor_system> actorSystem;
inline main_actor_int mainActor;
void initialiseMainActor() noexcept;
std::string getReasonString(caf::error &err) noexcept;
using exception_handler = std::function<caf::error(caf::scheduled_actor *, std::exception_ptr &)>;
exception_handler default_exception_handler(std::string const &msg);
}  // namespace supervisor
}  // namespace ok::actr
