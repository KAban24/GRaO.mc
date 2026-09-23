#pragma once
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <immintrin.h>

#pragma pack(push, 1)
struct fieldState {
    uint8_t visible      : 1;
    uint8_t mine         : 1;
    uint8_t flaged       : 1;
    uint8_t nearbyNumber : 1;
    uint8_t bombNearby   : 4;
};
#pragma pack(pop)

static_assert(sizeof(fieldState) == 1, "fieldState MUST be 1 byte!");

struct BombPos {
    uint16_t x;
    uint16_t y;
};

enum class GameStatus { Ready, Playing, Win, Defeat };

class MinesweeperBackend {
public:
    uint16_t SquareX = 1000;
    uint16_t SquareY = 1000;
    int NumberOfBomb = 150000;

    std::vector<fieldState> FS;
    std::vector<BombPos> PB;

    GameStatus status = GameStatus::Ready;
    bool firstClick = true;

    std::chrono::steady_clock::time_point startTime;
    float totalTime = 0.0f;
    int flagsCount = 0;

    void InitField(uint16_t width, uint16_t height, int bombs) {
        SquareX = width;
        SquareY = height;
        
        int maxBombs = (int)(SquareX * SquareY) - 9;
        if (maxBombs < 1) maxBombs = 1;
        NumberOfBomb = std::clamp(bombs, 1, maxBombs);

        size_t totalCells = (size_t)SquareX * SquareY;
        FS.assign(totalCells, { 0, 0, 0, 0, 0 });
        PB.clear();
        PB.reserve(NumberOfBomb);

        status = GameStatus::Ready;
        firstClick = true;
        totalTime = 0.0f;
        flagsCount = 0;
    }

    void IterativeUnlock(int startIdx) {
        if (startIdx < 0 || startIdx >= (int)FS.size()) return;
        if (FS[startIdx].visible || FS[startIdx].flaged) return;

        if (FS[startIdx].mine) {
            status = GameStatus::Defeat;
            RevealAllMines();
            return;
        }

        std::vector<int> bfsStack;
        bfsStack.reserve(8192);
        bfsStack.push_back(startIdx);

        FS[startIdx].visible = true;

        while (!bfsStack.empty()) {
            int idx = bfsStack.back();
            bfsStack.pop_back();

            if (FS[idx].bombNearby > 0) {
                FS[idx].nearbyNumber = true;
                continue;
            }

            int cx = idx % SquareX;
            int cy = idx / SquareX;

            for (int ny = -1; ny <= 1; ny++) {
                int dy = cy + ny;
                if (dy < 0 || dy >= SquareY) continue;

                for (int nx = -1; nx <= 1; nx++) {
                    if (nx == 0 && ny == 0) continue;
                    int dx = cx + nx;
                    if (dx < 0 || dx >= SquareX) continue;

                    int nIdx = dy * SquareX + dx;
                    if (!FS[nIdx].visible && !FS[nIdx].flaged && !FS[nIdx].mine) {
                        FS[nIdx].visible = true;
                        bfsStack.push_back(nIdx);
                    }
                }
            }
        }

        CheckWinConditionSIMD();
    }

    void ManagerBomb(int firstClickIdx) {
        size_t totalCells = (size_t)SquareX * SquareY;
        FS.assign(totalCells, { 0, 0, 0, 0, 0 });
        PB.clear();
        PB.reserve(NumberOfBomb);

        int firstX = firstClickIdx % SquareX;
        int firstY = firstClickIdx / SquareX;

        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_int_distribution<int> distX(0, SquareX - 1);
        std::uniform_int_distribution<int> distY(0, SquareY - 1);

        int placed = 0;
        while (placed < NumberOfBomb) {
            int rx = distX(rng);
            int ry = distY(rng);
            int targetIdx = ry * SquareX + rx;

            bool inSafeZone = (std::abs(rx - firstX) <= 1 && std::abs(ry - firstY) <= 1);
            if (FS[targetIdx].mine || inSafeZone) {
                continue;
            }

            FS[targetIdx].mine = true;
            PB.push_back({ (uint16_t)rx, (uint16_t)ry });
            placed++;
        }

        for (const auto& bomb : PB) {
            int bx = bomb.x;
            int by = bomb.y;

            for (int ny = -1; ny <= 1; ny++) {
                int dy = by + ny;
                if (dy < 0 || dy >= SquareY) continue;

                for (int nx = -1; nx <= 1; nx++) {
                    if (nx == 0 && ny == 0) continue;
                    int dx = bx + nx;
                    if (dx < 0 || dx >= SquareX) continue;

                    int neighborIdx = dy * SquareX + dx;
                    if (!FS[neighborIdx].mine) {
                        FS[neighborIdx].bombNearby++;
                    }
                }
            }
        }

        firstClick = false;
        status = GameStatus::Playing;
        startTime = std::chrono::steady_clock::now();

        IterativeUnlock(firstClickIdx);
    }

    void ToggleFlag(int idx) {
        if (idx < 0 || idx >= (int)FS.size()) return;
        if (FS[idx].visible || status != GameStatus::Playing) return;

        if (FS[idx].flaged) {
            FS[idx].flaged = false;
            flagsCount--;
        } else {
            if (flagsCount < NumberOfBomb) {
                FS[idx].flaged = true;
                flagsCount++;
                CheckWinConditionSIMD();
            }
        }
    }

    void RevealAllMines() {
        for (const auto& bomb : PB) {
            int idx = bomb.y * SquareX + bomb.x;
            FS[idx].visible = true;
        }
    }

    void CheckWinConditionSIMD() {
        if (status != GameStatus::Playing) return;

        size_t totalCells = (size_t)SquareX * SquareY;
        size_t revealedCount = 0;

        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(FS.data());
        size_t simdChunks = totalCells / 32;

        __m256i maskVisible = _mm256_set1_epi8(0x01);
        __m256i maskMine    = _mm256_set1_epi8(0x02);

        for (size_t i = 0; i < simdChunks; i++) {
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr + i * 32));
            __m256i isVis  = _mm256_and_si256(chunk, maskVisible);
            __m256i isMine = _mm256_and_si256(chunk, maskMine);

            __m256i visMatch = _mm256_cmpeq_epi8(isVis, maskVisible);
            __m256i notMineMatch = _mm256_cmpeq_epi8(isMine, _mm256_setzero_si256());
            __m256i validCell = _mm256_and_si256(visMatch, notMineMatch);

            uint32_t mask = _mm256_movemask_epi8(validCell);
            revealedCount += __popcnt(mask);
        }

        for (size_t i = simdChunks * 32; i < totalCells; i++) {
            if (FS[i].visible && !FS[i].mine) {
                revealedCount++;
            }
        }

        size_t targetRevealed = totalCells - NumberOfBomb;
        if (revealedCount >= targetRevealed) {
            status = GameStatus::Win;
            for (const auto& bomb : PB) {
                int idx = bomb.y * SquareX + bomb.x;
                FS[idx].flaged = true;
            }
            flagsCount = NumberOfBomb;
        }
    }

    void UpdateTimer() {
        if (status == GameStatus::Playing) {
            auto now = std::chrono::steady_clock::now();
            totalTime = std::chrono::duration<float>(now - startTime).count();
        }
    }
};
