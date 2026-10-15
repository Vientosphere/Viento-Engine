#pragma once

#include "raylib.h"

#include <algorithm>

// A simple scene object with safe defaults. Dimensions are full extents.
struct Entity {
    Vector3 pozisyon{ 0.0f, 0.0f, 0.0f };
    Vector3 boyut{ 1.0f, 1.0f, 1.0f };
    Color renk{ 140, 145, 155, 255 };
    Color cizgiRengi{ 230, 235, 245, 255 };
    bool cizgiler = false;

    BoundingBox GetBoundingBox() const {
        const Vector3 half = {
            std::fabs(boyut.x) * 0.5f,
            std::fabs(boyut.y) * 0.5f,
            std::fabs(boyut.z) * 0.5f
        };
        return {
            { pozisyon.x - half.x, pozisyon.y - half.y, pozisyon.z - half.z },
            { pozisyon.x + half.x, pozisyon.y + half.y, pozisyon.z + half.z }
        };
    }

    void Ciz(bool seciliMi = false) const {
        DrawCube(pozisyon, std::fabs(boyut.x), std::fabs(boyut.y), std::fabs(boyut.z), renk);
        if (seciliMi) {
            DrawCubeWires(pozisyon, std::fabs(boyut.x), std::fabs(boyut.y), std::fabs(boyut.z), ORANGE);
        } else if (cizgiler) {
            DrawCubeWires(pozisyon, std::fabs(boyut.x), std::fabs(boyut.y), std::fabs(boyut.z), cizgiRengi);
        }
    }
};
