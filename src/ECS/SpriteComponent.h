#pragma once
#include "Component.h"
#include <cstdint>

namespace gspengine {
namespace ecs {

struct SpriteComponent : Component {
    uint32_t textureID = 0;
    float width = 0.0f;
    float height = 0.0f;

    SpriteComponent() = default;
    SpriteComponent(uint32_t texID, float w, float h) 
        : textureID(texID), width(w), height(h) {}
};

} // namespace ecs
} // namespace gspengine
