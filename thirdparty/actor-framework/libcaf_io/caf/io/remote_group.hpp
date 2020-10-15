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

#include "caf/actor_system.hpp"

#include "caf/io/middleman.hpp"

namespace caf::io {

inline expected<group>
remote_group(actor_system& sys, const std::string& group_uri) {
  return sys.middleman().remote_group(group_uri);
}

inline expected<group>
remote_group(actor_system& sys, const std::string& group_identifier,
             const std::string& host, uint16_t port) {
  return sys.middleman().remote_group(group_identifier, host, port);
}

} // namespace caf::io
