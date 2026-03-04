#include <iostream>
#include "Registry.h"
#include "Entity.h"
#include "PositionComponent.h"
#include "SpriteComponent.h"

using namespace gspengine::ecs;

int main() {
  std::cout << "Registry Tests \n";

  //create registry
  Registry registry;

  //create entities
  EntityID player = registry.CreateEntity();
  EntityID enemy = registry.CreateEntity();
  std::cout << "created entities:" << player << ", "
    << enemy << "\n";

  //add pos comps to player
  PositionComponent playerPos(100.0f, 200.0f);
  registry.AddComponent<PositionComponent>(player, playerPos);
  std::cout << "Added position to entity " << player << "\n";

  //check if player has pos
  if(registry.HasComponent<PositionComponent>(player)) {
    std::cout << "Entity" << player << "has position: YES\n";
  }

  //get pos comps
  PositionComponent& pos = registry.GetComponent<PositionComponent>(player);
  std::cout << "Player pos: " << pos.x << ", " << pos.y << "\n";


  //modify pos comps
  pos.x += 50.0f;
  pos.y += 12.0f;
  std::cout << "After moving: " << pos.x << ", " << pos.y << "\n";

  //check if enemy has pos comps(shouldnt)
  if(!registry.HasComponent<PositionComponent>(enemy)){
    std::cout << "Entity " << enemy << " has Posistion : NO \n";
  }

  SpriteComponent playerSprite(1, 64.0f, 64.0f);
  registry.AddComponent<SpriteComponent>(player, playerSprite);
  std::cout << "Added Sprite to entity " << player << "\n";

  std::cout << "\n Tests Passed \n";
  return 0;

}
