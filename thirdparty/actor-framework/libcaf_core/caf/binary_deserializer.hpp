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

#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

#include "caf/detail/core_export.hpp"
#include "caf/error_code.hpp"
#include "caf/fwd.hpp"
#include "caf/load_inspector_base.hpp"
#include "caf/sec.hpp"
#include "caf/span.hpp"
#include "caf/string_view.hpp"

namespace caf {

/// Deserializes objects from sequence of bytes.
class CAF_CORE_EXPORT binary_deserializer
  : public load_inspector_base<binary_deserializer> {
public:
  // -- constructors, destructors, and assignment operators --------------------

  template <class Container>
  binary_deserializer(actor_system& sys, const Container& input) noexcept
    : binary_deserializer(sys) {
    reset(as_bytes(make_span(input)));
  }

  template <class Container>
  binary_deserializer(execution_unit* ctx, const Container& input) noexcept
    : context_(ctx) {
    reset(as_bytes(make_span(input)));
  }

  binary_deserializer(execution_unit* ctx, const void* buf,
                      size_t size) noexcept
    : binary_deserializer(ctx,
                          make_span(reinterpret_cast<const byte*>(buf), size)) {
    // nop
  }

  binary_deserializer(actor_system& sys, const void* buf, size_t size) noexcept
    : binary_deserializer(sys,
                          make_span(reinterpret_cast<const byte*>(buf), size)) {
    // nop
  }

  // -- properties -------------------------------------------------------------

  /// Returns how many bytes are still available to read.
  size_t remaining() const noexcept {
    return static_cast<size_t>(end_ - current_);
  }

  /// Returns the remaining bytes.
  span<const byte> remainder() const noexcept {
    return make_span(current_, end_);
  }

  /// Returns the current execution unit.
  execution_unit* context() const noexcept {
    return context_;
  }

  /// Jumps `num_bytes` forward.
  /// @pre `num_bytes <= remaining()`
  void skip(size_t num_bytes);

  /// Assigns a new input.
  void reset(span<const byte> bytes) noexcept;

  /// Returns the current read position.
  const byte* current() const noexcept {
    return current_;
  }

  /// Returns the end of the assigned memory block.
  const byte* end() const noexcept {
    return end_;
  }

  static constexpr bool has_human_readable_format() noexcept {
    return false;
  }

  // -- overridden member functions --------------------------------------------

  bool fetch_next_object_type(type_id_t& type) noexcept;

  constexpr bool begin_object(string_view) noexcept {
    return ok;
  }

  constexpr bool end_object() noexcept {
    return ok;
  }

  constexpr bool begin_field(string_view) noexcept {
    return true;
  }

  bool begin_field(string_view name, bool& is_present) noexcept;

  bool begin_field(string_view name, span<const type_id_t> types,
                   size_t& index) noexcept;

  bool begin_field(string_view name, bool& is_present,
                   span<const type_id_t> types, size_t& index) noexcept;

  constexpr bool end_field() {
    return ok;
  }

  constexpr bool begin_tuple(size_t) noexcept {
    return ok;
  }

  constexpr bool end_tuple() noexcept {
    return ok;
  }

  constexpr bool begin_key_value_pair() noexcept {
    return ok;
  }

  constexpr bool end_key_value_pair() noexcept {
    return ok;
  }

  bool begin_sequence(size_t& list_size) noexcept;

  constexpr bool end_sequence() noexcept {
    return ok;
  }

  bool begin_associative_array(size_t& size) noexcept {
    return begin_sequence(size);
  }

  bool end_associative_array() noexcept {
    return end_sequence();
  }

  bool value(bool& x) noexcept;

  bool value(byte& x) noexcept;

  bool value(uint8_t& x) noexcept;

  bool value(int8_t& x) noexcept;

  bool value(int16_t& x) noexcept;

  bool value(uint16_t& x) noexcept;

  bool value(int32_t& x) noexcept;

  bool value(uint32_t& x) noexcept;

  bool value(int64_t& x) noexcept;

  bool value(uint64_t& x) noexcept;

  bool value(float& x) noexcept;

  bool value(double& x) noexcept;

  bool value(long double& x);

  bool value(std::string& x);

  bool value(std::u16string& x);

  bool value(std::u32string& x);

  bool value(span<byte> x) noexcept;

  bool value(std::vector<bool>& x);

private:
  explicit binary_deserializer(actor_system& sys) noexcept;

  /// Checks whether we can read `read_size` more bytes.
  bool range_check(size_t read_size) const noexcept {
    return current_ + read_size <= end_;
  }

  /// Points to the current read position.
  const byte* current_;

  /// Points to the end of the assigned memory block.
  const byte* end_;

  /// Provides access to the ::proxy_registry and to the ::actor_system.
  execution_unit* context_;
};

} // namespace caf
