#pragma once

#include "raylib.h"

// Owns camera orientation, movement speed, and field-of-view constraints.
// This class does not read input or create a window, so its math is testable.
class CameraController {
public:
    CameraController();

    const Camera3D& GetCamera() const { return camera; }
    float GetSpeed() const { return speed; }
    float GetYaw() const { return yaw; }
    float GetPitch() const { return pitch; }

    void Rotate(Vector2 mouseDelta);
    void AdjustSpeed(float wheelDelta);
    void AdjustFov(float delta);
    void MoveForward(float distance);
    void MoveRight(float distance);
    void MoveVertical(float distance);

private:
    void SyncTarget();

    Camera3D camera{};
    float speed = 10.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
};
