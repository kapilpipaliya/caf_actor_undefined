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

#include "caf/detail/core_export.hpp"
#include "caf/fwd.hpp"
#include "caf/is_error_code_enum.hpp"

namespace caf {

/// PEC stands for "Parser Error Code". This enum contains error codes used by
/// various CAF parsers.
enum class pec : uint8_t {
  /// Not-an-error.
  success = 0,
  /// Parser succeeded but found trailing character(s).
  trailing_character = 1,
  /// Parser stopped after reaching the end while still expecting input.
  unexpected_eof,
  /// Parser stopped after reading an unexpected character.
  unexpected_character,
  /// Parsed integer exceeds the number of available bits of a `timespan`.
  timespan_overflow,
  /// Tried constructing a `timespan` with from a floating point number.
  fractional_timespan = 5,
  /// Too many characters for an atom.
  too_many_characters,
  /// Unrecognized character after escaping `\`.
  invalid_escape_sequence,
  /// Misplaced newline, e.g., inside a string.
  unexpected_newline,
  /// Parsed positive integer exceeds the number of available bits.
  integer_overflow,
  /// Parsed negative integer exceeds the number of available bits.
  integer_underflow = 10,
  /// Exponent of parsed double is less than the minimum supported exponent.
  exponent_underflow,
  /// Exponent of parsed double is greater than the maximum supported exponent.
  exponent_overflow,
  /// Parsed type does not match the expected type.
  type_mismatch,
  /// Stopped at an unrecognized option name.
  not_an_option,
  /// Stopped at an unparsable argument.
  invalid_argument = 15,
  /// Stopped because an argument was omitted.
  missing_argument,
  /// Stopped because the key of a category was taken.
  invalid_category,
  /// Stopped at an unexpected field name while reading a user-defined type.
  invalid_field_name,
  /// Stopped at a repeated field name while reading a user-defined type.
  repeated_field_name,
  /// Stopped while reading a user-defined type with one or more missing
  /// mandatory fields.
  missing_field = 20,
  /// Parsing a range statement ('n..m' or 'n..m..step') failed.
  invalid_range_expression,
  /// Stopped after running into an invalid parser state. Should never happen
  /// and most likely indicates a bug in the implementation.
  invalid_state,
};

CAF_CORE_EXPORT std::string to_string(pec);

} // namespace caf

CAF_ERROR_CODE_ENUM(pec)
