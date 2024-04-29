#include "service.h"

using namespace kero;

kero::Service::Service(const Kind kind,
                       std::vector<Kind>&& dependencies) noexcept
    : kind_{kind}, dependencies_{std::move(dependencies)} {}

auto
kero::Service::GetKind() const noexcept -> Kind {
  return kind_;
}

auto
kero::Service::GetDependencies() const noexcept -> const std::vector<Kind>& {
  return dependencies_;
}

auto
kero::Service::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  return ResultT::Ok(Void{});
}

auto
kero::Service::OnDestroy(Agent& agent) noexcept -> void {
  // noop
}

auto
kero::Service::OnUpdate(Agent& agent) noexcept -> void {
  // noop
}

auto
kero::Service::OnEvent(Agent& agent,
                       const std::string& event,
                       const Dict& data) noexcept -> void {
  // noop
}