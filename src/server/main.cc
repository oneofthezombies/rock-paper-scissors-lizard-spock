#include "core/tiny_json.h"
#include "core/utils.h"
#include "engine/config.h"
#include "engine/engine.h"
#include "server/contents/battle.h"
#include "server/contents/lobby.h"
#include "server/engine/event_loop.h"

enum Symbol : i32 {
  kHelpRequested = 0,
  kPortArgNotFound,
  kPortValueNotFound,
  kPortParsingFailed,
  kUnknownArgument,
};

auto
operator<<(std::ostream &os, const Symbol &symbol) -> std::ostream & {
  os << "Symbol{";
  os << static_cast<i32>(symbol);
  os << "}";
  return os;
}

using Error = core::Error;

template <typename T>
using Result = core::Result<T>;

auto
ParseArgs(core::Args &&args) noexcept -> Result<engine::Config>;

auto
main(int argc, char **argv) noexcept -> int {
  auto config_res = ParseArgs(core::ParseArgcArgv(argc, argv));
  if (config_res.IsErr()) {
    const auto &error = config_res.Err();
    switch (error.code) {
      case kHelpRequested:
        // noop
        break;
      case kPortArgNotFound:
        core::JsonParser{}
            .Set("message", "port argument not found")
            .Set("error", error)
            .LogLn();
        break;
      case kPortValueNotFound:
        core::JsonParser{}
            .Set("message", "port value not found")
            .Set("error", error)
            .LogLn();
        break;
      case kPortParsingFailed:
        core::JsonParser{}
            .Set("message", "port parsing failed")
            .Set("error", error)
            .LogLn();
        break;
      case kUnknownArgument:
        core::JsonParser{}
            .Set("message", "unknown argument")
            .Set("error", error)
            .LogLn();
        break;
    }

    core::JsonParser{}.Set("usage", "server [--port <port>]").LogLn();
    return 1;
  }

  auto config = std::move(config_res.Ok());
  config.primary_event_loop_name = "lobby";
  if (auto res = config.Validate(); res.IsErr()) {
    core::JsonParser{}
        .Set("message", "config validation failed")
        .Set("error", res.Err())
        .LogLn();
    return 1;
  }

  auto engine_res = engine::Engine::Builder{}.Build(std::move(config));
  if (engine_res.IsErr()) {
    core::JsonParser{}
        .Set("message", "engine build failed")
        .Set("error", engine_res.Err())
        .LogLn();
    return 1;
  }

  std::vector<std::pair<std::string, engine::EventLoopHandlerPtr>> event_loops;
  event_loops.emplace_back("lobby", std::make_unique<contents::Lobby>());
  event_loops.emplace_back("battle", std::make_unique<contents::Battle>());

  auto engine = std::move(engine_res.Ok());
  for (auto &[name, handler] : event_loops) {
    if (auto res =
            engine.RegisterEventLoop(std::string{name}, std::move(handler));
        res.IsErr()) {
      core::JsonParser{}
          .Set("message", "register event loop handler failed")
          .Set("error", res.Err())
          .LogLn();
      return 1;
    }
  }

  if (auto res = engine.Run(); res.IsErr()) {
    std::cout << res.Err() << std::endl;
    return 1;
  }

  return 0;
}

auto
ParseArgs(core::Args &&args) noexcept -> Result<engine::Config> {
  using ResultT = Result<engine::Config>;

  engine::Config config{};
  core::Tokenizer tokenizer{std::move(args)};

  // Skip the first argument which is the program name
  tokenizer.Eat();

  for (auto current = tokenizer.Current(); current.has_value();
       tokenizer.Eat(), current = tokenizer.Current()) {
    const auto token = *current;

    if (token == "--help") {
      return ResultT{Error::From(kHelpRequested)};
    }

    if (token == "--port") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return ResultT{Error::From(kPortValueNotFound)};
      }

      auto result = core::ParseNumberString<u16>(*next);
      if (result.IsErr()) {
        return ResultT{Error::From(
            kPortParsingFailed,
            core::JsonParser{}.Set("error", result.Err()).IntoMap())};
      }

      config.port = result.Ok();
      tokenizer.Eat();
      continue;
    }

    return ResultT{
        Error::From(kUnknownArgument,
                    core::JsonParser{}.Set("token", token).IntoMap())};
  }

  if (config.port == engine::Config::kUndefinedPort) {
    return ResultT{Error::From(kPortArgNotFound)};
  }

  return ResultT{std::move(config)};
}
