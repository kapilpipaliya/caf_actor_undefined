/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright 2011-2020 Dominik Charousset                                     *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include "caf/detail/meta_object.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "caf/binary_deserializer.hpp"
#include "caf/binary_serializer.hpp"
#include "caf/config.hpp"
#include "caf/deserializer.hpp"
#include "caf/error.hpp"
#include "caf/error_code.hpp"
#include "caf/serializer.hpp"
#include "caf/span.hpp"

namespace caf::detail {

#define fatal(str)                                                             \
  do {                                                                         \
    fprintf(stderr, "FATAL: " str "\n");                                       \
    abort();                                                                   \
  } while (false)

namespace {

// Stores global type information.
meta_object* meta_objects;

// Stores the size of `meta_objects`.
size_t meta_objects_size;

// Make sure to clean up all meta objects on program exit.
struct meta_objects_cleanup {
  ~meta_objects_cleanup() {
    delete[] meta_objects;
  }
} cleanup_helper;

} // namespace

span<const meta_object> global_meta_objects() {
  return {meta_objects, meta_objects_size};
}

const meta_object* global_meta_object(type_id_t id) {
  if (id < meta_objects_size) {
    auto& meta = meta_objects[id];
    return !meta.type_name.empty() ? &meta : nullptr;
  }
  return nullptr;
}

void clear_global_meta_objects() {
  if (meta_objects != nullptr) {
    delete[] meta_objects;
    meta_objects = nullptr;
    meta_objects_size = 0;
  }
}

span<meta_object> resize_global_meta_objects(size_t size) {
  if (size <= meta_objects_size)
    fatal("resize_global_meta_objects called with a new size that does not "
          "grow the array");
  auto new_storage = new meta_object[size];
  std::copy(meta_objects, meta_objects + meta_objects_size, new_storage);
  delete[] meta_objects;
  meta_objects = new_storage;
  meta_objects_size = size;
  return {new_storage, size};
}

void set_global_meta_objects(type_id_t first_id, span<const meta_object> xs) {
  auto new_size = first_id + xs.size();
  if (first_id < meta_objects_size) {
    if (new_size > meta_objects_size)
      fatal("set_global_meta_objects called with "
            "'first_id < meta_objects_size' and "
            "'new_size > meta_objects_size'");
    auto out = meta_objects + first_id;
    for (const auto& x : xs) {
      if (out->type_name.empty()) {
        // We support calling set_global_meta_objects for building the global
        // table chunk-by-chunk.
        *out = x;
      } else if (out->type_name == x.type_name) {
        // nop: set_global_meta_objects implements idempotency.
      } else {
        // Get null-terminated strings.
        auto name1 = to_string(out->type_name);
        auto name2 = to_string(x.type_name);
        fprintf(stderr,
                "FATAL: type ID %d already assigned to %s (tried to override "
                "with %s)\n",
                static_cast<int>(std::distance(meta_objects, out)),
                name1.c_str(), name2.c_str());
        abort();
      }
      ++out;
    }
    return;
  }
  auto dst = resize_global_meta_objects(first_id + xs.size());
  std::copy(xs.begin(), xs.end(), dst.begin() + first_id);
}

} // namespace caf::detail
