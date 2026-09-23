#pragma once
#include <cstdint>
#include "imgui.h"

struct VisualTheme {
    const char* name;
    float soundPitch;
    bool isRetro;
    ImU32 bgWindow;
    ImU32 cellCovered;
    ImU32 cellCoveredHover;
    ImU32 cellCoveredBorder;
    ImU32 cellRevealed;
    ImU32 cellRevealedBorder;
    ImU32 accentGlow;
    ImU32 flagColor;
    ImU32 mineColor;
    ImU32 pillBtnNormal;
    ImU32 pillBtnHover;
    ImU32 pillBtnActive;
    ImU32 pillBtnBorder;
    ImU32 digitColors[9];
};

static VisualTheme Themes[] = {
    {
        "Cyber Emerald",
        1.15f,
        false,
        IM_COL32(6, 16, 12, 255),
        IM_COL32(14, 40, 26, 255),
        IM_COL32(0, 220, 110, 255),
        IM_COL32(0, 180, 90, 200),
        IM_COL32(10, 26, 18, 255),
        IM_COL32(20, 55, 36, 255),
        IM_COL32(0, 255, 130, 255),
        IM_COL32(0, 255, 140, 255),
        IM_COL32(255, 50, 50, 255),
        IM_COL32(24, 75, 48, 255),
        IM_COL32(35, 140, 85, 255),
        IM_COL32(50, 180, 110, 255),
        IM_COL32(70, 255, 150, 255),
        { IM_COL32(0,0,0,0), IM_COL32(0, 255, 150, 255), IM_COL32(120, 255, 0, 255), IM_COL32(255, 215, 0, 255), IM_COL32(255, 128, 0, 255), IM_COL32(255, 0, 90, 255), IM_COL32(200, 0, 255, 255), IM_COL32(130, 0, 255, 255), IM_COL32(255, 255, 255, 255) }
    },
    {
        "Cyberpunk Neon",
        1.00f,
        false,
        IM_COL32(10, 12, 22, 255),
        IM_COL32(22, 30, 50, 255),
        IM_COL32(0, 220, 255, 255),
        IM_COL32(0, 150, 220, 200),
        IM_COL32(14, 18, 28, 255),
        IM_COL32(35, 45, 70, 255),
        IM_COL32(255, 0, 128, 255),
        IM_COL32(255, 0, 140, 255),
        IM_COL32(255, 40, 40, 255),
        IM_COL32(26, 50, 100, 255),
        IM_COL32(0, 130, 210, 255),
        IM_COL32(0, 175, 255, 255),
        IM_COL32(0, 240, 255, 255),
        { IM_COL32(0,0,0,0), IM_COL32(0, 230, 255, 255), IM_COL32(0, 255, 128, 255), IM_COL32(255, 220, 0, 255), IM_COL32(255, 128, 0, 255), IM_COL32(255, 0, 90, 255), IM_COL32(210, 0, 255, 255), IM_COL32(130, 0, 255, 255), IM_COL32(255, 255, 255, 255) }
    },
    {
        "Classic Win95",
        0.85f,
        true,
        IM_COL32(0, 128, 128, 255),
        IM_COL32(192, 192, 192, 255),
        IM_COL32(210, 210, 210, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(170, 170, 170, 255),
        IM_COL32(128, 128, 128, 255),
        IM_COL32(0, 0, 128, 255),
        IM_COL32(220, 0, 0, 255),
        IM_COL32(0, 0, 0, 255),
        IM_COL32(192, 192, 192, 255),
        IM_COL32(215, 215, 215, 255),
        IM_COL32(160, 160, 160, 255),
        IM_COL32(0, 0, 0, 255),
        { IM_COL32(0,0,0,0), IM_COL32(0, 0, 255, 255), IM_COL32(0, 128, 0, 255), IM_COL32(255, 0, 0, 255), IM_COL32(0, 0, 128, 255), IM_COL32(128, 0, 0, 255), IM_COL32(0, 128, 128, 255), IM_COL32(0, 0, 0, 255), IM_COL32(128, 128, 128, 255) }
    },
    {
        "Sunset Vaporwave",
        0.95f,
        false,
        IM_COL32(22, 10, 36, 255),
        IM_COL32(46, 20, 66, 255),
        IM_COL32(255, 105, 190, 255),
        IM_COL32(200, 60, 160, 200),
        IM_COL32(30, 16, 48, 255),
        IM_COL32(65, 30, 90, 255),
        IM_COL32(255, 110, 200, 255),
        IM_COL32(255, 210, 0, 255),
        IM_COL32(255, 40, 120, 255),
        IM_COL32(85, 28, 95, 255),
        IM_COL32(155, 45, 145, 255),
        IM_COL32(210, 75, 175, 255),
        IM_COL32(255, 130, 220, 255),
        { IM_COL32(0,0,0,0), IM_COL32(0, 230, 255, 255), IM_COL32(255, 120, 220, 255), IM_COL32(255, 180, 0, 255), IM_COL32(255, 90, 90, 255), IM_COL32(180, 90, 255, 255), IM_COL32(120, 180, 255, 255), IM_COL32(255, 255, 255, 255), IM_COL32(255, 220, 180, 255) }
    },
    {
        "Deep Space",
        1.25f,
        false,
        IM_COL32(8, 12, 22, 255),
        IM_COL32(18, 28, 46, 255),
        IM_COL32(80, 165, 255, 255),
        IM_COL32(40, 100, 180, 200),
        IM_COL32(12, 18, 30, 255),
        IM_COL32(28, 42, 68, 255),
        IM_COL32(0, 165, 255, 255),
        IM_COL32(255, 90, 90, 255),
        IM_COL32(255, 215, 0, 255),
        IM_COL32(22, 44, 78, 255),
        IM_COL32(38, 80, 135, 255),
        IM_COL32(55, 115, 185, 255),
        IM_COL32(95, 175, 255, 255),
        { IM_COL32(0,0,0,0), IM_COL32(100, 200, 255, 255), IM_COL32(100, 255, 180, 255), IM_COL32(255, 230, 100, 255), IM_COL32(255, 150, 100, 255), IM_COL32(255, 100, 150, 255), IM_COL32(200, 120, 255, 255), IM_COL32(150, 180, 255, 255), IM_COL32(255, 255, 255, 255) }
    }
};
