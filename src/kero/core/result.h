#ifndef KERO_CORE_RESULT_H
#define KERO_CORE_RESULT_H

#include <variant>

#include "kero/core/error.h"

namespace kero {

template <typename T>
class Result {
 public:
  Result(T&& value) noexcept : data_{std::forward<T>(value)} {}
  Result(Error&& error) noexcept : data_{std::forward<Error>(error)} {}
  ~Result() noexcept = default;
  CLASS_KIND_MOVABLE(Result);

  [[nodiscard]] auto
  IsOk() const noexcept -> bool {
    return std::holds_alternative<T>(data_);
  }

  [[nodiscard]] auto
  IsErr() const noexcept -> bool {
    return std::holds_alternative<Error>(data_);
  }

  [[nodiscard]] auto
  Ok() const noexcept -> const T& {
    return std::get<T>(data_);
  }

  [[nodiscard]] auto
  TakeOk() noexcept -> T {
    return std::move(std::get<T>(data_));
  }

  [[nodiscard]] auto
  Err() const noexcept -> const Error& {
    return std::get<Error>(data_);
  }

  [[nodiscard]] auto
  TakeErr() noexcept -> Error {
    return std::move(std::get<Error>(data_));
  }

  [[nodiscard]] auto
  Take() noexcept -> Result {
    return std::move(*this);
  }

  [[nodiscard]] static auto
  From(T&& value) noexcept -> Result {
    return Result{std::forward<T>(value)};
  }

  [[nodiscard]] static auto
  From(Error&& error) noexcept -> Result {
    return Result{std::forward<Error>(error)};
  }

  static auto
  Ok(T&& value) noexcept -> Result<T> {
    return Result<T>::From(std::forward<T>(value));
  }

  static auto
  Err(Error&& error) noexcept -> Result<T> {
    return Result<T>::From(std::forward<Error>(error));
  }

 private:
  std::variant<std::monostate, T, Error> data_;
};

}  // namespace kero

#endif  // KERO_CORE_RESULT_H