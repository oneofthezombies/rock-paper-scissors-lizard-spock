#ifndef KERO_SERVICE_ACTOR_SERVICE_H
#define KERO_SERVICE_ACTOR_SERVICE_H

#include <memory>

#include "kero/core/common.h"
#include "kero/engine/actor_system.h"
#include "kero/engine/service.h"
#include "runner_context.h"

namespace kero {

static const ServiceKind kServiceKindActor = {2, "actor"};

class ActorService final : public Service {
 public:
  virtual ~ActorService() noexcept override = default;
  CLASS_KIND_MOVABLE(ActorService);

  virtual auto
  OnUpdate() noexcept -> void override;

  [[nodiscard]] auto
  GetName() const noexcept -> const std::string &;

  auto
  SendMail(std::string &&to, std::string &&event, Json &&body) noexcept -> void;

 private:
  explicit ActorService(const Pin<RunnerContext> runner_context,
                        std::string &&name,
                        MailBox &&mail_box) noexcept;

  MailBox mail_box_;
  std::string name_;

  friend class ActorSystem;
};

using ActorServicePtr = Owned<ActorService>;

}  // namespace kero

#endif  // KERO_SERVICE_ACTOR_SERVICE_H