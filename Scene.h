#pragma once

#include "Entity.h"
#include <vector>

// Owns scene entities and the current selection, independently of rendering.
class Scene {
public:
    void Add(const Entity& entity);
    const std::vector<Entity>& GetEntities() const { return entities; }
    int GetSelectedIndex() const { return selectedIndex; }
    Entity* GetSelectedEntity();
    const Entity* GetSelectedEntity() const;

    void ClearSelection();
    void Select(const Ray& ray);

private:
    std::vector<Entity> entities;
    int selectedIndex = -1;
};
