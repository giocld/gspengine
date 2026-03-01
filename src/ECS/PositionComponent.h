#pragma once
#include "Component.h"

namespace gspengine{
namespace ecs{

struct PositionComponent : Component {
  float x = 0.0f;
  float y = 0.0f;

  PositionComponent() = default;
  PositionComponent(float xPos, float yPos) : x(xPos),
  y(yPos) {}
  };

}
}
