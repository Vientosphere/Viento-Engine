#include "Scene.h"

#include <limits>

void Scene::Add(const Entity& entity) {
    entities.push_back(entity);
}

const Entity* Scene::GetSelectedEntity() const {
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(entities.size())) {
        return nullptr;
    }
    return &entities[static_cast<std::size_t>(selectedIndex)];
}

Entity* Scene::GetSelectedEntity() {
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(entities.size())) {
        return nullptr;
    }
    return &entities[static_cast<std::size_t>(selectedIndex)];
}

void Scene::ClearSelection() {
    selectedIndex = -1;
}

void Scene::Select(const Ray& ray) {
    selectedIndex = -1;
    float nearestDistance = std::numeric_limits<float>::max();

    for (std::size_t i = 0; i < entities.size(); ++i) {
        const RayCollision collision = GetRayCollisionBox(ray, entities[i].GetBoundingBox());
        if (collision.hit && collision.distance >= 0.0f && collision.distance < nearestDistance) {
            nearestDistance = collision.distance;
            selectedIndex = static_cast<int>(i);
        }
    }
}
