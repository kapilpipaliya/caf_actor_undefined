/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright 2011-2019 Dominik Charousset                                     *
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

#include <vector>

#include "caf/actor_cast.hpp"
#include "caf/actor_clock.hpp"
#include "caf/actor_control_block.hpp"
#include "caf/actor_profiler.hpp"
#include "caf/fwd.hpp"
#include "caf/mailbox_element.hpp"
#include "caf/message_id.hpp"
#include "caf/no_stages.hpp"

namespace caf::detail {

template <class Self, class SelfHandle, class Handle, class... Ts>
void profiled_send(Self* self, SelfHandle&& src, const Handle& dst,
                   message_id msg_id, std::vector<strong_actor_ptr> stages,
                   execution_unit* context, Ts&&... xs) {
  if (dst) {
    auto element = make_mailbox_element(std::forward<SelfHandle>(src), msg_id,
                                        std::move(stages),
                                        std::forward<Ts>(xs)...);
    CAF_BEFORE_SENDING(self, *element);
    dst->enqueue(std::move(element), context);
  } else {
    self->home_system().base_metrics().rejected_messages->inc();
  }
}

template <class Self, class SelfHandle, class Handle, class... Ts>
void profiled_send(Self* self, SelfHandle&& src, const Handle& dst,
                   actor_clock& clock, actor_clock::time_point timeout,
                   message_id msg_id, Ts&&... xs) {
  if (dst) {
    if constexpr (std::is_same<Handle, group>::value) {
      clock.schedule_message(timeout, dst, std::forward<SelfHandle>(src),
                             make_message(std::forward<Ts>(xs)...));
    } else {
      auto element = make_mailbox_element(std::forward<SelfHandle>(src), msg_id,
                                          no_stages, std::forward<Ts>(xs)...);
      CAF_BEFORE_SENDING_SCHEDULED(self, timeout, *element);
      clock.schedule_message(timeout, actor_cast<strong_actor_ptr>(dst),
                             std::move(element));
    }
  } else {
    self->home_system().base_metrics().rejected_messages->inc();
  }
}

} // namespace caf::detail
