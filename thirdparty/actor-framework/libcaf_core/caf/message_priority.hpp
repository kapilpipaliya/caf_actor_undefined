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

#include <cstdint>
#include <string>
#include <type_traits>

#include "caf/detail/core_export.hpp"

namespace caf {

enum class message_priority {
  high = 0,
  normal = 1,
};

using high_message_priority_constant
  = std::integral_constant<message_priority, message_priority::high>;

using normal_message_priority_constant
  = std::integral_constant<message_priority, message_priority::normal>;

CAF_CORE_EXPORT std::string to_string(message_priority);

} // namespace caf
