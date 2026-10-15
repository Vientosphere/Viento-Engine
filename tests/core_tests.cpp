#include "CameraController.h"
#include "Entity.h"
#include "Scene.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {
bool NearlyEqual(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) <= epsilon;
}

void TestEntityDefaultsAndBounds() {
    Entity entity{};
    assert(NearlyEqual(entity.pozisyon.x, 0.0f));
    assert(NearlyEqual(entity.boyut.x, 1.0f));
    assert(!entity.cizgiler);

    entity.pozisyon = { 2.0f, 3.0f, 4.0f };
    entity.boyut = { 2.0f, 4.0f, 6.0f };
    const BoundingBox bounds = entity.GetBoundingBox();
    assert(NearlyEqual(bounds.min.x, 1.0f));
    assert(NearlyEqual(bounds.min.y, 1.0f));
    assert(NearlyEqual(bounds.min.z, 1.0f));
    assert(NearlyEqual(bounds.max.x, 3.0f));
    assert(NearlyEqual(bounds.max.y, 5.0f));
    assert(NearlyEqual(bounds.max.z, 7.0f));

    entity.boyut = { -2.0f, -4.0f, -6.0f };
    const BoundingBox normalized = entity.GetBoundingBox();
    assert(NearlyEqual(normalized.min.x, 1.0f));
    assert(NearlyEqual(normalized.max.z, 7.0f));
}

void TestCameraLimitsAndInitialDirection() {
    CameraController camera;
    const Camera3D initial = camera.GetCamera();
    const Vector3 initialDirection = Vector3Normalize(Vector3Subtract(initial.target, initial.position));
    const Vector3 towardOrigin = Vector3Normalize(Vector3Subtract({0.0f, 0.0f, 0.0f}, initial.position));
    assert(NearlyEqual(initialDirection.x, towardOrigin.x));
    assert(NearlyEqual(initialDirection.y, towardOrigin.y));
    assert(NearlyEqual(initialDirection.z, towardOrigin.z));

    camera.AdjustFov(-1000.0f);
    assert(NearlyEqual(camera.GetCamera().fovy, 30.0f));
    camera.AdjustFov(1000.0f);
    assert(NearlyEqual(camera.GetCamera().fovy, 110.0f));

    camera.AdjustSpeed(-1000.0f);
    assert(NearlyEqual(camera.GetSpeed(), 1.0f));
    camera.AdjustSpeed(1000.0f);
    assert(NearlyEqual(camera.GetSpeed(), 50.0f));

    camera.Rotate({ 0.0f, 100000.0f });
    assert(camera.GetPitch() >= -89.0f && camera.GetPitch() <= 89.0f);
}

void TestSceneSelectionAndClear() {
    Scene scene;
    Entity farEntity{};
    farEntity.pozisyon = { 0.0f, 0.0f, 0.0f };
    farEntity.boyut = { 2.0f, 2.0f, 2.0f };
    Entity nearEntity = farEntity;
    nearEntity.pozisyon = { 0.0f, 0.0f, -4.0f };
    scene.Add(farEntity);
    scene.Add(nearEntity);

    scene.Select({ { 0.0f, 0.0f, -10.0f }, { 0.0f, 0.0f, 1.0f } });
    assert(scene.GetSelectedIndex() == 1);
    assert(scene.GetSelectedEntity() != nullptr);
    scene.ClearSelection();
    assert(scene.GetSelectedIndex() == -1);
    assert(scene.GetSelectedEntity() == nullptr);
}
}

int main() {
    TestEntityDefaultsAndBounds();
    TestCameraLimitsAndInitialDirection();
    TestSceneSelectionAndClear();
    std::cout << "Core tests passed\n";
    return 0;
}
