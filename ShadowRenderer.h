#pragma once

#include "Entity.h"
#include "raylib.h"

#include <vector>

// Directional shadow mapping built on Raylib's rlgl depth-texture framebuffer.
// Owns GPU resources; call Init/Shutdown while the Raylib window is alive.
class ShadowRenderer {
public:
    ShadowRenderer() = default;
    ~ShadowRenderer();

    ShadowRenderer(const ShadowRenderer&) = delete;
    ShadowRenderer& operator=(const ShadowRenderer&) = delete;

    bool Init();
    void Shutdown();
    bool IsReady() const { return ready; }

    void AdjustLight(Vector2 delta);
    void RenderShadowMap(const std::vector<Entity>& entities);
    void BeginMainPass(const Camera3D& camera);
    void DrawEntity(const Entity& entity) const;
    void EndMainPass();

private:
    bool CreateShadowMap(int width, int height);
    void DestroyShadowMap();

    static constexpr int kShadowResolution = 2048;
    RenderTexture2D shadowMap{};
    Model cubeModel{};
    Shader shadowShader{};
    Camera3D lightCamera{};
    Matrix lightView{};
    Matrix lightProjection{};
    Matrix lightViewProjection{};
    Vector3 lightDirection{ 0.45f, -1.0f, 0.35f };
    int lightDirectionLocation = -1;
    int lightColorLocation = -1;
    int ambientLocation = -1;
    int lightViewProjectionLocation = -1;
    int shadowMapLocation = -1;
    int shadowMapResolutionLocation = -1;
    int viewPositionLocation = -1;
    bool ready = false;
};
