#pragma once
#include "Component.h"

namespace gspengine {
namespace ecs {
  struct VelocityComponent : Component {
    float vx = 0.0f; //pixels per second velX
    float vy = 0.0f; //pixels per second velY

    VelocityComponent() = default;
    VelocityComponent(float velX, float velY) : vx(velX),
  vy(velY) {}
  };
}
}
