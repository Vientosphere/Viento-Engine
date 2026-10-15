# 🌪️ Viento Engine

Viento Engine, C++17 ve Raylib ile geliştirilen hafif bir 3D viewport / oyun motoru prototipidir.

## Mevcut özellikler

- Raylib tabanlı 3D viewport, ızgara ve yön pusulası
- Raylib `rlgl` depth-texture + GLSL shadow mapping; sahne nesneleri birbirine ve zemine gölge düşürür
- Işık yönü sahne genelindedir; nesne seçimine bağlı değildir (ok tuşlarıyla ayarlanır)
- Sağ fare tuşuyla serbest kamera; WASD ve Q/E hareketi
- Fare tekerleğiyle kamera hızı, `[` ve `]` ile FOV ayarı
- Ray üzerinden en yakın nesneyi seçme ve seçim gizmosu
- Kamera denetimi, sahne/seçim ve gölge render sistemi ayrı bileşenlerde
- Güvenli Entity varsayılanları ve idempotent pencere kapatma

## Gereksinimler

- CMake 3.20+
- C++17 destekli derleyici (GCC/Clang/MSVC)
- Git (CMake, Raylib 5.5 kaynağını `FetchContent` ile indirir)
- Linux'ta Raylib için OpenGL/X11 ve ALSA geliştirme paketleri

## Derleme

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Windows üzerinde de CMake ve uygun C++ derleyicisiyle aynı komutlar kullanılabilir. İlk yapılandırmada Raylib kaynakları indirilir; çevrimdışı derleme için bağımlılığı önceden sağlayıp CMake yapılandırmasını buna göre uyarlayın.

## Testler

`viento_core_tests`, Entity sınır kutularını ve varsayılanlarını, kamera yönü ve limitlerini, sahne seçimi/temizleme davranışını kontrol eder. GitHub Actions her push ve pull request'te derleme ve test çalıştırır.

## Yol haritası

- [ ] Transform ve nesne hiyerarşisi
- [ ] Bullet3 fizik entegrasyonu
- [ ] Daha kapsamlı giriş ve viewport testleri
