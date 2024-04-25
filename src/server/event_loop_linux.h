#ifndef SERVER_EVENT_LOOP_LINUX_H
#define SERVER_EVENT_LOOP_LINUX_H

#include <functional>

#include "core/core.h"
#include "core/spsc_channel.h"

#include "file_descriptor_linux.h"

using EventLoopLinuxEvent = std::unordered_map<std::string, std::string>;

using OnEventLoopEvent =
    std::function<Result<core::Void>(const EventLoopLinuxEvent &)>;
using OnEpollEvent =
    std::function<Result<core::Void>(const struct epoll_event &)>;

class EventLoopLinux final : private core::NonCopyable, core::Movable {
public:
  [[nodiscard]] auto Add(const FileDescriptorLinux::Raw fd,
                         uint32_t events) noexcept -> Result<core::Void>;
  [[nodiscard]] auto Delete(const FileDescriptorLinux::Raw fd) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto Run(OnEventLoopEvent &&on_event_loop_event,
                         OnEpollEvent &&on_epoll_event) noexcept
      -> Result<core::Void>;

private:
  explicit EventLoopLinux(
      std::vector<core::Rx<EventLoopLinuxEvent>> &&event_loop_rxs,
      FileDescriptorLinux &&epoll_fd) noexcept;

  std::vector<core::Rx<EventLoopLinuxEvent>> event_loop_rxs_;
  FileDescriptorLinux epoll_fd_;

  static constexpr size_t kMaxEvents = 1024;

  friend class EventLoopLinuxBuilder;
};

class EventLoopLinuxBuilder final : private core::NonCopyable,
                                    core::NonMovable {
public:
  [[nodiscard]] auto
  Build(std::vector<core::Rx<EventLoopLinuxEvent>> &&event_loop_rxs)
      const noexcept -> Result<EventLoopLinux>;
};

#endif // SERVER_EVENT_LOOP_LINUX_H