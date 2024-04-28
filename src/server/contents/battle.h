#ifndef SERVER_CONTENTS_BATTLE_H
#define SERVER_CONTENTS_BATTLE_H

#include "common.h"
#include "core/core.h"
#include "server/engine/socket.h"
#include "server/engine/socket_event_loop_handler.h"

namespace contents {

struct BattleState {
  uint64_t battle_id{};

  explicit BattleState() noexcept = default;
  ~BattleState() noexcept = default;
  CLASS_KIND_MOVABLE(BattleState);
};

class Battle final : public engine::SocketEventLoopHandler<Battle> {
 public:
  using Super = engine::SocketEventLoopHandler<Battle>;

  explicit Battle() noexcept = default;
  virtual ~Battle() noexcept override = default;
  CLASS_KIND_MOVABLE(Battle);

  [[nodiscard]] virtual auto
  OnInit(engine::EventLoop &event_loop,
         const engine::Config &config) noexcept -> Result<Void> override;

  [[nodiscard]] virtual auto
  OnMail(engine::EventLoop &event_loop,
         const engine::Mail &mail) noexcept -> Result<Void> override;

  [[nodiscard]] virtual auto
  OnSocketIn(engine::EventLoop &event_loop,
             const engine::SocketId socket_id) noexcept
      -> Result<Void> override;

 private:
  [[nodiscard]] auto
  OnStart(engine::EventLoop &event_loop,
          const engine::Mail &mail) noexcept -> Result<Void>;

  std::unordered_map<engine::SocketId, BattleState> battle_states_;
};

}  // namespace contents

#endif  // SERVER_CONTENTS_BATTLE_H