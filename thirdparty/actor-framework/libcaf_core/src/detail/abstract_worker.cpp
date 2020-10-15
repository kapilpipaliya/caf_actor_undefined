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

#include "caf/detail/abstract_worker.hpp"

namespace caf::detail {

// -- constructors, destructors, and assignment operators ----------------------

abstract_worker::abstract_worker() : next_(nullptr) {
  // nop
}

abstract_worker::~abstract_worker() {
  // nop
}

// -- implementation of resumable ----------------------------------------------

resumable::subtype_t abstract_worker::subtype() const {
  return resumable::function_object;
}

void abstract_worker::intrusive_ptr_add_ref_impl() {
  ref();
}

void abstract_worker::intrusive_ptr_release_impl() {
  deref();
}

} // namespace caf::detail
