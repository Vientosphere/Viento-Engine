#include "CameraController.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kMinSpeed = 1.0f;
constexpr float kMaxSpeed = 50.0f;
constexpr float kMinFov = 30.0f;
constexpr float kMaxFov = 110.0f;
constexpr float kMaxPitch = 89.0f;
constexpr float kMouseSensitivity = 0.15f;
constexpr Vector3 kWorldUp = { 0.0f, 1.0f, 0.0f };
}

CameraController::CameraController() {
    camera.position = { 7.0f, 6.0f, 7.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = kWorldUp;
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Derive the angles from the initial view direction to avoid a jump
    // the first time the user holds the right mouse button.
    const Vector3 direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    yaw = atan2f(direction.x, direction.z) * RAD2DEG;
    pitch = asinf(std::clamp(direction.y, -1.0f, 1.0f)) * RAD2DEG;
    SyncTarget();
}

void CameraController::SyncTarget() {
    const float yawRad = yaw * DEG2RAD;
    const float pitchRad = pitch * DEG2RAD;
    const Vector3 forward = Vector3Normalize({
        cosf(pitchRad) * sinf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * cosf(yawRad)
    });
    camera.target = Vector3Add(camera.position, forward);
}

void CameraController::Rotate(Vector2 mouseDelta) {
    yaw -= mouseDelta.x * kMouseSensitivity;
    pitch = std::clamp(pitch - mouseDelta.y * kMouseSensitivity, -kMaxPitch, kMaxPitch);
    // Keep yaw numerically bounded during long editing sessions.
    if (yaw > 180.0f || yaw < -180.0f) {
        yaw = std::fmod(yaw + 180.0f, 360.0f);
        if (yaw < 0.0f) yaw += 360.0f;
        yaw -= 180.0f;
    }
    SyncTarget();
}

void CameraController::AdjustSpeed(float wheelDelta) {
    speed = std::clamp(speed + wheelDelta * 2.5f, kMinSpeed, kMaxSpeed);
}

void CameraController::AdjustFov(float delta) {
    camera.fovy = std::clamp(camera.fovy + delta, kMinFov, kMaxFov);
}

void CameraController::MoveForward(float distance) {
    const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    camera.position = Vector3Add(camera.position, Vector3Scale(forward, distance));
    camera.target = Vector3Add(camera.target, Vector3Scale(forward, distance));
}

void CameraController::MoveRight(float distance) {
    const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    camera.position = Vector3Add(camera.position, Vector3Scale(right, distance));
    camera.target = Vector3Add(camera.target, Vector3Scale(right, distance));
}

void CameraController::MoveVertical(float distance) {
    const Vector3 offset = Vector3Scale(kWorldUp, distance);
    camera.position = Vector3Add(camera.position, offset);
    camera.target = Vector3Add(camera.target, offset);
}
