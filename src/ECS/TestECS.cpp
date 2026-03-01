#include <iostream>
#include "Entity.h"
#include "Component.h"
#include "PositionComponent.h"
#include "SpriteComponent.h"
#include "VelocityComponent.h"

using namespace gspengine::ecs;

void TestECS(){
  //test entityid
  EntityID player = 1;
  std::cout << "Player entity ID: " << player << "\n";


  //test component type ids
  uint32_t posID = ComponentType::GetTypeID<PositionComponent>(); //pos
  uint32_t spriteID = ComponentType::GetTypeID<SpriteComponent>();  //sprite
  uint32_t velID = ComponentType::GetTypeID<VelocityComponent>(); //vel

  std::cout << "PositionComponent ID: " << posID << "\n";
  std::cout << "SpriteComponent ID: " << spriteID << "\n";
  std::cout << "VelocityComponent ID: " << velID << "\n";
  std::cout << "Total component types: " << ComponentType::GetMaxComponents() << "\n";

    // Test creating components
    PositionComponent pos(100.0f, 200.0f);
    SpriteComponent sprite(1, 64.0f, 64.0f);
    VelocityComponent vel(50.0f, 0.0f);

    std::cout << "Position: " << pos.x << ", " << pos.y << "\n";
    std::cout << "Sprite size: " << sprite.width << "x" << sprite.height << "\n";
    std::cout << "Velocity: " << vel.vx << ", " << vel.vy << "\n";



}
