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

#pragma once

#include <type_traits>

#include "caf/actor_control_block.hpp"
#include "caf/actor_storage.hpp"
#include "caf/fwd.hpp"
#include "caf/infer_handle.hpp"
#include "caf/intrusive_ptr.hpp"
#include "caf/logger.hpp"
#include "caf/ref_counted.hpp"

namespace caf {

template <class T, class R = infer_handle_from_class_t<T>, class... Ts>
R make_actor(actor_id aid, node_id nid, actor_system* sys, Ts&&... xs) {
#if CAF_LOG_LEVEL >= CAF_LOG_LEVEL_DEBUG
  if (logger::current_logger()->accepts(CAF_LOG_LEVEL_DEBUG,
                                        CAF_LOG_FLOW_COMPONENT)) {
    std::string args;
    args = deep_to_string(std::forward_as_tuple(xs...));
    actor_storage<T>* ptr;
    {
      CAF_PUSH_AID(aid);
      ptr = new actor_storage<T>(aid, std::move(nid), sys,
                                 std::forward<Ts>(xs)...);
    }
    CAF_LOG_SPAWN_EVENT(ptr->data, args);
    ptr->data.setup_metrics();
    return {&(ptr->ctrl), false};
  }
#endif
  CAF_PUSH_AID(aid);
  auto ptr = new actor_storage<T>(aid, std::move(nid), sys,
                                  std::forward<Ts>(xs)...);
  ptr->data.setup_metrics();
  return {&(ptr->ctrl), false};
}

} // namespace caf
