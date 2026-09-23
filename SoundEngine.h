#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <thread>
#include <cmath>
#include <algorithm>

enum class SoundType { Hover, OpenCell, Flag, Defeat, Win };

class SoundEngine {
public:
    static SoundEngine& Get() {
        static SoundEngine instance;
        return instance;
    }

    bool soundEnabled = true;
    bool hoverSoundEnabled = true;
    float masterVolume = 0.5f;
    float pitchMultiplier = 1.0f;

    void Play(SoundType type) {
        if (!soundEnabled || masterVolume <= 0.001f) return;
        if (type == SoundType::Hover && !hoverSoundEnabled) return;

        float pm = pitchMultiplier;
        std::thread([this, type, pm]() {
            GenerateAndPlayPCM(type, pm);
        }).detach();
    }

private:
    SoundEngine() = default;

    void GenerateAndPlayPCM(SoundType type, float pitchMult) {
        const DWORD sampleRate = 44100;
        float durationSec = 0.03f;
        
        switch (type) {
        case SoundType::Hover:    durationSec = 0.012f; break;
        case SoundType::OpenCell: durationSec = 0.035f; break;
        case SoundType::Flag:     durationSec = 0.05f;  break;
        case SoundType::Defeat:   durationSec = 0.25f;  break;
        case SoundType::Win:      durationSec = 0.35f;  break;
        }

        DWORD numSamples = (DWORD)(sampleRate * durationSec);
        std::vector<int16_t> pcmData(numSamples);

        for (DWORD i = 0; i < numSamples; i++) {
            float t = (float)i / sampleRate;
            float sample = 0.0f;

            if (type == SoundType::Hover) {
                float freq = 750.0f * pitchMult;
                float env = std::exp(-t * 300.0f);
                sample = std::sin(2.0f * 3.14159f * freq * t) * env * 0.2f;
            } else if (type == SoundType::OpenCell) {
                float freq = (420.0f + 450.0f * (t / durationSec)) * pitchMult;
                float env = std::sin((t / durationSec) * 3.14159f);
                sample = std::sin(2.0f * 3.14159f * freq * t) * env * 0.4f;
            } else if (type == SoundType::Flag) {
                float env = std::exp(-t * 60.0f);
                sample = (std::sin(2.0f * 3.14159f * 1200.0f * pitchMult * t) + 0.5f * std::sin(2.0f * 3.14159f * 1800.0f * pitchMult * t)) * env * 0.35f;
            } else if (type == SoundType::Defeat) {
                float noise = ((float)(rand() % 2000) / 1000.0f) - 1.0f;
                float env = std::exp(-t * 15.0f);
                float lowSine = std::sin(2.0f * 3.14159f * 90.0f * pitchMult * t);
                sample = (noise * 0.6f + lowSine * 0.4f) * env * 0.6f;
            } else if (type == SoundType::Win) {
                float phase = t / durationSec;
                float freq = 523.25f * pitchMult;
                if (phase > 0.75f) freq = 1046.50f * pitchMult;
                else if (phase > 0.50f) freq = 783.99f * pitchMult;
                else if (phase > 0.25f) freq = 659.25f * pitchMult;
                float env = std::sin(phase * 3.14159f);
                sample = std::sin(2.0f * 3.14159f * freq * t) * env * 0.5f;
            }

            sample *= masterVolume;
            sample = std::clamp(sample, -1.0f, 1.0f);
            pcmData[i] = (int16_t)(sample * 32767.0f);
        }

        WAVEFORMATEX wfx = { 0 };
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 1;
        wfx.nSamplesPerSec = sampleRate;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = 2;
        wfx.nAvgBytesPerSec = sampleRate * 2;

        HWAVEOUT hWaveOut = NULL;
        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
            WAVEHDR header = { 0 };
            header.lpData = (LPSTR)pcmData.data();
            header.dwBufferLength = (DWORD)(pcmData.size() * sizeof(int16_t));
            
            waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));

            while (!(header.dwFlags & WHDR_DONE)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }

            waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutClose(hWaveOut);
        }
    }
};
