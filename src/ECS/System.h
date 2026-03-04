#pragma once
#include "Registry.h"

namespace gspengine {
namespace ecs {

//all systems inherit from this
//systems process entities with specific components each frame
class System {
public:
    virtual ~System() = default;

    //called every frame
    virtual void Update(float dt, Registry& registry) = 0;
};

}
}
