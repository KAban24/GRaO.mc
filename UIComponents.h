#pragma once
#include <cmath>
#include <algorithm>
#include <string>
#include "imgui.h"
#include "Theme.h"
#include "SoundEngine.h"

inline float EaseOutCubic(float x) {
    return 1.0f - std::pow(1.0f - x, 3.0f);
}

inline float EaseOutExpo(float x) {
    return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);
}

inline float SpringOvershoot(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    float decay = std::exp(-5.5f * t);
    return 1.0f - decay * std::cos(7.5f * t);
}

inline void DrawCrispShadow(ImDrawList* draw, ImVec2 min, ImVec2 max, float round, ImU32 color) {
    draw->AddRectFilled(ImVec2(min.x - 5.0f, min.y - 2.0f), ImVec2(max.x + 5.0f, max.y + 8.0f), (color & 0x00FFFFFF) | 0x33000000, round + 4.0f);
    draw->AddRectFilled(ImVec2(min.x - 2.0f, min.y), ImVec2(max.x + 2.0f, max.y + 4.0f), (color & 0x00FFFFFF) | 0x55000000, round + 2.0f);
}

inline void DrawCyberPanel(ImVec2 min, ImVec2 max, ImU32 bg, ImU32 border, float round = 14.0f, bool isRetro = false) {
    ImDrawList* draw = ImGui::GetWindowDrawList();

    if (isRetro) {
        draw->AddRectFilled(min, max, bg);
        draw->AddLine(min, ImVec2(max.x, min.y), IM_COL32(255, 255, 255, 255), 2.5f);
        draw->AddLine(min, ImVec2(min.x, max.y), IM_COL32(255, 255, 255, 255), 2.5f);
        draw->AddLine(ImVec2(min.x, max.y), max, IM_COL32(0, 0, 0, 255), 2.5f);
        draw->AddLine(ImVec2(max.x, min.y), max, IM_COL32(0, 0, 0, 255), 2.5f);

        draw->AddLine(ImVec2(min.x + 1, max.y - 1), ImVec2(max.x - 1, max.y - 1), IM_COL32(128, 128, 128, 255), 1.5f);
        draw->AddLine(ImVec2(max.x - 1, min.y + 1), ImVec2(max.x - 1, max.y - 1), IM_COL32(128, 128, 128, 255), 1.5f);

        float barH = 26.0f;
        ImVec2 tbMin = ImVec2(min.x + 4.0f, min.y + 4.0f);
        ImVec2 tbMax = ImVec2(max.x - 4.0f, min.y + 4.0f + barH);
        draw->AddRectFilledMultiColor(tbMin, tbMax, IM_COL32(0, 0, 128, 255), IM_COL32(16, 132, 208, 255),
                                      IM_COL32(16, 132, 208, 255), IM_COL32(0, 0, 128, 255));

        float btnW = 16.0f, btnH = 14.0f;
        ImVec2 cMin = ImVec2(tbMax.x - btnW - 3.0f, tbMin.y + 6.0f);
        ImVec2 cMax = ImVec2(cMin.x + btnW, cMin.y + btnH);
        draw->AddRectFilled(cMin, cMax, IM_COL32(192, 192, 192, 255));
        draw->AddLine(cMin, ImVec2(cMax.x, cMin.y), IM_COL32(255, 255, 255, 255), 1.5f);
        draw->AddLine(cMin, ImVec2(cMin.x, cMax.y), IM_COL32(255, 255, 255, 255), 1.5f);
        draw->AddLine(ImVec2(cMin.x, cMax.y), cMax, IM_COL32(0, 0, 0, 255), 1.5f);
        draw->AddLine(ImVec2(cMax.x, cMin.y), cMax, IM_COL32(0, 0, 0, 255), 1.5f);
        draw->AddLine(ImVec2(cMin.x + 3, cMin.y + 3), ImVec2(cMax.x - 3, cMax.y - 3), IM_COL32(0, 0, 0, 255), 1.8f);
        draw->AddLine(ImVec2(cMax.x - 3, cMin.y + 3), ImVec2(cMin.x + 3, cMax.y - 3), IM_COL32(0, 0, 0, 255), 1.8f);
        return;
    }

    DrawCrispShadow(draw, min, max, round, border);

    draw->AddRectFilled(min, max, bg, round);
    draw->AddRect(min, max, border, round, 0, 2.0f);

    draw->AddLine(ImVec2(min.x + round, min.y + 1.0f), ImVec2(max.x - round, min.y + 1.0f), IM_COL32(255, 255, 255, 55), 1.2f);

    float bracketLen = 16.0f;
    ImU32 bracketCol = border | 0xFF000000;

    draw->AddLine(ImVec2(min.x + 4, min.y + 4), ImVec2(min.x + 4 + bracketLen, min.y + 4), bracketCol, 2.5f);
    draw->AddLine(ImVec2(min.x + 4, min.y + 4), ImVec2(min.x + 4, min.y + 4 + bracketLen), bracketCol, 2.5f);

    draw->AddLine(ImVec2(max.x - 4, min.y + 4), ImVec2(max.x - 4 - bracketLen, min.y + 4), bracketCol, 2.5f);
    draw->AddLine(ImVec2(max.x - 4, min.y + 4), ImVec2(max.x - 4, min.y + 4 + bracketLen), bracketCol, 2.5f);

    draw->AddLine(ImVec2(min.x + 4, max.y - 4), ImVec2(min.x + 4 + bracketLen, max.y - 4), bracketCol, 2.5f);
    draw->AddLine(ImVec2(min.x + 4, max.y - 4), ImVec2(min.x + 4, max.y - 4 - bracketLen), bracketCol, 2.5f);

    draw->AddLine(ImVec2(max.x - 4, max.y - 4), ImVec2(max.x - 4 - bracketLen, max.y - 4), bracketCol, 2.5f);
    draw->AddLine(ImVec2(max.x - 4, max.y - 4), ImVec2(max.x - 4, max.y - 4 - bracketLen), bracketCol, 2.5f);
}

inline bool DrawPillButton(const char* label, ImVec2 pos, ImVec2 size, const VisualTheme& theme, bool isSelected, ImFont* font, float animTime = 0.0f, bool isRetro = false) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    bool isHovered = (mousePos.x >= pos.x && mousePos.x <= pos.x + size.x && mousePos.y >= pos.y && mousePos.y <= pos.y + size.y);
    if (isHovered) isSelected = true;

    if (isRetro) {
        float pop = isSelected ? 1.5f : 0.0f;
        ImVec2 rPos = ImVec2(pos.x - pop, pos.y - pop);
        ImVec2 rMax = ImVec2(pos.x + size.x + pop, pos.y + size.y + pop);

        draw->AddRectFilled(rPos, rMax, isSelected ? theme.pillBtnHover : theme.pillBtnNormal);

        ImU32 colTopLeft = isSelected ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255);
        ImU32 colBotRight = isSelected ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255);

        draw->AddLine(rPos, ImVec2(rMax.x, rPos.y), colTopLeft, 2.5f);
        draw->AddLine(rPos, ImVec2(rPos.x, rMax.y), colTopLeft, 2.5f);
        draw->AddLine(ImVec2(rPos.x, rMax.y), rMax, colBotRight, 2.5f);
        draw->AddLine(ImVec2(rMax.x, rPos.y), rMax, colBotRight, 2.5f);

        ImVec2 textSize = font->CalcTextSizeA(22.0f, FLT_MAX, 0.0f, label);
        ImVec2 textPos = ImVec2(rPos.x + (rMax.x - rPos.x - textSize.x) * 0.5f, rPos.y + (rMax.y - rPos.y - textSize.y) * 0.5f);
        if (isSelected) { textPos.x += 1.0f; textPos.y += 1.0f; }

        draw->AddText(font, 22.0f, textPos, IM_COL32(0, 0, 0, 255), label);

        bool clicked = isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        if (clicked) SoundEngine::Get().Play(SoundType::OpenCell);
        return clicked;
    }

    float hoverW = isSelected ? 7.0f : 0.0f;
    float hoverH = isSelected ? 3.5f : 0.0f;
    ImVec2 actPos = ImVec2(pos.x - hoverW, pos.y - hoverH);
    ImVec2 actSize = ImVec2(size.x + hoverW * 2.0f, size.y + hoverH * 2.0f);
    ImVec2 actMax = ImVec2(actPos.x + actSize.x, actPos.y + actSize.y);

    float radius = actSize.y * 0.5f;
    ImU32 btnBg = isSelected ? theme.pillBtnHover : theme.pillBtnNormal;
    ImU32 btnBorder = isSelected ? theme.pillBtnBorder : IM_COL32(255, 255, 255, 75);

    if (isSelected) {
        DrawCrispShadow(draw, actPos, actMax, radius, theme.pillBtnBorder);
    }

    draw->AddRectFilled(actPos, actMax, btnBg, radius);

    ImVec2 glossMax = ImVec2(actMax.x, actPos.y + actSize.y * 0.44f);
    draw->AddRectFilled(actPos, glossMax, IM_COL32(255, 255, 255, 36), radius, ImDrawFlags_RoundCornersTop);

    draw->AddLine(ImVec2(actPos.x + radius * 0.5f, actPos.y + 1.0f),
                  ImVec2(actMax.x - radius * 0.5f, actPos.y + 1.0f),
                  IM_COL32(255, 255, 255, 75), 1.2f);

    if (isSelected) {
        float sheenX = std::fmod(animTime * 350.0f, actSize.x + 120.0f) - 60.0f;
        ImVec2 s1 = ImVec2(actPos.x + sheenX, actPos.y);
        ImVec2 s2 = ImVec2(actPos.x + sheenX + 28.0f, actMax.y);
        draw->AddLine(s1, s2, IM_COL32(255, 255, 255, 60), 10.0f);
    }

    draw->AddRect(actPos, actMax, btnBorder, radius, 0, isSelected ? 2.6f : 1.4f);

    if (isSelected) {
        float bob = std::sin(animTime * 7.5f) * 3.5f;
        ImVec2 triP1 = ImVec2(actPos.x - 26.0f + bob, actPos.y + actSize.y * 0.28f);
        ImVec2 triP2 = ImVec2(actPos.x - 11.0f + bob, actPos.y + actSize.y * 0.50f);
        ImVec2 triP3 = ImVec2(actPos.x - 26.0f + bob, actPos.y + actSize.y * 0.72f);
        draw->AddTriangleFilled(triP1, triP2, triP3, theme.pillBtnBorder);
        draw->AddTriangle(triP1, triP2, triP3, IM_COL32(255, 255, 255, 220), 1.4f);
    }

    ImVec2 textSize = font->CalcTextSizeA(24.0f, FLT_MAX, 0.0f, label);
    ImVec2 textPos = ImVec2(actPos.x + (actSize.x - textSize.x) * 0.5f, actPos.y + (actSize.y - textSize.y) * 0.5f);

    draw->AddText(font, 24.0f, ImVec2(textPos.x + 1.2f, textPos.y + 1.2f), IM_COL32(0, 0, 0, 220), label);
    draw->AddText(font, 24.0f, textPos, IM_COL32(255, 255, 255, 255), label);

    bool clicked = isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    if (clicked) {
        SoundEngine::Get().Play(SoundType::OpenCell);
    }
    return clicked;
}

inline bool DrawMiniPillButton(const char* label, ImVec2 pos, ImVec2 size, const VisualTheme& theme, ImFont* font, float animTime = 0.0f) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    bool isHovered = (mousePos.x >= pos.x && mousePos.x <= pos.x + size.x && mousePos.y >= pos.y && mousePos.y <= pos.y + size.y);
    float hoverPad = isHovered ? 2.0f : 0.0f;

    ImVec2 actPos = ImVec2(pos.x - hoverPad, pos.y - hoverPad * 0.5f);
    ImVec2 actSize = ImVec2(size.x + hoverPad * 2.0f, size.y + hoverPad);
    ImVec2 actMax = ImVec2(actPos.x + actSize.x, actPos.y + actSize.y);
    float radius = actSize.y * 0.5f;

    ImU32 bg = isHovered ? theme.pillBtnHover : theme.pillBtnNormal;
    ImU32 border = isHovered ? theme.pillBtnBorder : IM_COL32(255, 255, 255, 80);

    if (isHovered) {
        draw->AddRectFilled(ImVec2(actPos.x - 3.0f, actPos.y - 1.0f), ImVec2(actMax.x + 3.0f, actMax.y + 4.0f), (theme.pillBtnBorder & 0x00FFFFFF) | 0x44000000, radius + 2.0f);
    }

    draw->AddRectFilled(actPos, actMax, bg, radius);
    draw->AddRectFilled(actPos, ImVec2(actMax.x, actPos.y + actSize.y * 0.44f), IM_COL32(255, 255, 255, 32), radius, ImDrawFlags_RoundCornersTop);
    draw->AddRect(actPos, actMax, border, radius, 0, isHovered ? 2.2f : 1.3f);

    ImVec2 textSize = font->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, label);
    ImVec2 textPos = ImVec2(actPos.x + (actSize.x - textSize.x) * 0.5f, actPos.y + (actSize.y - textSize.y) * 0.5f);

    draw->AddText(font, 16.0f, ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 200), label);
    draw->AddText(font, 16.0f, textPos, IM_COL32(255, 255, 255, 255), label);

    bool clicked = isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    if (clicked) SoundEngine::Get().Play(SoundType::OpenCell);
    return clicked;
}

inline bool DrawCustomRadioButton(const char* label, ImVec2 pos, bool active, const VisualTheme& theme, ImFont* font) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();
    float radius = 9.5f;

    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
    bool isHovered = std::hypot(mousePos.x - center.x, mousePos.y - center.y) <= radius + 5.0f;

    draw->AddCircleFilled(center, radius, active ? theme.pillBtnHover : theme.cellCovered, 16);
    draw->AddCircle(center, radius, active ? theme.pillBtnBorder : IM_COL32(255, 255, 255, 75), 16, 1.8f);
    if (active) {
        draw->AddCircleFilled(center, 4.5f, IM_COL32(255, 255, 255, 255), 12);
        draw->AddCircle(center, 7.0f, theme.pillBtnBorder, 12, 1.2f);
    }

    draw->AddText(font, 17.0f, ImVec2(pos.x + 26.0f, pos.y), IM_COL32(245, 245, 245, 255), label);

    bool clicked = isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    if (clicked) SoundEngine::Get().Play(SoundType::OpenCell);
    return clicked;
}

inline bool DrawCustomToggle(const char* label, ImVec2 pos, bool active, const VisualTheme& theme, ImFont* font) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    float w = 46.0f, h = 24.0f;
    bool isHovered = (mousePos.x >= pos.x && mousePos.x <= pos.x + w && mousePos.y >= pos.y && mousePos.y <= pos.y + h);

    draw->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), active ? theme.pillBtnHover : IM_COL32(28, 34, 46, 255), 12.0f);
    draw->AddRect(pos, ImVec2(pos.x + w, pos.y + h), active ? theme.pillBtnBorder : IM_COL32(110, 125, 145, 200), 12.0f, 0, 1.6f);

    float knobX = active ? (pos.x + w - 12.0f) : (pos.x + 12.0f);
    draw->AddCircleFilled(ImVec2(knobX, pos.y + 12.0f), 9.0f, IM_COL32(255, 255, 255, 255));
    if (active) {
        draw->AddCircle(ImVec2(knobX, pos.y + 12.0f), 10.5f, theme.pillBtnBorder, 0, 1.6f);
    }

    draw->AddText(font, 16.0f, ImVec2(pos.x + w + 12.0f, pos.y + 3.0f), IM_COL32(240, 240, 240, 255), label);

    bool clicked = isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    if (clicked) SoundEngine::Get().Play(SoundType::OpenCell);
    return clicked;
}

inline void DrawCustomSlider(ImVec2 pos, float width, float& val, float minV, float maxV, const VisualTheme& theme) {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();

    float h = 10.0f;
    ImVec2 trackMin = ImVec2(pos.x, pos.y + 6.0f);
    ImVec2 trackMax = ImVec2(pos.x + width, pos.y + 6.0f + h);

    draw->AddRectFilled(trackMin, trackMax, IM_COL32(14, 18, 26, 255), 5.0f);
    draw->AddRect(trackMin, trackMax, IM_COL32(255, 255, 255, 35), 5.0f, 0, 1.0f);

    float norm = (val - minV) / (maxV - minV);
    norm = std::clamp(norm, 0.0f, 1.0f);

    ImVec2 fillMax = ImVec2(pos.x + norm * width, trackMax.y);
    draw->AddRectFilled(trackMin, fillMax, theme.pillBtnHover, 5.0f);
    draw->AddLine(ImVec2(trackMin.x, trackMin.y + 1.0f), ImVec2(fillMax.x, trackMin.y + 1.0f), IM_COL32(255, 255, 255, 80), 1.0f);

    ImVec2 handleCenter = ImVec2(pos.x + norm * width, pos.y + 11.0f);
    bool isHovered = std::hypot(mousePos.x - handleCenter.x, mousePos.y - handleCenter.y) <= 13.0f;

    if (isHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        norm = (mousePos.x - pos.x) / width;
        norm = std::clamp(norm, 0.0f, 1.0f);
        val = minV + norm * (maxV - minV);
    }

    if (isHovered) {
        draw->AddCircleFilled(handleCenter, 14.0f, (theme.pillBtnBorder & 0x00FFFFFF) | 0x44000000);
    }
    draw->AddCircleFilled(handleCenter, 10.0f, IM_COL32(255, 255, 255, 255));
    draw->AddCircle(handleCenter, 10.0f, theme.pillBtnBorder, 0, 2.2f);
}
