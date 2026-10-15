#include "Engine.h"

int main() {
    Engine motor;
    motor.Init(1280, 720, "Viento Engine - 3D Viewport");

    Entity baslangicKupu{};
    baslangicKupu.pozisyon = { 0.0f, 1.0f, 0.0f };
    baslangicKupu.boyut = { 2.0f, 2.0f, 2.0f };
    baslangicKupu.renk = { 140, 145, 155, 255 };
    baslangicKupu.cizgiRengi = { 230, 235, 245, 255 };
    baslangicKupu.cizgiler = true;
    motor.NesneEkle(baslangicKupu);

    motor.Run();
    return 0;
}
