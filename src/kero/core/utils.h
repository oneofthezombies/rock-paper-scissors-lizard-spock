#ifndef KERO_CORE_UTILS_H
#define KERO_CORE_UTILS_H

#include <charconv>
#include <string_view>

#include "kero/core/result.h"

namespace kero {

[[nodiscard]] auto
OkVoid() -> Result<Void>;

template <typename T>
  requires std::integral<T> || std::floating_point<T>
[[nodiscard]] auto
ParseNumberString(const std::string_view token) noexcept -> Result<T> {
  using ResultT = Result<T>;

  T value{};
  auto [ptr, ec] =
      std::from_chars(token.data(), token.data() + token.size(), value);
  if (ec != std::errc{}) {
    return ResultT::Err(
        Error::From(FlatJson{}
                        .Set("kind", "errc")
                        .Set("code", static_cast<i32>(ec))
                        .Set("message", std::make_error_code(ec).message())
                        .Take()));
  }

  return ResultT::Ok(std::move(value));
}

class Defer final {
 public:
  explicit Defer(std::function<void()> &&fn) noexcept;
  ~Defer() noexcept;
  KERO_CLASS_KIND_PINNABLE(Defer);

  auto
  Cancel() noexcept -> void;

 private:
  std::function<void()> fn_;
};

class StackDefer final {
 public:
  using Fn = std::function<void()>;

  explicit StackDefer() noexcept = default;
  ~StackDefer() noexcept;
  KERO_CLASS_KIND_PINNABLE(StackDefer);

  auto
  Push(Fn &&fn) noexcept -> void;

  auto
  Cancel() noexcept -> void;

 private:
  std::vector<Fn> stack_;
};

}  // namespace kero

#endif  // KERO_CORE_UTILS_H
