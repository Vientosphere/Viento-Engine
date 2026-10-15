#pragma once

#include "CameraController.h"
#include "Scene.h"

class Engine {
public:
    Engine() = default;
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void Init(int genislik, int yukseklik, const char* baslik);
    void Run();
    void Shutdown();
    void NesneEkle(const Entity& yeniNesne);

private:
    void Update();
    void Render();
    void CizSagAltGizmo();

    int ekranGenisligi = 1280;
    int ekranYuksekligi = 720;
    bool calisiyorMu = false;
    bool pencereHazir = false;

    CameraController kameraKontrol;
    Scene sahne;
};
