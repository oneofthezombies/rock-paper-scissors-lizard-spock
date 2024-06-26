#ifndef KERO_CORE_ERROR_H
#define KERO_CORE_ERROR_H

#include <source_location>

#include "kero/core/flat_json.h"

namespace kero {

struct Error final {
  using Code = i32;
  using Cause = Own<Error>;

  enum : i32 {
    kFailed = 1,
    kPropagated = 2,
  };

  Code code;
  FlatJson details;
  std::source_location location;
  Cause cause;

  explicit Error(const Code code,
                 FlatJson &&details,
                 std::source_location &&location,
                 Cause &&cause) noexcept;

  ~Error() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Error);

  [[nodiscard]] auto
  Take() noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       FlatJson &&details,
       Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       FlatJson &&details,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(FlatJson &&details,
       Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(FlatJson &&details,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;
};

auto
operator<<(std::ostream &os, const Error &error) -> std::ostream &;

}  // namespace kero

#endif  // KERO_CORE_ERROR_H
