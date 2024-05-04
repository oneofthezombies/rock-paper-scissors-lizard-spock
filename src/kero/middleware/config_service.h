#ifndef KERO_MIDDLEWARE_CONFIG_SERVICE_H
#define KERO_MIDDLEWARE_CONFIG_SERVICE_H

#include "kero/core/args_scanner.h"
#include "kero/core/common.h"
#include "kero/engine/service.h"
#include "kero/engine/service_factory.h"
#include "kero/middleware/common.h"

namespace kero {

class ConfigService final : public Service {
 public:
  explicit ConfigService(const Pin<RunnerContext> runner_context,
                         FlatJson&& config) noexcept;
  virtual ~ConfigService() noexcept override = default;
  CLASS_KIND_MOVABLE(ConfigService);

  static constexpr ServiceKindId kKindId = kServiceKindId_Config;
  static constexpr ServiceKindName kKindName = "config";

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  [[nodiscard]] auto
  GetConfig() const noexcept -> const FlatJson&;

  [[nodiscard]] auto
  GetConfig() noexcept -> FlatJson&;

  [[nodiscard]] static auto
  GetKindId() noexcept -> ServiceKindId;

  [[nodiscard]] static auto
  GetKindName() noexcept -> ServiceKindName;

 private:
  FlatJson config_;
};

class ConfigServiceFactory : public ServiceFactory {
 public:
  enum : Error::Code {
    kPortNotFound = 1,
    kPortParsingFailed,
    kUnknownArgument
  };

  explicit ConfigServiceFactory(int argc, char** argv) noexcept;
  virtual ~ConfigServiceFactory() noexcept override = default;

  [[nodiscard]] virtual auto
  Create(const Pin<RunnerContext> runner_context) noexcept
      -> Result<Own<Service>> override;

 private:
  Args args_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_CONFIG_SERVICE_H
