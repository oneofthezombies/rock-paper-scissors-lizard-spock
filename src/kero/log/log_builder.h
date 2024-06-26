#ifndef KERO_LOG_LOG_BUILDER_H
#define KERO_LOG_LOG_BUILDER_H

#include <source_location>
#include <sstream>

#include "kero/core/result.h"
#include "kero/log/core.h"
#include "kero/log/global_context.h"

namespace kero {
namespace log {

class LogBuilder final {
 public:
  explicit LogBuilder(std::string&& message,
                      std::source_location&& location,
                      const Level level) noexcept;
  ~LogBuilder() noexcept = default;
  KERO_CLASS_KIND_PINNABLE(LogBuilder);

  template <typename T>
  [[nodiscard]] auto
  Data(std::string&& key, T&& value) noexcept -> LogBuilder& {
    std::stringstream ss;
    ss << std::forward<T>(value);

    const auto entry = log_->data.find(key);
    if (entry != log_->data.end()) {
      GetGlobalContext().LogSystemError("Overwriting existing data key: " +
                                        key);

      entry->second = ss.str();
    } else {
      log_->data.emplace(key, ss.str());
    }

    return *this;
  }

  auto
  Log() noexcept -> Result<Void>;

 private:
  Own<kero::Log> log_;
};

[[nodiscard]] auto
Debug(std::string&& message,
      std::source_location&& location = std::source_location::current())
    -> LogBuilder;

[[nodiscard]] auto
Info(std::string&& message,
     std::source_location&& location = std::source_location::current())
    -> LogBuilder;

[[nodiscard]] auto
Warn(std::string&& message,
     std::source_location&& location = std::source_location::current())
    -> LogBuilder;

[[nodiscard]] auto
Error(std::string&& message,
      std::source_location&& location = std::source_location::current())
    -> LogBuilder;

}  // namespace log
}  // namespace kero

#endif  // KERO_LOG_LOG_BUILDER_H
