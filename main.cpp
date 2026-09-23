#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "GameLogic.h"
#include "SoundEngine.h"
#include "Theme.h"
#include "UIComponents.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

enum class Language { RU, EN };

struct LocStrings {
    const char* appSubtitle;
    const char* newGame;
    const char* settings;
    const char* exitApp;
    const char* selectPreset;
    const char* presetBeginner;
    const char* presetIntermediate;
    const char* presetExpert;
    const char* presetMaster;
    const char* presetLegend;
    const char* presetCustomBtn;
    const char* customModalTitle;
    const char* applyStart;
    const char* backToMenu;
    const char* theme;
    const char* language;
    const char* audioSettings;
    const char* volume;
    const char* soundFx;
    const char* hoverSound;
    const char* aviasalesText;
    const char* recordBadge;
    const char* navLegend;
    const char* winTitle;
    const char* defeatTitle;
    const char* playAgain;
    const char* cellsUnit;
    const char* deciphering;
    const char* fullscreen;
    const char* resetGame;
    const char* minimapLabel;
    const char* densityLabel;
    const char* difficultyLabel;
};

static LocStrings Loc[] = {
    {
        "МИЛЛИОН КЛЕТОК · DX11 + SIMD",
        "НОВАЯ ИГРА",
        "НАСТРОЙКИ",
        "ВЫХОД",
        "ВЫБОР РЕЖИМА ИГРЫ",
        "🌱 НОВИЧОК (9 × 9, 10 МИН)",
        "🌿 ЛЮБИТЕЛЬ (16 × 16, 40 МИН)",
        "🔥 ЭКСПЕРТ (30 × 16, 99 МИН)",
        "⚡ МАСТЕР (100 × 100, 1500 МИН)",
        "👑 ЛЕГЕНДА (1000 × 1000 — 1,000,000 КЛЕТОК)",
        "🛠️ КАСТОМНЫЙ РЕЖИМ",
        "🛠️ КАСТОМНАЯ НАСТРОЙКА ПОЛЯ",
        "⚡ НАЧАТЬ ИГРУ",
        "НАЗАД",
        "Тема оформления",
        "Язык / Language",
        "Звуковые эффекты",
        "Громкость",
        "Включить звуки",
        "Звук при наведении",
        "✈️ Клетка весит меньше, чем билет на Авиасейлс!",
        "РЕКОРД",
        "W / S — выбор · ENTER — ок · ESC — назад · F11 — полноэкранный",
        "🏆 ПОБЕДА! ВСЕ МИНЫ ОБЕЗВРЕЖЕНЫ! 🏆",
        "💥 БУМ! ВЫ ПОДОРВАЛИСЬ НА МИНЕ! 💥",
        "Сыграть снова",
        "клеток",
        "Хороший рендер и оптимизация · миллион клеток (GRaO.mc)",
        "Полноэкранный режим (F11)",
        "Сброс",
        "Радар",
        "Плотность мин",
        "Сложность"
    },
    {
        "1,000,000 CELLS · DX11 + SIMD",
        "NEW GAME",
        "SETTINGS",
        "EXIT",
        "SELECT GAME MODE",
        "🌱 BEGINNER (9 × 9, 10 MINES)",
        "🌿 INTERMEDIATE (16 × 16, 40 MINES)",
        "🔥 EXPERT (30 × 16, 99 MINES)",
        "⚡ MASTER (100 × 100, 1,500 MINES)",
        "👑 LEGEND (1000 × 1000 — 1,000,000 CELLS)",
        "🛠️ CUSTOM MODE",
        "🛠️ CUSTOM BOARD CONFIGURATION",
        "⚡ START GAME",
        "BACK",
        "Visual Theme",
        "Language / Язык",
        "Sound Effects",
        "Master Volume",
        "Enable Sound FX",
        "Enable Hover Sound",
        "✈️ Cells weigh less than a ticket on Aviasales!",
        "RECORD",
        "W / S — select · ENTER — ok · ESC — back · F11 — fullscreen",
        "🏆 VICTORY! ALL MINES CLEARED! 🏆",
        "💥 GAME OVER! YOU HIT A MINE! 💥",
        "Play Again",
        "cells",
        "Great Render and Optimization · million cells (GRaO.mc)",
        "Fullscreen Mode (F11)",
        "Reset",
        "Radar",
        "Mine Density",
        "Difficulty"
    }
};

struct Particle {
    ImVec2 pos;
    ImVec2 vel;
    ImU32 color;
    float size;
    float life;
    float maxLife;
    bool isConfetti;
};

class ParticleEngine {
public:
    std::vector<Particle> particles;

    void SpawnExplosion(ImVec2 center, int count = 120) {
        particles.clear();
        particles.reserve(count);

        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_real_distribution<float> distAngle(0.0f, 6.28318f);
        std::uniform_real_distribution<float> distSpeed(90.0f, 490.0f);
        std::uniform_real_distribution<float> distLife(0.6f, 1.6f);
        std::uniform_real_distribution<float> distSize(4.0f, 15.0f);

        for (int i = 0; i < count; i++) {
            float angle = distAngle(rng);
            float speed = distSpeed(rng);
            
            Particle p;
            p.pos = center;
            p.vel = ImVec2(std::cos(angle) * speed, std::sin(angle) * speed);
            p.size = distSize(rng);
            p.life = distLife(rng);
            p.maxLife = p.life;
            p.isConfetti = false;
            
            if (i % 3 == 0) p.color = IM_COL32(255, 50, 30, 255);
            else if (i % 3 == 1) p.color = IM_COL32(255, 190, 20, 255);
            else p.color = IM_COL32(255, 255, 230, 255);

            particles.push_back(p);
        }
    }

    void SpawnConfetti(ImVec2 center, int count = 140) {
        particles.clear();
        particles.reserve(count);

        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_real_distribution<float> distAngle(-2.4f, -0.74f);
        std::uniform_real_distribution<float> distSpeed(180.0f, 520.0f);
        std::uniform_real_distribution<float> distLife(1.4f, 2.8f);
        std::uniform_real_distribution<float> distSize(5.0f, 12.0f);

        for (int i = 0; i < count; i++) {
            float angle = distAngle(rng);
            float speed = distSpeed(rng);

            Particle p;
            p.pos = ImVec2(center.x + ((i % 11) - 5) * 45.0f, center.y + 100.0f);
            p.vel = ImVec2(std::cos(angle) * speed, std::sin(angle) * speed);
            p.size = distSize(rng);
            p.life = distLife(rng);
            p.maxLife = p.life;
            p.isConfetti = true;

            int cMod = i % 5;
            if (cMod == 0) p.color = IM_COL32(0, 255, 150, 255);
            else if (cMod == 1) p.color = IM_COL32(0, 210, 255, 255);
            else if (cMod == 2) p.color = IM_COL32(255, 220, 0, 255);
            else if (cMod == 3) p.color = IM_COL32(255, 80, 180, 255);
            else p.color = IM_COL32(255, 255, 255, 255);

            particles.push_back(p);
        }
    }

    void UpdateAndRender(ImDrawList* drawList, float dt) {
        for (auto it = particles.begin(); it != particles.end();) {
            it->life -= dt;
            if (it->life <= 0.0f) {
                it = particles.erase(it);
                continue;
            }

            it->pos.x += it->vel.x * dt;
            it->pos.y += it->vel.y * dt;

            if (it->isConfetti) {
                it->vel.y += 240.0f * dt;
                it->vel.x *= 0.985f;
            } else {
                it->vel.x *= 0.94f;
                it->vel.y *= 0.94f;
            }

            float alpha = std::clamp(it->life / it->maxLife, 0.0f, 1.0f);
            ImU32 col = (it->color & 0x00FFFFFF) | ((uint32_t)(alpha * 255.0f) << 24);

            if (it->isConfetti) {
                drawList->AddRectFilled(ImVec2(it->pos.x - it->size * 0.5f, it->pos.y - it->size * 0.3f),
                                        ImVec2(it->pos.x + it->size * 0.5f, it->pos.y + it->size * 0.3f), col, 2.0f);
            } else {
                drawList->AddCircleFilled(it->pos, it->size * alpha, col);
            }
            ++it;
        }
    }
};

enum class AppState {
    MainMenu,
    PresetSelect,
    CustomSetup,
    Settings,
    InGame
};

class RenderApp {
public:
    HWND hwnd = nullptr;
    ID3D11Device* d3dDevice = nullptr;
    ID3D11DeviceContext* d3dContext = nullptr;
    IDXGISwapChain1* swapChain = nullptr;
    ID3D11RenderTargetView* mainRTV = nullptr;

    ImFont* fontTitle = nullptr;
    ImFont* fontSub = nullptr;
    ImFont* fontButton = nullptr;
    ImFont* fontBody = nullptr;

    MinesweeperBackend game;
    ParticleEngine particles;

    AppState state = AppState::MainMenu;
    Language lang = Language::RU;
    int currentThemeIdx = 0;

    int activeMenuIdx = 0;

    float menuSlideProgress = 0.0f;
    float titleHoverTimer = 0.0f;

    bool isFullscreen = false;
    WINDOWPLACEMENT prevPlacement = { sizeof(prevPlacement) };

    ImVec2 cameraOffset = ImVec2(50.0f, 80.0f);
    float cameraZoom = 1.0f;
    
    bool isRmbDown = false;
    ImVec2 rmbPressPos;
    ImVec2 cameraOffsetAtRmbPress;
    bool isRmbDragging = false;

    int lastHoveredCellIdx = -1;
    int culledCellsThisFrame = 0;
    bool showMinimap = true;

    int titleClickCount = 0;
    std::chrono::steady_clock::time_point lastTitleClickTime;
    bool showAviasalesBanner = false;
    float aviasalesBannerTimer = 0.0f;

    int customW = 1000;
    int customH = 1000;
    int customB = 150000;

    float bgAnimTime = 0.0f;
    float shakeTimer = 0.0f;
    float shakeIntensity = 0.0f;
    bool winCelebrated = false;
    bool isCellPressed = false;
    bool justResetGame = false;

    void ResetGame() {
        game.InitField(game.SquareX, game.SquareY, game.NumberOfBomb);
        winCelebrated = false;
        particles.particles.clear();
        shakeTimer = 0.0f;
        shakeIntensity = 0.0f;
        isRmbDown = false;
        isRmbDragging = false;
        justResetGame = true;
        SoundEngine::Get().Play(SoundType::OpenCell);
    }

    void CenterCameraOnField() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 canvasSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - 70.0f);
        float baseStep = 34.0f;
        float fieldW = game.SquareX * baseStep;
        float fieldH = game.SquareY * baseStep;

        if (game.SquareX <= 40 && game.SquareY <= 40) {
            float fitZoom = std::min((canvasSize.x - 80.0f) / fieldW, (canvasSize.y - 80.0f) / fieldH);
            cameraZoom = std::clamp(fitZoom, 0.7f, 2.2f);
        } else {
            cameraZoom = 1.0f;
        }

        float step = (32.0f + 2.0f) * cameraZoom;
        cameraOffset.x = (canvasSize.x - game.SquareX * step) * 0.5f;
        cameraOffset.y = (canvasSize.y - game.SquareY * step) * 0.5f;
        if (game.SquareX >= 100 || game.SquareY >= 100) {
            cameraOffset.x = 40.0f;
            cameraOffset.y = 30.0f;
        }
    }

    void LaunchGame(int w, int h, int b) {
        game.InitField(w, h, b);
        state = AppState::InGame;
        winCelebrated = false;
        particles.particles.clear();
        shakeTimer = 0.0f;
        shakeIntensity = 0.0f;
        isRmbDown = false;
        isRmbDragging = false;
        justResetGame = true;
        CenterCameraOnField();
        SoundEngine::Get().Play(SoundType::OpenCell);
    }

    void ToggleFullscreen() {
        DWORD style = GetWindowLong(hwnd, GWL_STYLE);
        if (!isFullscreen) {
            MONITORINFO mi = { sizeof(mi) };
            if (GetWindowPlacement(hwnd, &prevPlacement) &&
                GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
                SetWindowLong(hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(hwnd, HWND_TOP,
                             mi.rcMonitor.left, mi.rcMonitor.top,
                             mi.rcMonitor.right - mi.rcMonitor.left,
                             mi.rcMonitor.bottom - mi.rcMonitor.top,
                             SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                isFullscreen = true;
            }
        } else {
            SetWindowLong(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
            SetWindowPlacement(hwnd, &prevPlacement);
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            isFullscreen = false;
        }
    }

    bool InitWindowAndD3D(int width, int height) {
        timeBeginPeriod(1);

        WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"GRaO_MC_Class", NULL };
        RegisterClassExW(&wc);

        hwnd = CreateWindowW(wc.lpszClassName, L"GRaO.mc", WS_OVERLAPPEDWINDOW, 100, 100, width, height, NULL, NULL, wc.hInstance, this);
        if (!hwnd) return false;

        BOOL useDarkMode = TRUE;
        DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));

        UINT createDeviceFlags = 0;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &d3dDevice, &featureLevel, &d3dContext);
        if (FAILED(hr)) return false;

        IDXGIDevice* dxgiDevice = nullptr;
        d3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
        IDXGIAdapter* dxgiAdapter = nullptr;
        dxgiDevice->GetAdapter(&dxgiAdapter);
        IDXGIFactory2* dxgiFactory = nullptr;
        dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

        DXGI_SWAP_CHAIN_DESC1 sd1 = {};
        sd1.Width = 0;
        sd1.Height = 0;
        sd1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd1.SampleDesc.Count = 1;
        sd1.SampleDesc.Quality = 0;
        sd1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd1.BufferCount = 2;
        sd1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd1.Scaling = DXGI_SCALING_NONE;
        sd1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

        hr = dxgiFactory->CreateSwapChainForHwnd(d3dDevice, hwnd, &sd1, NULL, NULL, &swapChain);
        if (FAILED(hr)) {
            sd1.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            hr = dxgiFactory->CreateSwapChainForHwnd(d3dDevice, hwnd, &sd1, NULL, NULL, &swapChain);
            if (FAILED(hr)) return false;
        }

        dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        dxgiFactory->Release();
        dxgiAdapter->Release();
        dxgiDevice->Release();

        CreateRenderTarget();

        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        ImGui::StyleColorsDark();

        static const ImWchar emojiRanges[] = {
            0x0020, 0x00FF,
            0x0400, 0x052F,
            0x2000, 0x2BFF,
            0x1F000, 0x1FAFF,
            0
        };

        ImFontConfig fontConfig;
        fontConfig.OversampleH = 1;
        fontConfig.OversampleV = 1;

        const char* fontPathBold   = "C:\\Windows\\Fonts\\segoeuib.ttf";
        const char* fontPathReg    = "C:\\Windows\\Fonts\\segoeui.ttf";
        const char* fontPathEmoji  = "C:\\Windows\\Fonts\\seguiemj.ttf";
        const char* fontPathSymbol = "C:\\Windows\\Fonts\\seguisym.ttf";

        fontTitle  = io.Fonts->AddFontFromFileTTF(fontPathBold, 54.0f, &fontConfig, emojiRanges);
        
        fontSub    = io.Fonts->AddFontFromFileTTF(fontPathBold, 20.0f, &fontConfig, emojiRanges);
        ImFontConfig fontConfigMerge;
        fontConfigMerge.MergeMode = true;
        fontConfigMerge.OversampleH = 1;
        fontConfigMerge.OversampleV = 1;
        io.Fonts->AddFontFromFileTTF(fontPathEmoji, 18.0f, &fontConfigMerge, emojiRanges);
        io.Fonts->AddFontFromFileTTF(fontPathSymbol, 18.0f, &fontConfigMerge, emojiRanges);

        fontButton = io.Fonts->AddFontFromFileTTF(fontPathBold, 24.0f, &fontConfig, emojiRanges);
        io.Fonts->AddFontFromFileTTF(fontPathEmoji, 22.0f, &fontConfigMerge, emojiRanges);
        io.Fonts->AddFontFromFileTTF(fontPathSymbol, 22.0f, &fontConfigMerge, emojiRanges);

        fontBody   = io.Fonts->AddFontFromFileTTF(fontPathReg,  18.0f, &fontConfig, emojiRanges);
        io.Fonts->AddFontFromFileTTF(fontPathEmoji, 16.0f, &fontConfigMerge, emojiRanges);
        io.Fonts->AddFontFromFileTTF(fontPathSymbol, 16.0f, &fontConfigMerge, emojiRanges);

        if (!fontTitle) fontTitle = io.Fonts->AddFontDefault();
        if (!fontSub) fontSub = io.Fonts->AddFontDefault();
        if (!fontButton) fontButton = io.Fonts->AddFontDefault();
        if (!fontBody) fontBody = io.Fonts->AddFontDefault();

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(d3dDevice, d3dContext);

        game.InitField(1000, 1000, 150000);
        CenterCameraOnField();

        return true;
    }

    void CreateRenderTarget() {
        ID3D11Texture2D* pBackBuffer = nullptr;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer) {
            d3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRTV);
            pBackBuffer->Release();
        }
    }

    void CleanupRenderTarget() {
        if (mainRTV) { mainRTV->Release(); mainRTV = nullptr; }
    }

    void Shutdown() {
        timeEndPeriod(1);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupRenderTarget();
        if (swapChain) { swapChain->Release(); swapChain = nullptr; }
        if (d3dContext) { d3dContext->Release(); d3dContext = nullptr; }
        if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
        if (hwnd) { DestroyWindow(hwnd); hwnd = nullptr; }
        UnregisterClassW(L"GRaO_MC_Class", GetModuleHandle(NULL));
    }

    void RunLoop() {
        MSG msg = {};

        while (msg.message != WM_QUIT) {
            if (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
                continue;
            }

            if (IsIconic(hwnd)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
                continue;
            }

            float dt = ImGui::GetIO().DeltaTime;
            if (dt > 0.05f) dt = 0.05f;
            bgAnimTime += dt;
            game.UpdateTimer();

            SoundEngine::Get().pitchMultiplier = Themes[currentThemeIdx].soundPitch;

            menuSlideProgress = std::clamp(menuSlideProgress + dt * 1.85f, 0.0f, 1.0f);

            if (shakeTimer > 0.0f) {
                shakeTimer -= dt;
                if (shakeTimer <= 0.0f) shakeIntensity = 0.0f;
            }

            if (showAviasalesBanner) {
                aviasalesBannerTimer -= dt;
                if (aviasalesBannerTimer <= 0.0f) showAviasalesBanner = false;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            RenderUI(dt);

            ImGui::Render();
            const float clear_color[4] = { 0.02f, 0.04f, 0.03f, 1.00f };
            d3dContext->OMSetRenderTargets(1, &mainRTV, NULL);
            d3dContext->ClearRenderTargetView(mainRTV, clear_color);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            swapChain->Present(1, 0);
        }
    }

private:
    void RenderUI(float dt) {
        if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
            ToggleFullscreen();
        }

        const VisualTheme& theme = Themes[currentThemeIdx];
        const LocStrings& str = Loc[(int)lang];

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 vpPos = viewport->WorkPos;
        if (shakeIntensity > 0.001f) {
            float ox = ((float)(rand() % 200 - 100) / 100.0f) * shakeIntensity;
            float oy = ((float)(rand() % 200 - 100) / 100.0f) * shakeIntensity;
            vpPos.x += ox;
            vpPos.y += oy;
        }

        ImGui::SetNextWindowPos(vpPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, theme.bgWindow);
        ImGui::Begin("GRaO_FullCanvas", nullptr, windowFlags);
        ImGui::Dummy(viewport->WorkSize);

        if (state != AppState::InGame) {
            RenderThemeSpecificBackground(theme);
        } else {
            RenderInGameBackground(theme);
        }

        switch (state) {
        case AppState::MainMenu:
            RenderMainMenuUI(str, theme, dt);
            break;
        case AppState::PresetSelect:
            RenderPresetSelectUI(str, theme, dt);
            break;
        case AppState::CustomSetup:
            RenderCustomSetupUI(str, theme, dt);
            break;
        case AppState::Settings:
            RenderSettingsUI(str, theme, dt);
            break;
        case AppState::InGame:
            RenderInGameUI(str, theme, dt);
            break;
        }

        if (state != AppState::InGame) {
            RenderBottomHUD(str, theme);
        }

        ImGui::End();
        ImGui::PopStyleColor();
    }

    void RenderInGameBackground(const VisualTheme& theme) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 winPos = ImGui::GetWindowPos();

        if (theme.isRetro) {
            draw->AddRectFilled(winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y), IM_COL32(0, 128, 128, 255));
            for (float y = winPos.y; y < winPos.y + winSize.y; y += 4.0f) {
                for (float x = winPos.x; x < winPos.x + winSize.x; x += 4.0f) {
                    if (((int)(x + y) / 4) % 2 == 0) {
                        draw->AddRectFilled(ImVec2(x, y), ImVec2(x + 2, y + 2), IM_COL32(0, 140, 140, 110));
                    }
                }
            }
            return;
        }

        float gridSpacing = 56.0f;
        float shift = std::fmod(bgAnimTime * 14.0f, gridSpacing);
        ImU32 lineCol = (theme.cellCoveredBorder & 0x00FFFFFF) | 0x22000000;

        for (float x = winPos.x + shift; x < winPos.x + winSize.x; x += gridSpacing) {
            draw->AddLine(ImVec2(x, winPos.y), ImVec2(x, winPos.y + winSize.y), lineCol, 1.2f);
        }
        for (float y = winPos.y + shift; y < winPos.y + winSize.y; y += gridSpacing) {
            draw->AddLine(ImVec2(winPos.x, y), ImVec2(winPos.x + winSize.x, y), lineCol, 1.2f);
        }

        ImVec2 center = ImVec2(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.5f);
        draw->AddCircleFilled(center, winSize.y * 0.65f, (theme.accentGlow & 0x00FFFFFF) | 0x16000000);

        if (currentThemeIdx == 0) {
            for (int i = 0; i < 16; i++) {
                float lineX = winPos.x + (i * 110.0f);
                float lineY = winPos.y + std::fmod(bgAnimTime * 110.0f + i * 85.0f, winSize.y);
                draw->AddCircleFilled(ImVec2(lineX, lineY), 3.0f, IM_COL32(0, 255, 140, 140));
                draw->AddLine(ImVec2(lineX, lineY - 12.0f), ImVec2(lineX, lineY), IM_COL32(0, 255, 140, 80), 1.5f);
            }
        } else if (currentThemeIdx == 1) {
            float barW = 8.0f;
            for (int i = 0; i < 24; i++) {
                float h = (std::sin(bgAnimTime * 4.5f + i * 0.38f) + 1.0f) * 0.5f * 70.0f + 12.0f;
                draw->AddRectFilled(ImVec2(winPos.x + 8.0f + i * 14.0f, winPos.y + winSize.y - h - 10.0f),
                                    ImVec2(winPos.x + 8.0f + i * 14.0f + barW, winPos.y + winSize.y - 10.0f),
                                    IM_COL32(0, 220, 255, 75), 2.0f);
                draw->AddRect(ImVec2(winPos.x + 8.0f + i * 14.0f, winPos.y + winSize.y - h - 10.0f),
                              ImVec2(winPos.x + 8.0f + i * 14.0f + barW, winPos.y + winSize.y - 10.0f),
                              IM_COL32(0, 240, 255, 140), 2.0f);
            }
        }
    }

    void RenderThemeSpecificBackground(const VisualTheme& theme) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 winPos = ImGui::GetWindowPos();

        if (currentThemeIdx == 0) {
            int cols = 36;
            float colWidth = winSize.x / cols;
            for (int i = 0; i < cols; i++) {
                float speed = 85.0f + (i % 6) * 35.0f;
                float headY = std::fmod(bgAnimTime * speed + i * 140.0f, winSize.y + 200.0f) - 100.0f;
                float x = winPos.x + i * colWidth + colWidth * 0.5f;

                for (int g = 0; g < 9; g++) {
                    float gy = headY - g * 22.0f;
                    if (gy < winPos.y || gy > winPos.y + winSize.y) continue;
                    float trailAlpha = (1.0f - (float)g / 9.0f) * 0.85f;
                    char c = (char)('0' + ((i * 7 + g * 3 + (int)(bgAnimTime * 8.0f)) % 10));
                    char buf[2] = { c, '\0' };

                    ImU32 col = (g == 0) ? IM_COL32(230, 255, 230, 255) : IM_COL32(0, 255, 120, (uint32_t)(trailAlpha * 230.0f));
                    draw->AddText(fontBody, 16.0f, ImVec2(x, gy), col, buf);
                }
            }

            int segments = 40;
            ImVec2 prevP;
            for (int i = 0; i <= segments; i++) {
                float t = (float)i / segments;
                float x = winPos.x + t * winSize.x;
                float waveY = winPos.y + winSize.y - 130.0f + std::sin(t * 14.0f + bgAnimTime * 2.2f) * 16.0f;
                ImVec2 currP = ImVec2(x, waveY);
                if (i > 0) {
                    draw->AddLine(prevP, currP, theme.pillBtnBorder, 3.5f);
                    draw->AddLine(ImVec2(prevP.x, prevP.y + 4.0f), ImVec2(currP.x, currP.y + 4.0f), (theme.pillBtnBorder & 0x00FFFFFF) | 0x55000000, 2.0f);
                }
                prevP = currP;
            }

            ImVec2 reticleC = ImVec2(winPos.x + winSize.x - 70.0f, winPos.y + 70.0f);
            draw->AddCircle(reticleC, 35.0f, theme.pillBtnBorder, 32, 1.8f);
            draw->AddCircle(reticleC, 18.0f, (theme.pillBtnBorder & 0x00FFFFFF) | 0xAA000000, 24, 1.5f);
            float a1 = bgAnimTime * 1.6f;
            draw->AddLine(ImVec2(reticleC.x + std::cos(a1) * 10.0f, reticleC.y + std::sin(a1) * 10.0f),
                          ImVec2(reticleC.x + std::cos(a1) * 44.0f, reticleC.y + std::sin(a1) * 44.0f),
                          theme.pillBtnBorder, 2.2f);

            draw->AddText(fontBody, 13.0f, ImVec2(winPos.x + 24.0f, winPos.y + 24.0f), IM_COL32(0, 255, 140, 180), "SYS_KERNEL: ACTIVE | SIMD_AVX2");
            draw->AddText(fontBody, 13.0f, ImVec2(winPos.x + 24.0f, winPos.y + 42.0f), IM_COL32(0, 255, 140, 140), "MEM_BUFFER: 1,000,000 CELLS [1-BYTE/CELL]");
        }
        else if (currentThemeIdx == 1) {
            float horizonY = winPos.y + winSize.y * 0.72f;

            float sunR = 120.0f;
            ImVec2 sunCenter = ImVec2(winPos.x + winSize.x * 0.5f, horizonY);
            draw->AddCircleFilled(sunCenter, sunR, IM_COL32(255, 40, 130, 210), 64);
            draw->AddCircleFilled(sunCenter, sunR * 0.75f, IM_COL32(255, 200, 40, 240), 64);

            for (int s = 0; s < 8; s++) {
                float cutY = horizonY - sunR * 0.75f + s * 14.0f;
                float cutH = 2.0f + s * 0.9f;
                draw->AddRectFilled(ImVec2(sunCenter.x - sunR, cutY), ImVec2(sunCenter.x + sunR, cutY + cutH), theme.bgWindow);
            }

            int numGridLines = 20;
            float gridSpeed = std::fmod(bgAnimTime * 48.0f, 35.0f);
            for (float gy = horizonY + gridSpeed; gy < winPos.y + winSize.y; gy += 30.0f) {
                float alpha = (gy - horizonY) / (winSize.y * 0.28f);
                ImU32 col = IM_COL32(0, 220, 255, (uint32_t)(alpha * 190.0f));
                draw->AddLine(ImVec2(winPos.x, gy), ImVec2(winPos.x + winSize.x, gy), col, 1.8f);
            }

            for (int i = 0; i <= numGridLines; i++) {
                float t = (float)i / numGridLines;
                float botX = winPos.x + t * winSize.x;
                float topX = winPos.x + winSize.x * 0.5f + (t - 0.5f) * winSize.x * 0.25f;
                draw->AddLine(ImVec2(topX, horizonY), ImVec2(botX, winPos.y + winSize.y), IM_COL32(255, 0, 140, 140), 1.8f);
            }

            int numRays = 7;
            for (int i = 0; i < numRays; i++) {
                float rayX = winPos.x + winSize.x * (0.10f + i * 0.13f);
                float pulse = (std::sin(bgAnimTime * 2.2f + i * 0.8f) + 1.0f) * 0.5f;
                ImVec2 rMin = ImVec2(rayX - 35.0f, winPos.y);
                ImVec2 rMax = ImVec2(rayX + 35.0f, horizonY);
                ImU32 rayColorTop = (theme.accentGlow & 0x00FFFFFF) | ((uint32_t)(45 + pulse * 45) << 24);
                draw->AddRectFilledMultiColor(rMin, rMax, rayColorTop, rayColorTop, 0x00000000, 0x00000000);
            }
        }
        else if (currentThemeIdx == 2) {
            draw->AddRectFilled(winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y), IM_COL32(0, 128, 128, 255));
            for (float y = winPos.y; y < winPos.y + winSize.y; y += 45.0f) {
                draw->AddLine(ImVec2(winPos.x, y), ImVec2(winPos.x + winSize.x, y), IM_COL32(0, 145, 145, 95), 1.2f);
            }
            for (float x = winPos.x; x < winPos.x + winSize.x; x += 45.0f) {
                draw->AddLine(ImVec2(x, winPos.y), ImVec2(x, winPos.y + winSize.y), IM_COL32(0, 145, 145, 95), 1.2f);
            }
        }
        else if (currentThemeIdx == 3) {
            ImVec2 topC = ImVec2(winPos.x + winSize.x * 0.5f, winPos.y + 40.0f);
            draw->AddCircleFilled(topC, 280.0f, IM_COL32(255, 90, 160, 55), 64);
            draw->AddCircleFilled(topC, 180.0f, IM_COL32(255, 180, 50, 55), 64);

            for (int i = 0; i < 35; i++) {
                float px = std::fmod(i * 125.0f, winSize.x);
                float py = std::fmod(i * 73.0f, winSize.y * 0.75f);
                float starAlpha = (std::sin(bgAnimTime * 2.8f + i) + 1.0f) * 0.5f;

                ImVec2 sC = ImVec2(winPos.x + px, winPos.y + py);
                ImU32 sCol = IM_COL32(255, 210, 240, (uint32_t)(starAlpha * 255));

                draw->AddLine(ImVec2(sC.x - 4.0f, sC.y), ImVec2(sC.x + 4.0f, sC.y), sCol, 1.4f);
                draw->AddLine(ImVec2(sC.x, sC.y - 4.0f), ImVec2(sC.x, sC.y + 4.0f), sCol, 1.4f);
            }
        }
        else if (currentThemeIdx == 4) {
            ImVec2 nC1 = ImVec2(winPos.x + winSize.x * 0.35f, winPos.y + winSize.y * 0.35f);
            ImVec2 nC2 = ImVec2(winPos.x + winSize.x * 0.70f, winPos.y + winSize.y * 0.65f);
            draw->AddCircleFilled(nC1, 260.0f, IM_COL32(30, 80, 180, 45), 64);
            draw->AddCircleFilled(nC2, 220.0f, IM_COL32(90, 40, 160, 40), 64);

            for (int i = 0; i < 45; i++) {
                float px = std::fmod(i * 115.0f, winSize.x);
                float py = std::fmod(i * 81.0f, winSize.y);
                float starAlpha = (std::sin(bgAnimTime * 2.2f + i * 1.3f) + 1.0f) * 0.5f;
                draw->AddCircleFilled(ImVec2(winPos.x + px, winPos.y + py), (i % 2 == 0) ? 2.5f : 1.5f, IM_COL32(140, 205, 255, (uint32_t)(starAlpha * 255)));
            }

            float cometT = std::fmod(bgAnimTime * 0.35f, 4.0f);
            if (cometT < 1.2f) {
                float cNorm = cometT / 1.2f;
                float startX = winPos.x + winSize.x * 0.75f;
                float startY = winPos.y + 40.0f;
                ImVec2 head = ImVec2(startX - cNorm * winSize.x * 0.5f, startY + cNorm * winSize.y * 0.45f);
                ImVec2 tail = ImVec2(head.x + 60.0f, head.y - 54.0f);
                draw->AddLine(head, tail, IM_COL32(180, 225, 255, (uint32_t)((1.0f - cNorm) * 230)), 2.8f);
                draw->AddCircleFilled(head, 4.0f, IM_COL32(255, 255, 255, 255));
            }
        }
    }

    void RenderMainMenuUI(const LocStrings& str, const VisualTheme& theme, float dt) {
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 winPos = ImGui::GetWindowPos();

        float ease = SpringOvershoot(menuSlideProgress);
        float offsetY = (1.0f - ease) * (winSize.y * 0.45f);

        ImGui::PushFont(fontTitle);
        const char* mainTitle = (lang == Language::RU) ? "САПЕР" : "MINESWEEPER";
        ImVec2 titleSize = fontTitle->CalcTextSizeA(54.0f, FLT_MAX, 0.0f, mainTitle);
        ImVec2 titlePos = ImVec2(winPos.x + (winSize.x - titleSize.x) * 0.5f, winPos.y + winSize.y * 0.16f + offsetY);

        ImVec2 mousePos = ImGui::GetMousePos();
        bool isTitleHovered = (mousePos.x >= titlePos.x - 30.0f && mousePos.x <= titlePos.x + titleSize.x + 30.0f &&
                               mousePos.y >= titlePos.y - 15.0f && mousePos.y <= titlePos.y + titleSize.y + 15.0f);

        if (isTitleHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(now - lastTitleClickTime).count();
            lastTitleClickTime = now;
            if (elapsed < 1.5f) {
                titleClickCount++;
            } else {
                titleClickCount = 1;
            }
            SoundEngine::Get().Play(SoundType::OpenCell);
            if (titleClickCount >= 5) {
                showAviasalesBanner = true;
                aviasalesBannerTimer = 6.0f;
                titleClickCount = 0;
                SoundEngine::Get().Play(SoundType::Win);
            }
        }

        if (isTitleHovered) {
            titleHoverTimer = std::min(titleHoverTimer + dt, 2.0f);
        } else {
            titleHoverTimer = std::max(titleHoverTimer - dt, 0.0f);
        }

        float hoverFactor = titleHoverTimer / 2.0f;
        float easeHover = EaseOutCubic(hoverFactor);

        uint32_t darkR = (uint32_t)(255 - easeHover * 140.0f);
        uint32_t darkG = (uint32_t)(255 - easeHover * 120.0f);
        uint32_t darkB = (uint32_t)(255 - easeHover * 80.0f);
        ImU32 titleCol = IM_COL32(darkR, darkG, darkB, 255);

        if (showAviasalesBanner) {
            float bannerAlpha = std::clamp(aviasalesBannerTimer / 0.5f, 0.0f, 1.0f);
            ImGui::PushFont(fontBody);
            ImVec2 msgSize = fontBody->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, str.aviasalesText);
            float bW = msgSize.x + 48.0f;
            float bH = 42.0f;
            float bY = titlePos.y - 56.0f;
            ImVec2 bPos = ImVec2(winPos.x + (winSize.x - bW) * 0.5f, bY);
            ImDrawList* draw = ImGui::GetWindowDrawList();

            draw->AddRectFilled(ImVec2(bPos.x - 3, bPos.y - 2), ImVec2(bPos.x + bW + 3, bPos.y + bH + 4), IM_COL32(0, 180, 255, (uint32_t)(bannerAlpha * 120.0f)), 21.0f);
            draw->AddRectFilled(bPos, ImVec2(bPos.x + bW, bPos.y + bH), IM_COL32(0, 115, 235, (uint32_t)(bannerAlpha * 245.0f)), 21.0f);
            draw->AddRect(bPos, ImVec2(bPos.x + bW, bPos.y + bH), IM_COL32(200, 240, 255, (uint32_t)(bannerAlpha * 255.0f)), 21.0f, 0, 2.0f);

            draw->AddText(fontBody, 18.0f, ImVec2(bPos.x + 24.0f, bPos.y + 11.0f), IM_COL32(255, 255, 255, (uint32_t)(bannerAlpha * 255.0f)), str.aviasalesText);
            ImGui::PopFont();
        }

        ImGui::GetWindowDrawList()->AddText(fontTitle, 54.0f, ImVec2(titlePos.x + 3, titlePos.y + 3), IM_COL32(0, 0, 0, 200), mainTitle);
        ImGui::GetWindowDrawList()->AddText(fontTitle, 54.0f, titlePos, titleCol, mainTitle);
        ImGui::PopFont();

        ImGui::PushFont(fontSub);
        const char* acronymText = "GRaO.mc";
        ImVec2 acrSize = fontSub->CalcTextSizeA(20.0f, FLT_MAX, 0.0f, acronymText);
        ImVec2 acrPos = ImVec2(winPos.x + (winSize.x - acrSize.x) * 0.5f, titlePos.y + 60.0f);
        ImGui::GetWindowDrawList()->AddText(fontSub, 20.0f, acrPos, theme.pillBtnBorder, acronymText);
        ImGui::PopFont();

        if (hoverFactor > 0.001f) {
            ImGui::PushFont(fontBody);
            ImVec2 decSize = fontBody->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, str.deciphering);
            ImVec2 decPos = ImVec2(winPos.x + (winSize.x - decSize.x) * 0.5f, acrPos.y + 28.0f);
            
            uint32_t alphaByte = (uint32_t)(easeHover * 240.0f);
            uint32_t decR = (uint32_t)(255 - easeHover * 70.0f);
            uint32_t decG = (uint32_t)(220 - easeHover * 60.0f);
            uint32_t decB = (uint32_t)(100 - easeHover * 30.0f);
            ImU32 decCol = IM_COL32(decR, decG, decB, alphaByte);

            ImGui::GetWindowDrawList()->AddText(fontBody, 18.0f, decPos, decCol, str.deciphering);
            ImGui::PopFont();
        }

        const int numBtns = 3;
        const char* btnLabels[numBtns] = { str.newGame, str.settings, str.exitApp };

        HandleKeyboardNav(numBtns);

        float btnW = 420.0f;
        float btnH = 58.0f;
        float startY = winPos.y + winSize.y * 0.46f;
        float spacingY = 74.0f;

        for (int i = 0; i < numBtns; i++) {
            float btnProgress = std::clamp((menuSlideProgress - i * 0.07f) / 0.85f, 0.0f, 1.0f);
            float btnEase = SpringOvershoot(btnProgress);
            float btnOffsetY = (1.0f - btnEase) * (winSize.y * 0.45f);

            ImVec2 bPos = ImVec2(winPos.x + (winSize.x - btnW) * 0.5f, startY + i * spacingY + btnOffsetY);
            bool isSelected = (activeMenuIdx == i);

            if (DrawPillButton(btnLabels[i], bPos, ImVec2(btnW, btnH), theme, isSelected, fontButton, bgAnimTime, theme.isRetro)) {
                TriggerMenuAction(i);
            }
        }
    }

    void RenderPresetSelectUI(const LocStrings& str, const VisualTheme& theme, float dt) {
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 winPos = ImGui::GetWindowPos();

        float ease = SpringOvershoot(menuSlideProgress);
        float offsetY = (1.0f - ease) * (winSize.y * 0.45f);

        ImGui::PushFont(fontTitle);
        ImVec2 titleSize = fontTitle->CalcTextSizeA(42.0f, FLT_MAX, 0.0f, str.selectPreset);
        ImVec2 titlePos = ImVec2(winPos.x + (winSize.x - titleSize.x) * 0.5f, winPos.y + 55.0f + offsetY);
        ImGui::GetWindowDrawList()->AddText(fontTitle, 42.0f, titlePos, IM_COL32(255, 255, 255, 255), str.selectPreset);
        ImGui::PopFont();

        const int numBtns = 7;
        const char* btnLabels[numBtns] = {
            str.presetBeginner,
            str.presetIntermediate,
            str.presetExpert,
            str.presetMaster,
            str.presetLegend,
            str.presetCustomBtn,
            str.backToMenu
        };

        HandleKeyboardNav(numBtns);

        float btnW = 540.0f;
        float btnH = 52.0f;
        float startY = winPos.y + 130.0f;
        float spacingY = 62.0f;

        for (int i = 0; i < numBtns; i++) {
            float btnProgress = std::clamp((menuSlideProgress - i * 0.05f) / 0.85f, 0.0f, 1.0f);
            float btnEase = SpringOvershoot(btnProgress);
            float btnOffsetY = (1.0f - btnEase) * (winSize.y * 0.45f);

            ImVec2 bPos = ImVec2(winPos.x + (winSize.x - btnW) * 0.5f, startY + i * spacingY + btnOffsetY);
            bool isSelected = (activeMenuIdx == i);

            if (DrawPillButton(btnLabels[i], bPos, ImVec2(btnW, btnH), theme, isSelected, fontButton, bgAnimTime, theme.isRetro)) {
                TriggerPresetAction(i);
            }
        }
    }

    void RenderCustomSetupUI(const LocStrings& str, const VisualTheme& theme, float dt) {
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 winPos = ImGui::GetWindowPos();

        float ease = SpringOvershoot(menuSlideProgress);
        float offsetY = (1.0f - ease) * (winSize.y * 0.45f);

        ImGui::PushFont(fontTitle);
        ImVec2 titleSize = fontTitle->CalcTextSizeA(38.0f, FLT_MAX, 0.0f, str.customModalTitle);
        ImVec2 titlePos = ImVec2(winPos.x + (winSize.x - titleSize.x) * 0.5f, winPos.y + 35.0f + offsetY);
        ImGui::GetWindowDrawList()->AddText(fontTitle, 38.0f, titlePos, IM_COL32(255, 255, 255, 255), str.customModalTitle);
        ImGui::PopFont();

        float panelW = 660.0f;
        float panelH = 540.0f;
        ImVec2 panelPos = ImVec2(winPos.x + (winSize.x - panelW) * 0.5f, winPos.y + 90.0f + offsetY);

        DrawCyberPanel(panelPos, ImVec2(panelPos.x + panelW, panelPos.y + panelH), theme.cellCovered, theme.pillBtnBorder, 14.0f, theme.isRetro);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImGui::PushFont(fontBody);
        float curY = panelPos.y + 20.0f;
        float curX = panelPos.x + 30.0f;

        struct SizePreset { const char* label; int w; int h; };
        SizePreset presets[6] = {
            { "9×9", 9, 9 },
            { "16×16", 16, 16 },
            { "30×16", 30, 16 },
            { "50×50", 50, 50 },
            { "100×100", 100, 100 },
            { "1000×1000", 1000, 1000 }
        };

        for (int i = 0; i < 6; i++) {
            ImVec2 prePos = ImVec2(curX + i * 100.0f, curY);
            if (DrawMiniPillButton(presets[i].label, prePos, ImVec2(92.0f, 28.0f), theme, fontBody, bgAnimTime)) {
                customW = presets[i].w;
                customH = presets[i].h;
                int maxB = (customW * customH) - 9;
                customB = std::clamp((int)((customW * customH) * 0.15f), 1, maxB);
            }
        }
        curY += 40.0f;

        char bufW[64];
        snprintf(bufW, sizeof(bufW), "Width X: %d", customW);
        draw->AddText(fontBody, 17.0f, ImVec2(curX, curY), IM_COL32(240, 240, 240, 255), bufW);

        if (DrawMiniPillButton("-10", ImVec2(curX + panelW - 180.0f, curY - 2.0f), ImVec2(50.0f, 26.0f), theme, fontBody, bgAnimTime)) {
            customW = std::clamp(customW - 10, 5, 1000);
        }
        if (DrawMiniPillButton("+10", ImVec2(curX + panelW - 120.0f, curY - 2.0f), ImVec2(50.0f, 26.0f), theme, fontBody, bgAnimTime)) {
            customW = std::clamp(customW + 10, 5, 1000);
        }
        curY += 26.0f;
        float wVal = (float)customW;
        DrawCustomSlider(ImVec2(curX, curY), panelW - 60.0f, wVal, 5.0f, 1000.0f, theme);
        customW = (int)wVal;
        curY += 38.0f;

        char bufH[64];
        snprintf(bufH, sizeof(bufH), "Height Y: %d", customH);
        draw->AddText(fontBody, 17.0f, ImVec2(curX, curY), IM_COL32(240, 240, 240, 255), bufH);

        if (DrawMiniPillButton("-10", ImVec2(curX + panelW - 180.0f, curY - 2.0f), ImVec2(50.0f, 26.0f), theme, fontBody, bgAnimTime)) {
            customH = std::clamp(customH - 10, 5, 1000);
        }
        if (DrawMiniPillButton("+10", ImVec2(curX + panelW - 120.0f, curY - 2.0f), ImVec2(50.0f, 26.0f), theme, fontBody, bgAnimTime)) {
            customH = std::clamp(customH + 10, 5, 1000);
        }
        curY += 26.0f;
        float hVal = (float)customH;
        DrawCustomSlider(ImVec2(curX, curY), panelW - 60.0f, hVal, 5.0f, 1000.0f, theme);
        customH = (int)hVal;
        curY += 38.0f;

        int totalCells = customW * customH;
        int maxB = totalCells - 9;
        if (maxB < 1) maxB = 1;
        if (customB > maxB) customB = maxB;
        if (customB < 1) customB = 1;

        float densityPct = ((float)customB / (float)totalCells) * 100.0f;

        char bufB[128];
        snprintf(bufB, sizeof(bufB), "💣 %s: %d  (%.1f%%)", str.densityLabel, customB, densityPct);
        draw->AddText(fontBody, 17.0f, ImVec2(curX, curY), IM_COL32(255, 215, 0, 255), bufB);
        curY += 28.0f;

        float dPills[5] = { 10.0f, 15.0f, 20.0f, 25.0f, 30.0f };
        for (int i = 0; i < 5; i++) {
            char pStr[16];
            snprintf(pStr, sizeof(pStr), "%d%%", (int)dPills[i]);
            ImVec2 dpPos = ImVec2(curX + i * 72.0f, curY);
            if (DrawMiniPillButton(pStr, dpPos, ImVec2(64.0f, 26.0f), theme, fontBody, bgAnimTime)) {
                customB = std::clamp((int)(totalCells * (dPills[i] / 100.0f)), 1, maxB);
            }
        }

        int stepDelta = (totalCells >= 100000) ? 1000 : ((totalCells >= 10000) ? 100 : 10);
        char sMinus[16], sPlus[16];
        snprintf(sMinus, sizeof(sMinus), "-%d", stepDelta);
        snprintf(sPlus, sizeof(sPlus), "+%d", stepDelta);

        if (DrawMiniPillButton(sMinus, ImVec2(curX + panelW - 186.0f, curY), ImVec2(58.0f, 26.0f), theme, fontBody, bgAnimTime)) {
            customB = std::clamp(customB - stepDelta, 1, maxB);
        }
        if (DrawMiniPillButton(sPlus, ImVec2(curX + panelW - 120.0f, curY), ImVec2(58.0f, 26.0f), theme, fontBody, bgAnimTime)) {
            customB = std::clamp(customB + stepDelta, 1, maxB);
        }
        curY += 34.0f;

        float densitySliderVal = densityPct;
        DrawCustomSlider(ImVec2(curX, curY), panelW - 60.0f, densitySliderVal, 1.0f, 35.0f, theme);
        if (std::abs(densitySliderVal - densityPct) > 0.1f) {
            customB = std::clamp((int)(totalCells * (densitySliderVal / 100.0f)), 1, maxB);
        }
        curY += 34.0f;

        const char* diffStr = "🟢 Casual";
        ImU32 diffCol = IM_COL32(0, 255, 120, 255);
        if (densityPct > 24.0f) {
            diffStr = "🔴 Nightmare";
            diffCol = IM_COL32(255, 50, 50, 255);
        } else if (densityPct > 18.0f) {
            diffStr = "🟠 Hardcore";
            diffCol = IM_COL32(255, 140, 0, 255);
        } else if (densityPct > 12.0f) {
            diffStr = "🟡 Classic";
            diffCol = IM_COL32(255, 215, 0, 255);
        }

        char infoBuf[128];
        snprintf(infoBuf, sizeof(infoBuf), "Total: %d cells  |  %s: %s", totalCells, str.difficultyLabel, diffStr);
        draw->AddText(fontBody, 16.0f, ImVec2(curX, curY), diffCol, infoBuf);
        curY += 40.0f;

        if (DrawPillButton(str.applyStart, ImVec2(curX, curY), ImVec2(panelW - 240.0f, 46.0f), theme, true, fontButton, bgAnimTime, theme.isRetro)) {
            LaunchGame(customW, customH, customB);
        }

        if (DrawPillButton(str.backToMenu, ImVec2(curX + panelW - 220.0f, curY), ImVec2(160.0f, 46.0f), theme, false, fontButton, bgAnimTime, theme.isRetro)) {
            state = AppState::PresetSelect;
            activeMenuIdx = 5;
            menuSlideProgress = 0.0f;
        }

        ImGui::PopFont();
    }

    void RenderSettingsUI(const LocStrings& str, const VisualTheme& theme, float dt) {
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 winPos = ImGui::GetWindowPos();

        float ease = SpringOvershoot(menuSlideProgress);
        float offsetY = (1.0f - ease) * (winSize.y * 0.45f);

        ImGui::PushFont(fontTitle);
        ImVec2 titleSize = fontTitle->CalcTextSizeA(42.0f, FLT_MAX, 0.0f, str.settings);
        ImVec2 titlePos = ImVec2(winPos.x + (winSize.x - titleSize.x) * 0.5f, winPos.y + 35.0f + offsetY);
        ImGui::GetWindowDrawList()->AddText(fontTitle, 42.0f, titlePos, IM_COL32(255, 255, 255, 255), str.settings);
        ImGui::PopFont();

        float panelW = 590.0f;
        float panelH = 530.0f;
        ImVec2 panelPos = ImVec2(winPos.x + (winSize.x - panelW) * 0.5f, winPos.y + 95.0f + offsetY);

        DrawCyberPanel(panelPos, ImVec2(panelPos.x + panelW, panelPos.y + panelH), theme.cellCovered, theme.pillBtnBorder, 14.0f, theme.isRetro);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        float curY = panelPos.y + 20.0f;
        float curX = panelPos.x + 30.0f;

        ImGui::PushFont(fontBody);

        draw->AddText(fontBody, 20.0f, ImVec2(curX, curY), IM_COL32(0, 220, 255, 255), str.theme);
        curY += 30.0f;

        for (int i = 0; i < 5; i++) {
            ImVec2 rPos = ImVec2(curX + (i % 3) * 180.0f, curY + (i / 3) * 38.0f);
            if (DrawCustomRadioButton(Themes[i].name, rPos, currentThemeIdx == i, theme, fontBody)) {
                currentThemeIdx = i;
            }
        }
        curY += 85.0f;

        draw->AddText(fontBody, 20.0f, ImVec2(curX, curY), IM_COL32(0, 220, 255, 255), str.language);
        curY += 30.0f;

        if (DrawCustomRadioButton("Русский 🇷🇺", ImVec2(curX, curY), lang == Language::RU, theme, fontBody)) lang = Language::RU;
        if (DrawCustomRadioButton("English 🇬🇧", ImVec2(curX + 180.0f, curY), lang == Language::EN, theme, fontBody)) lang = Language::EN;
        curY += 45.0f;

        if (DrawCustomToggle(str.fullscreen, ImVec2(curX, curY), isFullscreen, theme, fontBody)) {
            ToggleFullscreen();
        }
        curY += 42.0f;

        draw->AddText(fontBody, 20.0f, ImVec2(curX, curY), IM_COL32(0, 220, 255, 255), str.audioSettings);
        curY += 30.0f;

        if (DrawCustomToggle(str.soundFx, ImVec2(curX, curY), SoundEngine::Get().soundEnabled, theme, fontBody)) {
            SoundEngine::Get().soundEnabled = !SoundEngine::Get().soundEnabled;
        }
        if (DrawCustomToggle(str.hoverSound, ImVec2(curX + 240.0f, curY), SoundEngine::Get().hoverSoundEnabled, theme, fontBody)) {
            SoundEngine::Get().hoverSoundEnabled = !SoundEngine::Get().hoverSoundEnabled;
        }
        curY += 40.0f;

        draw->AddText(fontBody, 18.0f, ImVec2(curX, curY), IM_COL32(230, 230, 230, 255), str.volume);
        curY += 26.0f;
        DrawCustomSlider(ImVec2(curX, curY), panelW - 60.0f, SoundEngine::Get().masterVolume, 0.0f, 1.0f, theme);
        curY += 40.0f;

        if (DrawPillButton(str.backToMenu, ImVec2(curX, curY), ImVec2(panelW - 60.0f, 44.0f), theme, activeMenuIdx == 0, fontButton, bgAnimTime, theme.isRetro)) {
            state = AppState::MainMenu;
            activeMenuIdx = 0;
            menuSlideProgress = 0.0f;
        }

        ImGui::PopFont();
    }

    void HandleKeyboardNav(int maxItems) {
        if (ImGui::IsKeyPressed(ImGuiKey_W) || ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            activeMenuIdx = (activeMenuIdx - 1 + maxItems) % maxItems;
            SoundEngine::Get().Play(SoundType::Hover);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_S) || ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            activeMenuIdx = (activeMenuIdx + 1) % maxItems;
            SoundEngine::Get().Play(SoundType::Hover);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            if (state == AppState::CustomSetup) {
                state = AppState::PresetSelect;
                activeMenuIdx = 5;
                menuSlideProgress = 0.0f;
            } else if (state == AppState::PresetSelect || state == AppState::Settings) {
                state = AppState::MainMenu;
                activeMenuIdx = 0;
                menuSlideProgress = 0.0f;
            }
        }
    }

    void TriggerMenuAction(int index) {
        if (index == 0) {
            state = AppState::PresetSelect;
            activeMenuIdx = 0;
            menuSlideProgress = 0.0f;
        } else if (index == 1) {
            state = AppState::Settings;
            activeMenuIdx = 0;
            menuSlideProgress = 0.0f;
        } else if (index == 2) {
            PostQuitMessage(0);
        }
    }

    void TriggerPresetAction(int index) {
        switch (index) {
        case 0: LaunchGame(9, 9, 10); break;
        case 1: LaunchGame(16, 16, 40); break;
        case 2: LaunchGame(30, 16, 99); break;
        case 3: LaunchGame(100, 100, 1500); break;
        case 4: LaunchGame(1000, 1000, 150000); break;
        case 5: state = AppState::CustomSetup; menuSlideProgress = 0.0f; break;
        case 6: state = AppState::MainMenu; activeMenuIdx = 0; menuSlideProgress = 0.0f; break;
        }
    }

    void RenderBottomHUD(const LocStrings& str, const VisualTheme& theme) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 winPos = ImGui::GetWindowPos();

        float hudY = winPos.y + winSize.y - 50.0f;

        ImVec2 badgePos = ImVec2(winPos.x + 30.0f, hudY);
        ImVec2 badgeSize = ImVec2(140.0f, 36.0f);
        draw->AddRectFilled(badgePos, ImVec2(badgePos.x + badgeSize.x, badgePos.y + badgeSize.y), IM_COL32(10, 30, 20, 220), 18.0f);
        draw->AddRect(badgePos, ImVec2(badgePos.x + badgeSize.x, badgePos.y + badgeSize.y), theme.pillBtnBorder, 18.0f);
        
        char recBuf[32];
        snprintf(recBuf, sizeof(recBuf), "🏆 %s 0", str.recordBadge);
        ImGui::PushFont(fontBody);
        draw->AddText(fontBody, 15.0f, ImVec2(badgePos.x + 14.0f, badgePos.y + 8.0f), IM_COL32(255, 255, 255, 240), recBuf);

        ImVec2 legSize = fontBody->CalcTextSizeA(15.0f, FLT_MAX, 0.0f, str.navLegend);
        draw->AddText(fontBody, 15.0f, ImVec2(winPos.x + (winSize.x - legSize.x) * 0.5f, hudY + 8.0f), IM_COL32(200, 200, 200, 200), str.navLegend);

        draw->AddText(fontBody, 15.0f, ImVec2(winPos.x + winSize.x - 70.0f, hudY + 8.0f), IM_COL32(150, 150, 150, 180), "v2.0");
        ImGui::PopFont();
    }

    void RenderInGameUI(const LocStrings& str, const VisualTheme& theme, float dt) {
        RenderTopHUD(str, theme);
        RenderMinesweeperGrid(str, theme, dt);
        if (showMinimap) RenderMinimap(theme);
        RenderStatusOverlay(str, theme, dt);
    }

    void RenderTopHUD(const LocStrings& str, const VisualTheme& theme) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();

        float barH = 54.0f;
        ImVec2 barMin = ImVec2(winPos.x + 10.0f, winPos.y + 10.0f);
        ImVec2 barMax = ImVec2(winPos.x + winSize.x - 10.0f, winPos.y + 10.0f + barH);

        DrawCyberPanel(barMin, barMax, theme.cellCovered, theme.pillBtnBorder, 12.0f, theme.isRetro);

        float curX = barMin.x + 14.0f;
        float curY = barMin.y + 10.0f;

        if (DrawMiniPillButton(str.backToMenu, ImVec2(curX, curY), ImVec2(100.0f, 34.0f), theme, fontBody, bgAnimTime)) {
            state = AppState::MainMenu;
            activeMenuIdx = 0;
            menuSlideProgress = 0.0f;
        }
        curX += 112.0f;

        char resetLabel[32];
        snprintf(resetLabel, sizeof(resetLabel), "🔄 %s", str.resetGame);
        if (DrawMiniPillButton(resetLabel, ImVec2(curX, curY), ImVec2(100.0f, 34.0f), theme, fontBody, bgAnimTime)) {
            ResetGame();
        }
        curX += 112.0f;

        const char* fsText = isFullscreen ? "🗗 Window" : "🗖 Fullscreen";
        if (DrawMiniPillButton(fsText, ImVec2(curX, curY), ImVec2(115.0f, 34.0f), theme, fontBody, bgAnimTime)) {
            ToggleFullscreen();
        }
        curX += 127.0f;

        char radarLabel[32];
        snprintf(radarLabel, sizeof(radarLabel), "🗺️ %s", str.minimapLabel);
        if (DrawMiniPillButton(radarLabel, ImVec2(curX, curY), ImVec2(100.0f, 34.0f), theme, fontBody, bgAnimTime)) {
            showMinimap = !showMinimap;
        }

        ImVec2 mousePos = ImGui::GetMousePos();
        float smW = 42.0f;
        float smH = 36.0f;
        float smX = winPos.x + (winSize.x - smW) * 0.5f;
        float smY = barMin.y + 9.0f;
        ImVec2 smMin = ImVec2(smX, smY);
        ImVec2 smMax = ImVec2(smX + smW, smY + smH);
        bool isSmHovered = (mousePos.x >= smMin.x && mousePos.x <= smMax.x && mousePos.y >= smMin.y && mousePos.y <= smMax.y);
        bool isSmClicked = isSmHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        const char* faceStr = "🙂";
        if (game.status == GameStatus::Defeat) {
            faceStr = "😵";
        } else if (game.status == GameStatus::Win) {
            faceStr = "😎";
        } else if (isCellPressed) {
            faceStr = "😮";
        }

        if (theme.isRetro) {
            bool isSmDown = isSmHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
            draw->AddRectFilled(smMin, smMax, IM_COL32(192, 192, 192, 255));
            ImU32 tl = isSmDown ? IM_COL32(128, 128, 128, 255) : IM_COL32(255, 255, 255, 255);
            ImU32 br = isSmDown ? IM_COL32(255, 255, 255, 255) : IM_COL32(128, 128, 128, 255);
            draw->AddLine(smMin, ImVec2(smMax.x, smMin.y), tl, 2.0f);
            draw->AddLine(smMin, ImVec2(smMin.x, smMax.y), tl, 2.0f);
            draw->AddLine(ImVec2(smMin.x, smMax.y), smMax, br, 2.0f);
            draw->AddLine(ImVec2(smMax.x, smMin.y), smMax, br, 2.0f);

            ImVec2 smC = ImVec2(smX + smW * 0.5f + (isSmDown ? 1.0f : 0.0f), smY + smH * 0.5f + (isSmDown ? 1.0f : 0.0f));
            draw->AddCircleFilled(smC, 12.0f, IM_COL32(255, 220, 0, 255));
            draw->AddCircle(smC, 12.0f, IM_COL32(0, 0, 0, 255), 0, 1.5f);

            if (game.status == GameStatus::Win) {
                draw->AddRectFilled(ImVec2(smC.x - 9.0f, smC.y - 5.0f), ImVec2(smC.x + 9.0f, smC.y - 1.0f), IM_COL32(0, 0, 0, 255), 2.0f);
                draw->AddLine(ImVec2(smC.x - 6.0f, smC.y + 4.0f), ImVec2(smC.x + 6.0f, smC.y + 4.0f), IM_COL32(0, 0, 0, 255), 2.0f);
            } else if (game.status == GameStatus::Defeat) {
                draw->AddLine(ImVec2(smC.x - 6.0f, smC.y - 5.0f), ImVec2(smC.x - 2.0f, smC.y - 1.0f), IM_COL32(0, 0, 0, 255), 1.5f);
                draw->AddLine(ImVec2(smC.x - 2.0f, smC.y - 5.0f), ImVec2(smC.x - 6.0f, smC.y - 1.0f), IM_COL32(0, 0, 0, 255), 1.5f);
                draw->AddLine(ImVec2(smC.x + 2.0f, smC.y - 5.0f), ImVec2(smC.x + 6.0f, smC.y - 1.0f), IM_COL32(0, 0, 0, 255), 1.5f);
                draw->AddLine(ImVec2(smC.x + 6.0f, smC.y - 5.0f), ImVec2(smC.x + 2.0f, smC.y - 1.0f), IM_COL32(0, 0, 0, 255), 1.5f);
                draw->AddLine(ImVec2(smC.x - 5.0f, smC.y + 6.0f), ImVec2(smC.x + 5.0f, smC.y + 6.0f), IM_COL32(0, 0, 0, 255), 1.5f);
            } else if (isCellPressed) {
                draw->AddCircleFilled(ImVec2(smC.x - 4.5f, smC.y - 3.0f), 1.5f, IM_COL32(0, 0, 0, 255));
                draw->AddCircleFilled(ImVec2(smC.x + 4.5f, smC.y - 3.0f), 1.5f, IM_COL32(0, 0, 0, 255));
                draw->AddCircle(ImVec2(smC.x, smC.y + 4.0f), 3.0f, IM_COL32(0, 0, 0, 255), 0, 1.5f);
            } else {
                draw->AddCircleFilled(ImVec2(smC.x - 4.5f, smC.y - 3.0f), 1.5f, IM_COL32(0, 0, 0, 255));
                draw->AddCircleFilled(ImVec2(smC.x + 4.5f, smC.y - 3.0f), 1.5f, IM_COL32(0, 0, 0, 255));
                draw->AddLine(ImVec2(smC.x - 5.0f, smC.y + 4.0f), ImVec2(smC.x + 5.0f, smC.y + 4.0f), IM_COL32(0, 0, 0, 255), 1.5f);
            }
        } else {
            ImU32 smBg = isSmHovered ? theme.pillBtnHover : theme.cellCovered;
            ImU32 smBorder = isSmHovered ? theme.pillBtnBorder : IM_COL32(255, 255, 255, 90);
            if (isSmHovered) {
                DrawCrispShadow(draw, smMin, smMax, 10.0f, theme.pillBtnBorder);
            }
            draw->AddRectFilled(smMin, smMax, smBg, 10.0f);
            draw->AddRectFilled(smMin, ImVec2(smMax.x, smMin.y + smH * 0.44f), IM_COL32(255, 255, 255, 28), 10.0f, ImDrawFlags_RoundCornersTop);
            draw->AddRect(smMin, smMax, smBorder, 10.0f, 0, isSmHovered ? 2.2f : 1.4f);

            ImVec2 fSize = fontButton->CalcTextSizeA(22.0f, FLT_MAX, 0.0f, faceStr);
            ImVec2 fPos = ImVec2(smMin.x + (smW - fSize.x) * 0.5f, smMin.y + (smH - fSize.y) * 0.5f);
            draw->AddText(fontButton, 22.0f, fPos, IM_COL32(255, 255, 255, 255), faceStr);
        }

        if (isSmClicked) {
            ResetGame();
        }

        int hours = (int)game.totalTime / 3600;
        int minutes = ((int)game.totalTime % 3600) / 60;
        int seconds = (int)game.totalTime % 60;

        float rightX = barMax.x - 20.0f;
        char fpsBuf[32];
        snprintf(fpsBuf, sizeof(fpsBuf), "🚀 %d FPS", (int)ImGui::GetIO().Framerate);
        ImVec2 fpsSize = fontBody->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, fpsBuf);
        rightX -= fpsSize.x;
        draw->AddText(fontBody, 18.0f, ImVec2(rightX, curY + 6.0f), IM_COL32(50, 255, 120, 255), fpsBuf);

        rightX -= 30.0f;
        char bombBuf[32];
        snprintf(bombBuf, sizeof(bombBuf), "💣 %d / %d", game.flagsCount, game.NumberOfBomb);
        ImVec2 bombSize = fontBody->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, bombBuf);
        rightX -= bombSize.x;
        draw->AddText(fontBody, 18.0f, ImVec2(rightX, curY + 6.0f), IM_COL32(255, 60, 80, 255), bombBuf);

        rightX -= 30.0f;
        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "⏱️ %02d:%02d:%02d", hours, minutes, seconds);
        ImVec2 timeSize = fontBody->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, timeBuf);
        rightX -= timeSize.x;
        draw->AddText(fontBody, 18.0f, ImVec2(rightX, curY + 6.0f), IM_COL32(255, 215, 0, 255), timeBuf);
    }

    void RenderMinesweeperGrid(const LocStrings& str, const VisualTheme& theme, float dt) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 mousePos = ImGui::GetMousePos();
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();

        ImVec2 canvasMin = ImVec2(winPos.x, winPos.y + 70.0f);
        ImVec2 canvasMax = ImVec2(winPos.x + winSize.x, winPos.y + winSize.y);
        ImVec2 canvasSize = ImVec2(canvasMax.x - canvasMin.x, canvasMax.y - canvasMin.y);

        bool isGameOver = (game.status == GameStatus::Win || game.status == GameStatus::Defeat);
        bool isMouseOverCanvas = (mousePos.x >= canvasMin.x && mousePos.x <= canvasMax.x && mousePos.y >= canvasMin.y && mousePos.y <= canvasMax.y);

        if (showMinimap) {
            ImVec2 mBoxMin = ImVec2(winPos.x + winSize.x - 240.0f, winPos.y + winSize.y - 240.0f);
            ImVec2 mBoxMax = ImVec2(winPos.x + winSize.x - 10.0f, winPos.y + winSize.y - 10.0f);
            if (mousePos.x >= mBoxMin.x && mousePos.x <= mBoxMax.x && mousePos.y >= mBoxMin.y && mousePos.y <= mBoxMax.y) {
                isMouseOverCanvas = false;
            }
        }

        if (isGameOver) {
            float panelW = 490.0f;
            float panelH = 210.0f;
            ImVec2 pPos = ImVec2(winPos.x + (winSize.x - panelW) * 0.5f, winPos.y + (winSize.y - panelH) * 0.5f);
            if (mousePos.x >= pPos.x && mousePos.x <= pPos.x + panelW && mousePos.y >= pPos.y && mousePos.y <= pPos.y + panelH) {
                isMouseOverCanvas = false;
            }
        }

        if (justResetGame) {
            isMouseOverCanvas = false;
            justResetGame = false;
        }

        float panSpeed = 650.0f * dt / std::clamp(cameraZoom, 0.2f, 2.0f);
        if (ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_UpArrow)) cameraOffset.y += panSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_S) || ImGui::IsKeyDown(ImGuiKey_DownArrow)) cameraOffset.y -= panSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_LeftArrow)) cameraOffset.x += panSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_D) || ImGui::IsKeyDown(ImGuiKey_RightArrow)) cameraOffset.x -= panSpeed;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            ResetGame();
        }

        if (isMouseOverCanvas && io.MouseWheel != 0.0f) {
            float zoomFactor = (io.MouseWheel > 0) ? 1.15f : 0.8695f;
            float newZoom = std::clamp(cameraZoom * zoomFactor, 0.15f, 5.0f);

            ImVec2 mouseRel = ImVec2(mousePos.x - canvasMin.x - cameraOffset.x, mousePos.y - canvasMin.y - cameraOffset.y);
            cameraOffset.x -= mouseRel.x * (newZoom / cameraZoom - 1.0f);
            cameraOffset.y -= mouseRel.y * (newZoom / cameraZoom - 1.0f);
            cameraZoom = newZoom;
        }

        float baseCellSize = 32.0f;
        float cellSize = baseCellSize * cameraZoom;
        float spacing = 2.0f * cameraZoom;
        float step = cellSize + spacing;

        if (isMouseOverCanvas) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                isRmbDown = true;
                rmbPressPos = mousePos;
                cameraOffsetAtRmbPress = cameraOffset;
                isRmbDragging = false;
            }
        }

        if (isRmbDown) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                float dragDist = std::hypot(mousePos.x - rmbPressPos.x, mousePos.y - rmbPressPos.y);
                if (dragDist > 4.0f) {
                    isRmbDragging = true;
                    cameraOffset.x = cameraOffsetAtRmbPress.x + (mousePos.x - rmbPressPos.x);
                    cameraOffset.y = cameraOffsetAtRmbPress.y + (mousePos.y - rmbPressPos.y);
                }
            } else {
                if (!isRmbDragging && isMouseOverCanvas && !isGameOver) {
                    int col = (int)((mousePos.x - canvasMin.x - cameraOffset.x) / step);
                    int row = (int)((mousePos.y - canvasMin.y - cameraOffset.y) / step);
                    if (col >= 0 && col < game.SquareX && row >= 0 && row < game.SquareY) {
                        int idx = row * game.SquareX + col;
                        game.ToggleFlag(idx);
                        SoundEngine::Get().Play(SoundType::Flag);
                    }
                }
                isRmbDown = false;
                isRmbDragging = false;
            }
        }

        isCellPressed = (isMouseOverCanvas && !isGameOver && ImGui::IsMouseDown(ImGuiMouseButton_Left));

        int startCol = std::clamp((int)((-cameraOffset.x) / step), 0, (int)game.SquareX);
        int endCol   = std::clamp(startCol + (int)(canvasSize.x / step) + 2, 0, (int)game.SquareX);
        int startRow = std::clamp((int)((-cameraOffset.y) / step), 0, (int)game.SquareY);
        int endRow   = std::clamp(startRow + (int)(canvasSize.y / step) + 2, 0, (int)game.SquareY);

        culledCellsThisFrame = (int)(game.SquareX * game.SquareY) - ((endCol - startCol) * (endRow - startRow));

        int hoveredCellThisFrame = -1;

        for (int r = startRow; r < endRow; r++) {
            for (int c = startCol; c < endCol; c++) {
                int idx = r * game.SquareX + c;
                const fieldState& cell = game.FS[idx];

                ImVec2 cellMin = ImVec2(canvasMin.x + cameraOffset.x + c * step, canvasMin.y + cameraOffset.y + r * step);
                ImVec2 cellMax = ImVec2(cellMin.x + cellSize, cellMin.y + cellSize);

                if (cellMax.x < canvasMin.x || cellMin.x > canvasMax.x || cellMax.y < canvasMin.y || cellMin.y > canvasMax.y) continue;

                bool isHovered = isMouseOverCanvas && !isGameOver && (mousePos.x >= cellMin.x && mousePos.x < cellMax.x && mousePos.y >= cellMin.y && mousePos.y < cellMax.y);
                if (isHovered && !cell.visible) hoveredCellThisFrame = idx;

                ImVec2 drawMin = cellMin;
                ImVec2 drawMax = cellMax;

                if (isHovered && !cell.visible) {
                    float pop = 3.0f * std::clamp(cameraZoom, 0.4f, 1.4f);
                    drawMin.x -= pop;
                    drawMin.y -= pop;
                    drawMax.x += pop;
                    drawMax.y += pop;

                    ImVec2 sMin = ImVec2(drawMin.x - 2, drawMin.y - 1);
                    ImVec2 sMax = ImVec2(drawMax.x + 2, drawMax.y + 4);
                    drawList->AddRectFilled(sMin, sMax, (theme.pillBtnBorder & 0x00FFFFFF) | 0x77000000, 4.0f * cameraZoom);
                }

                if (!isGameOver && isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    if (game.firstClick) {
                        game.ManagerBomb(idx);
                        SoundEngine::Get().Play(SoundType::OpenCell);
                    } else if (!cell.flaged) {
                        game.IterativeUnlock(idx);
                        if (game.status == GameStatus::Defeat) {
                            SoundEngine::Get().Play(SoundType::Defeat);
                            particles.SpawnExplosion(ImVec2((cellMin.x + cellMax.x) * 0.5f, (cellMin.y + cellMax.y) * 0.5f));
                            shakeTimer = 0.45f;
                            shakeIntensity = 12.0f;
                        } else if (game.status == GameStatus::Win) {
                            SoundEngine::Get().Play(SoundType::Win);
                            if (!winCelebrated) {
                                winCelebrated = true;
                                particles.SpawnConfetti(ImVec2((canvasMin.x + canvasMax.x) * 0.5f, canvasMin.y + 50.0f));
                            }
                        } else {
                            SoundEngine::Get().Play(SoundType::OpenCell);
                        }
                    }
                }

                if (cell.visible) {
                    if (cell.mine) {
                        drawList->AddRectFilled(drawMin, drawMax, theme.mineColor, 3.0f * cameraZoom);
                        drawList->AddRect(drawMin, drawMax, IM_COL32(255, 255, 255, 220), 3.0f * cameraZoom, 0, 1.5f);

                        if (cellSize > 10.0f) {
                            ImVec2 center = ImVec2((drawMin.x + drawMax.x) * 0.5f, (drawMin.y + drawMax.y) * 0.5f);
                            float radius = cellSize * 0.28f;

                            drawList->AddLine(ImVec2(center.x - radius * 1.4f, center.y), ImVec2(center.x + radius * 1.4f, center.y), IM_COL32(15, 15, 15, 255), 2.5f * cameraZoom);
                            drawList->AddLine(ImVec2(center.x, center.y - radius * 1.4f), ImVec2(center.x, center.y + radius * 1.4f), IM_COL32(15, 15, 15, 255), 2.5f * cameraZoom);
                            drawList->AddLine(ImVec2(center.x - radius, center.y - radius), ImVec2(center.x + radius, center.y + radius), IM_COL32(15, 15, 15, 255), 2.0f * cameraZoom);
                            drawList->AddLine(ImVec2(center.x + radius, center.y - radius), ImVec2(center.x - radius, center.y + radius), IM_COL32(15, 15, 15, 255), 2.0f * cameraZoom);

                            drawList->AddCircleFilled(center, radius, IM_COL32(35, 35, 35, 255));
                            drawList->AddCircleFilled(ImVec2(center.x - radius * 0.3f, center.y - radius * 0.3f), radius * 0.55f, IM_COL32(90, 90, 90, 255));

                            float pulse = (std::sin(bgAnimTime * 8.5f) + 1.0f) * 0.5f;
                            drawList->AddCircleFilled(center, radius * 0.45f, IM_COL32(255, (uint32_t)(pulse * 90), (uint32_t)(pulse * 90), 255));
                            drawList->AddCircleFilled(ImVec2(center.x - radius * 0.25f, center.y - radius * 0.25f), radius * 0.18f, IM_COL32(255, 255, 255, 240));
                        }
                    } else {
                        drawList->AddRectFilled(drawMin, drawMax, theme.cellRevealed, 2.5f * cameraZoom);
                        
                        drawList->AddLine(drawMin, ImVec2(drawMax.x, drawMin.y), IM_COL32(0, 0, 0, 140), 1.6f * cameraZoom);
                        drawList->AddLine(drawMin, ImVec2(drawMin.x, drawMax.y), IM_COL32(0, 0, 0, 140), 1.6f * cameraZoom);
                        drawList->AddLine(ImVec2(drawMin.x, drawMax.y), drawMax, IM_COL32(255, 255, 255, 35), 1.2f * cameraZoom);
                        drawList->AddLine(ImVec2(drawMax.x, drawMin.y), drawMax, IM_COL32(255, 255, 255, 35), 1.2f * cameraZoom);

                        drawList->AddRect(drawMin, drawMax, theme.cellRevealedBorder, 2.5f * cameraZoom, 0, 1.2f);

                        if (cell.bombNearby > 0 && cellSize >= 8.0f) {
                            char buf[4];
                            snprintf(buf, sizeof(buf), "%d", cell.bombNearby);
                            ImU32 numColor = theme.digitColors[cell.bombNearby];

                            if (cellSize > 16.0f) {
                                float fontSize = cellSize * 0.65f;
                                ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, buf);
                                ImVec2 textPos = ImVec2((drawMin.x + drawMax.x - textSize.x) * 0.5f, (drawMin.y + drawMax.y - textSize.y) * 0.5f);
                                
                                drawList->AddText(ImGui::GetFont(), fontSize, ImVec2(textPos.x + 1.2f, textPos.y + 1.2f), IM_COL32(0, 0, 0, 240), buf);
                                drawList->AddText(ImGui::GetFont(), fontSize, textPos, numColor, buf);
                            } else {
                                drawList->AddRectFilled(ImVec2(drawMin.x + 2, drawMin.y + 2), ImVec2(drawMax.x - 2, drawMax.y - 2), numColor);
                            }
                        }
                    }
                } else if (cell.flaged) {
                    drawList->AddRectFilled(drawMin, drawMax, theme.cellCovered, 3.5f * cameraZoom);
                    drawList->AddRect(drawMin, drawMax, theme.flagColor, 3.5f * cameraZoom, 0, 1.6f);

                    if (cellSize >= 10.0f) {
                        ImVec2 base1 = ImVec2(drawMin.x + cellSize * 0.18f, drawMax.y - cellSize * 0.14f);
                        ImVec2 base2 = ImVec2(drawMin.x + cellSize * 0.52f, drawMax.y - cellSize * 0.14f);
                        drawList->AddLine(base1, base2, IM_COL32(220, 220, 220, 255), 2.5f * cameraZoom);

                        ImVec2 poleBot = ImVec2(drawMin.x + cellSize * 0.35f, drawMax.y - cellSize * 0.14f);
                        ImVec2 poleTop = ImVec2(drawMin.x + cellSize * 0.35f, drawMin.y + cellSize * 0.16f);
                        drawList->AddLine(poleBot, poleTop, IM_COL32(250, 250, 250, 255), 2.5f * cameraZoom);
                        drawList->AddCircleFilled(poleTop, 2.0f * cameraZoom, IM_COL32(255, 215, 0, 255));

                        float waveOffset = std::sin(bgAnimTime * 7.5f + c * 0.4f) * 2.2f * cameraZoom;
                        ImVec2 f1 = poleTop;
                        ImVec2 f2 = ImVec2(drawMin.x + cellSize * 0.85f + waveOffset, drawMin.y + cellSize * 0.38f);
                        ImVec2 f3 = ImVec2(drawMin.x + cellSize * 0.35f, drawMin.y + cellSize * 0.58f);

                        drawList->AddTriangleFilled(f1, f2, f3, theme.flagColor);
                        drawList->AddTriangle(f1, f2, f3, IM_COL32(255, 255, 255, 240), 1.3f * cameraZoom);
                    }
                } else {
                    ImU32 cellBg = isHovered ? theme.cellCoveredHover : theme.cellCovered;
                    drawList->AddRectFilled(drawMin, drawMax, cellBg, 3.5f * cameraZoom);

                    if (theme.isRetro) {
                        drawList->AddLine(drawMin, ImVec2(drawMax.x, drawMin.y), IM_COL32(255, 255, 255, 255), 2.2f * cameraZoom);
                        drawList->AddLine(drawMin, ImVec2(drawMin.x, drawMax.y), IM_COL32(255, 255, 255, 255), 2.2f * cameraZoom);
                        drawList->AddLine(ImVec2(drawMin.x, drawMax.y), drawMax, IM_COL32(0, 0, 0, 255), 2.2f * cameraZoom);
                        drawList->AddLine(ImVec2(drawMax.x, drawMin.y), drawMax, IM_COL32(0, 0, 0, 255), 2.2f * cameraZoom);
                    } else {
                        drawList->AddLine(drawMin, ImVec2(drawMax.x - 1, drawMin.y), IM_COL32(255, 255, 255, 95), 1.6f * cameraZoom);
                        drawList->AddLine(drawMin, ImVec2(drawMin.x, drawMax.y - 1), IM_COL32(255, 255, 255, 95), 1.6f * cameraZoom);
                        drawList->AddLine(ImVec2(drawMin.x + 1, drawMax.y), drawMax, IM_COL32(0, 0, 0, 130), 1.6f * cameraZoom);
                        drawList->AddLine(ImVec2(drawMax.x, drawMin.y + 1), drawMax, IM_COL32(0, 0, 0, 130), 1.6f * cameraZoom);

                        if (cellSize >= 22.0f) {
                            float rOff = 3.5f * cameraZoom;
                            float rRad = 1.2f * cameraZoom;
                            ImU32 rCol = IM_COL32(255, 255, 255, 75);
                            drawList->AddCircleFilled(ImVec2(drawMin.x + rOff, drawMin.y + rOff), rRad, rCol);
                            drawList->AddCircleFilled(ImVec2(drawMax.x - rOff, drawMin.y + rOff), rRad, rCol);
                            drawList->AddCircleFilled(ImVec2(drawMin.x + rOff, drawMax.y - rOff), rRad, rCol);
                            drawList->AddCircleFilled(ImVec2(drawMax.x - rOff, drawMax.y - rOff), rRad, rCol);
                        }

                        drawList->AddRect(drawMin, drawMax, theme.cellCoveredBorder, 3.5f * cameraZoom, 0, 1.4f);
                    }
                }
            }
        }

        particles.UpdateAndRender(drawList, dt);

        if (hoveredCellThisFrame != -1 && hoveredCellThisFrame != lastHoveredCellIdx) {
            SoundEngine::Get().Play(SoundType::Hover);
        }
        lastHoveredCellIdx = hoveredCellThisFrame;
    }

    void RenderMinimap(const VisualTheme& theme) {
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();

        float miniW = 210.0f;
        float miniH = 210.0f;
        ImVec2 miniMin = ImVec2(winPos.x + winSize.x - miniW - 14.0f, winPos.y + winSize.y - miniH - 14.0f);
        ImVec2 miniMax = ImVec2(miniMin.x + miniW, miniMin.y + miniH);
        ImVec2 miniSize = ImVec2(miniW, miniH);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 mousePos = ImGui::GetMousePos();

        drawList->AddRectFilled(ImVec2(miniMin.x - 3, miniMin.y - 3), ImVec2(miniMax.x + 3, miniMax.y + 3), IM_COL32(0, 0, 0, 180), 10.0f);
        drawList->AddRectFilled(miniMin, miniMax, IM_COL32(10, 14, 22, 245), 8.0f);
        drawList->AddRect(miniMin, miniMax, theme.pillBtnBorder, 8.0f, 0, 2.0f);

        float scaleX = miniSize.x / (float)game.SquareX;
        float scaleY = miniSize.y / (float)game.SquareY;

        float baseCellSize = 32.0f;
        float cellSize = baseCellSize * cameraZoom;
        float step = cellSize + 2.0f * cameraZoom;

        ImVec2 canvasMin = ImVec2(winPos.x, winPos.y + 70.0f);
        ImVec2 canvasSize = ImVec2(winSize.x, winSize.y - 70.0f);

        int sampleN = 75;
        float stepC = (float)game.SquareX / sampleN;
        float stepR = (float)game.SquareY / sampleN;
        float pW = miniSize.x / sampleN;
        float pH = miniSize.y / sampleN;

        for (int gy = 0; gy < sampleN; gy++) {
            int r = (int)(gy * stepR);
            if (r >= game.SquareY) r = game.SquareY - 1;
            int rowOffset = r * game.SquareX;

            for (int gx = 0; gx < sampleN; gx++) {
                int c = (int)(gx * stepC);
                if (c >= game.SquareX) c = game.SquareX - 1;

                const fieldState& cs = game.FS[rowOffset + c];
                if (cs.visible) {
                    ImVec2 p0 = ImVec2(miniMin.x + gx * pW, miniMin.y + gy * pH);
                    ImVec2 p1 = ImVec2(p0.x + pW + 0.5f, p0.y + pH + 0.5f);
                    ImU32 col = cs.mine ? theme.mineColor : theme.cellRevealedBorder;
                    drawList->AddRectFilled(p0, p1, (col & 0x00FFFFFF) | 0xDD000000);
                } else if (cs.flaged) {
                    ImVec2 p0 = ImVec2(miniMin.x + gx * pW, miniMin.y + gy * pH);
                    ImVec2 p1 = ImVec2(p0.x + pW + 0.5f, p0.y + pH + 0.5f);
                    drawList->AddRectFilled(p0, p1, theme.flagColor);
                }
            }
        }

        bool isMouseInMinimap = (mousePos.x >= miniMin.x && mousePos.x <= miniMax.x && mousePos.y >= miniMin.y && mousePos.y <= miniMax.y);
        if (isMouseInMinimap && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            float targetC = (mousePos.x - miniMin.x) / scaleX;
            float targetR = (mousePos.y - miniMin.y) / scaleY;

            cameraOffset.x = canvasSize.x * 0.5f - targetC * step;
            cameraOffset.y = canvasSize.y * 0.5f - targetR * step;
        }

        float col0 = -cameraOffset.x / step;
        float row0 = -cameraOffset.y / step;
        float col1 = (canvasSize.x - cameraOffset.x) / step;
        float row1 = (canvasSize.y - cameraOffset.y) / step;

        ImVec2 vMin = ImVec2(miniMin.x + col0 * scaleX, miniMin.y + row0 * scaleY);
        ImVec2 vMax = ImVec2(miniMin.x + col1 * scaleX, miniMin.y + row1 * scaleY);

        ImVec2 cvMin = ImVec2(std::clamp(vMin.x, miniMin.x, miniMax.x), std::clamp(vMin.y, miniMin.y, miniMax.y));
        ImVec2 cvMax = ImVec2(std::clamp(vMax.x, miniMin.x, miniMax.x), std::clamp(vMax.y, miniMin.y, miniMax.y));

        if (cvMax.x > cvMin.x && cvMax.y > cvMin.y) {
            drawList->AddRectFilled(cvMin, cvMax, (theme.pillBtnBorder & 0x00FFFFFF) | 0x28000000);
            drawList->AddRect(cvMin, cvMax, theme.pillBtnBorder, 0, 0, 2.0f);

            float bLen = 6.0f;
            drawList->AddLine(cvMin, ImVec2(cvMin.x + bLen, cvMin.y), IM_COL32(255, 255, 255, 255), 2.0f);
            drawList->AddLine(cvMin, ImVec2(cvMin.x, cvMin.y + bLen), IM_COL32(255, 255, 255, 255), 2.0f);
            drawList->AddLine(cvMax, ImVec2(cvMax.x - bLen, cvMax.y), IM_COL32(255, 255, 255, 255), 2.0f);
            drawList->AddLine(cvMax, ImVec2(cvMax.x, cvMax.y - bLen), IM_COL32(255, 255, 255, 255), 2.0f);
        }

        ImVec2 miniC = ImVec2(miniMin.x + miniSize.x * 0.5f, miniMin.y + miniSize.y * 0.5f);
        float sweepA = bgAnimTime * 2.4f;
        ImVec2 sweepEnd = ImVec2(miniC.x + std::cos(sweepA) * miniSize.x * 0.70f,
                                 miniC.y + std::sin(sweepA) * miniSize.y * 0.70f);
        drawList->AddLine(miniC, sweepEnd, (theme.pillBtnBorder & 0x00FFFFFF) | 0x88000000, 1.8f);

        drawList->AddCircle(miniC, miniSize.x * 0.25f, (theme.pillBtnBorder & 0x00FFFFFF) | 0x44000000, 32, 1.0f);
        drawList->AddCircle(miniC, miniSize.x * 0.45f, (theme.pillBtnBorder & 0x00FFFFFF) | 0x44000000, 32, 1.0f);

        ImGui::PushFont(fontBody);
        int centerC = (int)((canvasSize.x * 0.5f - cameraOffset.x) / step);
        int centerR = (int)((canvasSize.y * 0.5f - cameraOffset.y) / step);
        centerC = std::clamp(centerC, 0, (int)game.SquareX - 1);
        centerR = std::clamp(centerR, 0, (int)game.SquareY - 1);

        char coordBuf[32];
        snprintf(coordBuf, sizeof(coordBuf), "X:%d Y:%d", centerC, centerR);
        drawList->AddText(fontBody, 13.0f, ImVec2(miniMin.x + 8.0f, miniMin.y + 6.0f), IM_COL32(255, 255, 255, 220), coordBuf);

        char zoomBuf[16];
        snprintf(zoomBuf, sizeof(zoomBuf), "%.1fx", cameraZoom);
        ImVec2 zSize = fontBody->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, zoomBuf);
        drawList->AddText(fontBody, 13.0f, ImVec2(miniMax.x - zSize.x - 8.0f, miniMin.y + 6.0f), theme.pillBtnBorder, zoomBuf);
        ImGui::PopFont();
    }

    void RenderStatusOverlay(const LocStrings& str, const VisualTheme& theme, float dt) {
        if (game.status == GameStatus::Win || game.status == GameStatus::Defeat) {
            ImVec2 winSize = ImGui::GetWindowSize();
            ImVec2 winPos = ImGui::GetWindowPos();

            float panelW = 490.0f;
            float panelH = 210.0f;
            ImVec2 pPos = ImVec2(winPos.x + (winSize.x - panelW) * 0.5f, winPos.y + (winSize.y - panelH) * 0.5f);

            DrawCyberPanel(pPos, ImVec2(pPos.x + panelW, pPos.y + panelH), theme.cellCovered, theme.pillBtnBorder, 14.0f, theme.isRetro);

            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImGui::PushFont(fontBody);
            float textY = pPos.y + 22.0f;

            if (game.status == GameStatus::Win) {
                draw->AddText(fontBody, 22.0f, ImVec2(pPos.x + 30.0f, textY), IM_COL32(0, 255, 120, 255), str.winTitle);
            } else {
                draw->AddText(fontBody, 22.0f, ImVec2(pPos.x + 30.0f, textY), IM_COL32(255, 60, 60, 255), str.defeatTitle);
            }
            textY += 38.0f;

            char bufTime[64], bufField[64];
            snprintf(bufTime, sizeof(bufTime), "⏱️ Time: %.2f sec", game.totalTime);
            snprintf(bufField, sizeof(bufField), "🗺️ Grid: %d × %d (%d %s)", game.SquareX, game.SquareY, game.SquareX * game.SquareY, str.cellsUnit);

            draw->AddText(fontBody, 18.0f, ImVec2(pPos.x + 30.0f, textY), IM_COL32(240, 240, 240, 255), bufTime);
            textY += 26.0f;
            draw->AddText(fontBody, 18.0f, ImVec2(pPos.x + 30.0f, textY), IM_COL32(240, 240, 240, 255), bufField);
            textY += 40.0f;

            if (DrawPillButton(str.playAgain, ImVec2(pPos.x + 25.0f, textY), ImVec2(panelW - 50.0f, 44.0f), theme, true, fontButton, bgAnimTime, theme.isRetro)) {
                ResetGame();
            }

            ImGui::PopFont();
        }
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        RenderApp* app = (RenderApp*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
            return 0;
        }
        case WM_SIZE:
            if (app && app->d3dDevice && wParam != SIZE_MINIMIZED) {
                app->CleanupRenderTarget();
                app->swapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                app->CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
};

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    RenderApp app;
    if (!app.InitWindowAndD3D(1400, 900)) {
        return 1;
    }

    app.RunLoop();
    app.Shutdown();

    return 0;
}