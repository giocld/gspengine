#pragma once
#include "Entity.h"
#include "Component.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>

namespace gspengine {
namespace ecs {

class IComponentStorage;

template<typename T>
class SparseSet;

//base class so Registry can store different component types together
class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;
};

template<typename T>
class SparseSet : public IComponentStorage {
public:

    //add component to entity
    void Add(EntityID entity, const T& component) {

      if(entity >= sparse_.size()){
            sparse_.resize(entity +1, -1); //fill new slots with -1
        }
      if (Has(entity)){
        return;

      }
      size_t index = dense_.size(); //index where we are storing

      dense_.push_back(component); //add

      entities_.push_back(entity); //add entity to mapping

      sparse_[entity] = static_cast<int>(index);
    }

    //remove component from entity using swap-and-pop
    void Remove(EntityID entity) {
        //check if entity has component
        if (!Has(entity)) {
            return;
        }

        //get current dense index
        int indexToRemove = sparse_[entity];
        int lastIndex = static_cast<int>(dense_.size()) - 1;

        //if not the last element, swap with last
        if (indexToRemove != lastIndex) {
            //get the entity that's at the last position
            EntityID lastEntity = entities_[lastIndex];

            //move last element to the removed position
            dense_[indexToRemove] = std::move(dense_[lastIndex]);
            entities_[indexToRemove] = lastEntity;

            //update sparse for the moved entity
            sparse_[lastEntity] = indexToRemove;
        }

        //pop the last element (which is either the removed one or was swapped)
        dense_.pop_back();
        entities_.pop_back();

        //mark entity as not having this component
        sparse_[entity] = -1;
    }

    //get component reference (non-const)
    T& Get(EntityID entity) {
      assert(Has(entity));
      int index = sparse_[entity];
      return dense_[index];
    }

    //check if entity has this component
    bool Has(EntityID entity) const {
        // TODO: Check if entity index is valid in sparse
        if(entity >= sparse_.size()) {
          return false;
        }
        return sparse_[entity] != -1;
    }

    size_t Size() const {
      return dense_.size(); }

    //get entity at dense index
    EntityID GetEntity(size_t index) const {
        return entities_[index];
    }


    //access dense array directly (for systems to iterate)
    const std::vector<T>& GetDense() const { return dense_; }
    std::vector<T>& GetDense() { return dense_; }

private:
    //sparse array: indexed by EntityID, stores index into dense array (-1 = no component)
    std::vector<int> sparse_;

    //dense array: packed component data
    std::vector<T> dense_;

    //reverse mapping: dense index -> EntityID
    std::vector<EntityID> entities_;
};

class Registry {
public:
    Registry() = default;
    ~Registry() = default;

    EntityID CreateEntity() {
        return nextEntityID_++;
    }

    void DestroyEntity(EntityID entity) {
      //TODO add implementation with signatures
    }

    template<typename T>
    void AddComponent(EntityID entity, const T& component) {
        SparseSet<T>* storage = GetStorage<T>(); //create/find storage
        storage->Add(entity, component);
    }


    template<typename T>
    T& GetComponent(EntityID entity) {
      SparseSet<T>* storage = GetStorage<T>();
      return storage->Get(entity);
    }

    template<typename T>
    bool HasComponent(EntityID entity) const {
      const SparseSet<T>* storage = GetStorage<T>();
      if (!storage) return false;
      return storage->Has(entity);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        SparseSet<T>* storage = GetStorage<T>();
        if (storage) {
            storage->Remove(entity);
        }
    }

    //get all entities with a component
    template<typename T>
    SparseSet<T>* GetComponentStorage() {
        return GetStorage<T>();
    }

private:
    //entity counter for ID generation
    EntityID nextEntityID_ = 1; //start at 1, 0 is INVALID_ENTITY

    //component storage: type ID -> storage pointer
    //we use type erasure to store different SparseSet<T> types together
    std::unordered_map<uint32_t, std::unique_ptr<IComponentStorage>> componentStorages_;

    //helper to get or create storage for a component type
    template<typename T>
    SparseSet<T>* GetStorage() {
        uint32_t typeID = ComponentType::GetTypeID<T>();

        //check if storage exists
        auto it = componentStorages_.find(typeID);
        if (it == componentStorages_.end()) {
            //create new storage
            auto storage = std::make_unique<SparseSet<T>>();
            SparseSet<T>* ptr = storage.get();
            componentStorages_[typeID] = std::move(storage);
            return ptr;
        }

        //cast back from base pointer
        return static_cast<SparseSet<T>*>(it->second.get());
    }

    //const version for HasComponent (doesn't create storage)
    template<typename T>
    const SparseSet<T>* GetStorage() const {
        uint32_t typeID = ComponentType::GetTypeID<T>();
        auto it = componentStorages_.find(typeID);
        if (it != componentStorages_.end()) {
            return static_cast<const SparseSet<T>*>(it->second.get());
        }
        return nullptr;
    }
};

}
}
