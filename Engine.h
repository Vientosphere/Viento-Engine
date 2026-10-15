#pragma once

#include "CameraController.h"
#include "Scene.h"
#include "ShadowRenderer.h"

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
    bool nesneSurukleniyor = false;
    Vector3 suruklemeBaslangicKonumu{};
    Vector3 suruklemeBaslangicNoktasi{};
    Vector3 suruklemeDuzlemiNormali{};

    CameraController kameraKontrol;
    Scene sahne;
    ShadowRenderer shadowRenderer;
};
