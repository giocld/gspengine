#pragma once
#include <cstdint>
#include <type_traits>

namespace gspengine {
namespace ecs {

  struct Component {
    //TODO:
  };
  class ComponentType {
    public:
      template<typename T>
      static uint32_t GetTypeID() {
        static_assert(std::is_base_of_v<Component,T>,"T must inherit from Component");
        static uint32_t id = nextID_++;
        return id;
    }
      static uint32_t GetMaxComponents() {return nextID_; }

    private:
      inline static uint32_t nextID_ = 0;
  };
}
}
