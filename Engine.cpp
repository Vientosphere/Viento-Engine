#include "Engine.h"
#include "raymath.h"

#include <algorithm>

namespace {
constexpr float kMoveUnitsPerSecond = 1.0f;
}

Engine::~Engine() {
    Shutdown();
}

void Engine::Init(int genislik, int yukseklik, const char* baslik) {
    if (pencereHazir || genislik <= 0 || yukseklik <= 0) return;

    ekranGenisligi = genislik;
    ekranYuksekligi = yukseklik;
    InitWindow(ekranGenisligi, ekranYuksekligi, baslik);
    pencereHazir = IsWindowReady();
    if (!pencereHazir) return;

    SetTargetFPS(60);
    // A failed shader/FBO initialization leaves the viewport usable without shadows.
    shadowRenderer.Init();
    calisiyorMu = true;
}

void Engine::Shutdown() {
    calisiyorMu = false;
    if (pencereHazir) {
        shadowRenderer.Shutdown();
        if (IsCursorHidden()) EnableCursor();
        CloseWindow();
        pencereHazir = false;
    }
}

void Engine::NesneEkle(const Entity& yeniNesne) {
    sahne.Add(yeniNesne);
}

void Engine::Run() {
    if (!pencereHazir) return;
    while (calisiyorMu && !WindowShouldClose()) {
        Update();
        if (calisiyorMu) Render();
    }
    Shutdown();
}

void Engine::Update() {
    const float dt = GetFrameTime();

    kameraKontrol.AdjustSpeed(GetMouseWheelMove());
    if (IsKeyDown(KEY_LEFT_BRACKET)) kameraKontrol.AdjustFov(-20.0f * dt);
    if (IsKeyDown(KEY_RIGHT_BRACKET)) kameraKontrol.AdjustFov(20.0f * dt);

    // The light is a scene-wide render setting, never tied to entity selection.
    constexpr float lightStep = 0.65f;
    Vector2 lightDelta = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_LEFT)) lightDelta.x -= lightStep*dt;
    if (IsKeyDown(KEY_RIGHT)) lightDelta.x += lightStep*dt;
    if (IsKeyDown(KEY_UP)) lightDelta.y -= lightStep*dt;
    if (IsKeyDown(KEY_DOWN)) lightDelta.y += lightStep*dt;
    if (lightDelta.x != 0.0f || lightDelta.y != 0.0f) shadowRenderer.AdjustLight(lightDelta);

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) DisableCursor();

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        kameraKontrol.Rotate(GetMouseDelta());
        const float distance = kameraKontrol.GetSpeed() * dt * kMoveUnitsPerSecond;
        if (IsKeyDown(KEY_W)) kameraKontrol.MoveForward(distance);
        if (IsKeyDown(KEY_S)) kameraKontrol.MoveForward(-distance);
        if (IsKeyDown(KEY_D)) kameraKontrol.MoveRight(distance);
        if (IsKeyDown(KEY_A)) kameraKontrol.MoveRight(-distance);
        if (IsKeyDown(KEY_E)) kameraKontrol.MoveVertical(distance);
        if (IsKeyDown(KEY_Q)) kameraKontrol.MoveVertical(-distance);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) EnableCursor();

    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        sahne.Select(GetMouseRay(GetMousePosition(), kameraKontrol.GetCamera()));
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (sahne.GetSelectedIndex() != -1) sahne.ClearSelection();
        else calisiyorMu = false;
    }
}

void Engine::CizSagAltGizmo() {
    const Camera3D& camera = kameraKontrol.GetCamera();
    const Vector3 cameraDirection = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    Camera3D gizmoKamera = { 0 };
    gizmoKamera.position = Vector3Scale(cameraDirection, -3.5f);
    gizmoKamera.target = { 0.0f, 0.0f, 0.0f };
    gizmoKamera.up = { 0.0f, 1.0f, 0.0f };
    gizmoKamera.fovy = 50.0f;
    gizmoKamera.projection = CAMERA_PERSPECTIVE;

    constexpr int gizmoBoyut = 120;
    const int gizmoX = ekranGenisligi - gizmoBoyut - 20;
    const int gizmoY = ekranYuksekligi - gizmoBoyut - 20;

    DrawRectangleRounded({ (float)gizmoX, (float)gizmoY, (float)gizmoBoyut, (float)gizmoBoyut }, 0.2f, 4, { 20, 22, 28, 200 });
    DrawRectangleRoundedLines({ (float)gizmoX, (float)gizmoY, (float)gizmoBoyut, (float)gizmoBoyut }, 0.2f, 4, { 60, 65, 80, 255 });

    BeginScissorMode(gizmoX, gizmoY, gizmoBoyut, gizmoBoyut);
    BeginMode3D(gizmoKamera);
        DrawLine3D({ 0, 0, 0 }, { 1, 0, 0 }, RED);
        DrawSphere({ 1, 0, 0 }, 0.1f, RED);
        DrawLine3D({ 0, 0, 0 }, { 0, 1, 0 }, GREEN);
        DrawSphere({ 0, 1, 0 }, 0.1f, GREEN);
        DrawLine3D({ 0, 0, 0 }, { 0, 0, 1 }, BLUE);
        DrawSphere({ 0, 0, 1 }, 0.1f, BLUE);
        DrawSphere({ 0, 0, 0 }, 0.06f, WHITE);
    EndMode3D();
    EndScissorMode();

    DrawText("X", gizmoX + gizmoBoyut - 20, gizmoY + gizmoBoyut / 2 - 5, 12, RED);
    DrawText("Y", gizmoX + gizmoBoyut / 2 - 4, gizmoY + 8, 12, GREEN);
    DrawText("Z", gizmoX + 12, gizmoY + gizmoBoyut - 20, 12, BLUE);
}

void Engine::Render() {
    const Camera3D& camera = kameraKontrol.GetCamera();
    const auto& entities = sahne.GetEntities();
    const int selectedIndex = sahne.GetSelectedIndex();

    // Pass 1: populate the depth texture from the light's point of view.
    shadowRenderer.RenderShadowMap(entities);

    BeginDrawing();
    ClearBackground({ 30, 31, 36, 255 });

    // Draw editor-only lines with Raylib's default shader, not the lighting shader.
    BeginMode3D(camera);
        DrawGrid(24, 1.0f);
    EndMode3D();

    // Pass 2: the same geometry is rendered from the camera and compared with the depth map.
    shadowRenderer.BeginMainPass(camera);
    BeginMode3D(camera);
        shadowRenderer.DrawGround();
        for (std::size_t i = 0; i < entities.size(); ++i) {
            shadowRenderer.DrawEntity(entities[i]);
        }
    EndMode3D();
    shadowRenderer.EndMainPass();

    // Editor overlays use Raylib's default shader and never affect the shadow map.
    BeginMode3D(camera);
        for (std::size_t i = 0; i < entities.size(); ++i) {
            const Entity& entity = entities[i];
            const bool selected = static_cast<int>(i) == selectedIndex;
            if (selected) {
                DrawCubeWires(entity.pozisyon, std::fabs(entity.boyut.x),
                              std::fabs(entity.boyut.y), std::fabs(entity.boyut.z), ORANGE);
                const Vector3 pos = entity.pozisyon;
                constexpr float axisLength = 2.5f;
                DrawLine3D(pos, { pos.x + axisLength, pos.y, pos.z }, RED);
                DrawSphere({ pos.x + axisLength, pos.y, pos.z }, 0.15f, RED);
                DrawLine3D(pos, { pos.x, pos.y + axisLength, pos.z }, GREEN);
                DrawSphere({ pos.x, pos.y + axisLength, pos.z }, 0.15f, GREEN);
                DrawLine3D(pos, { pos.x, pos.y, pos.z + axisLength }, BLUE);
                DrawSphere({ pos.x, pos.y, pos.z + axisLength }, 0.15f, BLUE);
            } else if (entity.cizgiler) {
                DrawCubeWires(entity.pozisyon, std::fabs(entity.boyut.x),
                              std::fabs(entity.boyut.y), std::fabs(entity.boyut.z), entity.cizgiRengi);
            }
        }
    EndMode3D();

    DrawText("Viento Engine 3D - Viewport", 20, 20, 20, RAYWHITE);
    DrawText("Sol tik: Obje sec | Sag tik + WASD/QE: Kamera | Ok tuslari: Isik yonu", 20, 48, 14, LIGHTGRAY);
    DrawText(TextFormat("Kamera Hizi: %.1fx | FOV: %.0f ([ / ] ile ayarla) | Golge: %s",
                        kameraKontrol.GetSpeed(), camera.fovy,
                        shadowRenderer.IsReady() ? "Acik" : "Kapali (shader/FBO)"), 20, 70, 14, YELLOW);

    if (const Entity* selected = sahne.GetSelectedEntity()) {
        const Vector3 pos = selected->pozisyon;
        DrawText(TextFormat("Secili Obje #%i | Konum: (X: %.2f, Y: %.2f, Z: %.2f)",
                            selectedIndex, pos.x, pos.y, pos.z),
                 20, ekranYuksekligi - 40, 15, ORANGE);
    } else {
        DrawText("Secili Obje: Yok", 20, ekranYuksekligi - 40, 14, DARKGRAY);
    }

    CizSagAltGizmo();
    DrawFPS(ekranGenisligi - 90, 20);
    EndDrawing();
}
