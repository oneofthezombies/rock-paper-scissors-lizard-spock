#include "io_event_loop_service.h"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>

#include "kero/core/utils.h"
#include "kero/core/utils_linux.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/common.h"

using namespace kero;

namespace {

[[nodiscard]] static auto
AddOptionsToEpollEvents(
    const kero::IoEventLoopService::AddOptions options) noexcept -> u32 {
  u32 events{0};

  if (options.in) {
    events |= EPOLLIN;
  }

  if (options.out) {
    events |= EPOLLOUT;
  }

  if (options.edge_trigger) {
    events |= EPOLLET;
  }

  return events;
}

}  // namespace

kero::IoEventLoopService::IoEventLoopService(
    const Borrow<RunnerContext> runner_context) noexcept
    : Service{runner_context, {}} {}

auto
kero::IoEventLoopService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  const auto epoll_fd = epoll_create1(0);
  if (!Fd::IsValid(epoll_fd)) {
    return ResultT::Err(
        Error::From(Errno::FromErrno()
                        .IntoFlatJson()
                        .Set("message", std::string{"Failed to create epoll"})
                        .Take()));
  }

  epoll_fd_ = epoll_fd;
  return OkVoid();
}

auto
kero::IoEventLoopService::OnDestroy() noexcept -> void {
  if (!Fd::IsValid(epoll_fd_)) {
    return;
  }

  if (auto res = Fd::Close(epoll_fd_); res.IsErr()) {
    log::Error("Failed to close epoll fd").Data("fd", epoll_fd_).Log();
  }
}

auto
kero::IoEventLoopService::OnUpdate() noexcept -> void {
  if (!Fd::IsValid(epoll_fd_)) {
    log::Error("Invalid epoll fd").Data("fd", epoll_fd_).Log();
    return;
  }

  struct epoll_event events[kMaxEvents]{};
  const auto fd_count = epoll_wait(epoll_fd_, events, kMaxEvents, 0);
  if (fd_count == -1) {
    if (errno == EINTR) {
      return;
    }

    log::Error("Failed to wait for epoll events")
        .Data("fd", epoll_fd_)
        .Data("errno", Errno::FromErrno())
        .Log();
    return;
  }

  for (int i = 0; i < fd_count; ++i) {
    const auto& event = events[i];
    if (auto res = OnUpdateEpollEvent(event); res.IsErr()) {
      log::Error("Failed to update epoll event")
          .Data("fd", event.data.fd)
          .Data("error", res.TakeErr())
          .Log();
      continue;
    }
  }
}

auto
kero::IoEventLoopService::OnUpdateEpollEvent(
    const struct epoll_event& event) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (event.events & EPOLLERR) {
    int code{};
    socklen_t code_size = sizeof(code);
    if (getsockopt(event.data.fd, SOL_SOCKET, SO_ERROR, &code, &code_size) <
        0) {
      return ResultT::Err(
          Error::From(Errno::FromErrno()
                          .IntoFlatJson()
                          .Set("message", "Failed to get socket error")
                          .Set("fd", event.data.fd)
                          .Take()));
    }

    if (code == 0) {
      return ResultT::Err(
          Error::From(FlatJson{}
                          .Set("message", "Socket error is zero")
                          .Set("fd", event.data.fd)
                          .Take()));
    }

    const auto description = std::string_view{strerror(code)};
    if (auto res = InvokeEvent(
            EventSocketError::kEvent,
            FlatJson{}
                .Set(EventSocketError::kSocketId,
                     static_cast<u64>(event.data.fd))
                .Set(EventSocketError::kErrorCode, code)
                .Set(EventSocketError::kErrorDescription, description))) {
      log::Error("Failed to invoke socket error event")
          .Data("error", res.TakeErr())
          .Log();
    }
  }

  if (event.events & EPOLLHUP) {
    if (auto res =
            InvokeEvent(EventSocketClose::kEvent,
                        FlatJson{}.Set(EventSocketClose::kSocketId,
                                       static_cast<u64>(event.data.fd)))) {
      log::Error("Failed to invoke socket close event")
          .Data("error", res.TakeErr())
          .Log();
    }

    if (auto res = Fd::Close(event.data.fd); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  }

  if (event.events & EPOLLIN) {
    if (auto res = InvokeEvent(EventSocketRead::kEvent,
                               FlatJson{}.Set(EventSocketRead::kSocketId,
                                              static_cast<u64>(event.data.fd)));
        res.IsErr()) {
      log::Error("Failed to invoke socket read event")
          .Data("error", res.TakeErr())
          .Log();
    }
  }

  return OkVoid();
}

auto
kero::IoEventLoopService::AddFd(const Fd::Value fd, const AddOptions options)
    const noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (!Fd::IsValid(epoll_fd_)) {
    return ResultT::Err(Error::From(kInvalidEpollFd));
  }

  struct epoll_event ev {};
  ev.events = AddOptionsToEpollEvents(options);
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoFlatJson()
            .Set("message", std::string{"Failed to add fd to epoll"})
            .Set("fd", static_cast<double>(fd))
            .Take()));
  }

  return OkVoid();
}

auto
kero::IoEventLoopService::RemoveFd(const Fd::Value fd) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (!Fd::IsValid(epoll_fd_)) {
    return ResultT::Err(Error::From(kInvalidEpollFd));
  }

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoFlatJson()
            .Set("message", std::string{"Failed to remove fd from epoll"})
            .Set("fd", static_cast<double>(fd))
            .Take()));
  }

  return OkVoid();
}

auto
kero::IoEventLoopService::WriteToFd(const Fd::Value fd,
                                    const std::string_view data) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  const auto data_size = data.size();
  const auto data_ptr = data.data();
  auto data_sent = 0;
  while (data_sent < data_size) {
    const auto sent = send(fd, data_ptr + data_sent, data_size - data_sent, 0);
    if (sent == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }

      return ResultT::Err(Error::From(
          Errno::FromErrno()
              .IntoFlatJson()
              .Set("message", std::string{"Failed to send data to fd"})
              .Set("fd", static_cast<double>(fd))
              .Set("data", std::string{data_ptr, data_size})
              .Take()));
    }

    data_sent += sent;
  }

  return OkVoid();
}

auto
kero::IoEventLoopService::ReadFromFd(const Fd::Value fd) noexcept
    -> Result<std::string> {
  using ResultT = Result<std::string>;

  std::string buffer(4096, '\0');
  size_t total_read{0};
  while (true) {
    const auto read = recv(fd, buffer.data(), buffer.size(), 0);
    if (read == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }

      return ResultT::Err(Error::From(
          Errno::FromErrno()
              .IntoFlatJson()
              .Set("message", std::string{"Failed to read data from fd"})
              .Set("fd", static_cast<double>(fd))
              .Take()));
    }

    if (read == 0) {
      if (auto res = InvokeEvent(EventSocketClose::kEvent,
                                 FlatJson{}.Set(EventSocketClose::kSocketId,
                                                static_cast<u64>(fd)));
          res.IsErr()) {
        log::Error("Failed to invoke socket close event")
            .Data("error", res.TakeErr())
            .Log();
      }

      return ResultT::Err(
          Error::From(kSocketClosed, FlatJson{}.Set("fd", fd).Take()));
    }

    total_read += read;
  }

  return ResultT::Ok(std::string{buffer.data(), total_read});
}
