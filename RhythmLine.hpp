#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <vector>

enum class HitResult {
    None,
    Perfect,
    Great,
    Miss
};

class RhythmLine
{
public:
    RhythmLine() = default;

    struct LineEvent {
        float startTime;
        float endTime;
        sf::Vector2f startPos, endPos;
        float startAngle, endAngle;
        float startAlpha, endAlpha;
    };

    void update(float deltaTime, float musicTimeSeconds);
    void draw(sf::RenderWindow& window) const;
    void reset();

    HitResult onPlayerPressSpace();

private:
    struct ActiveNote
    {
        float hitTime = 0.f;
        float travelDuration = 1.0f;
        float spawnTime = 0.f;
        float size = 20.f;
        int lineIndex = 0;
        float positionOnLine = 0.f;
        bool isHit = false;
    };

    struct HitEffect {
        sf::Vector2f pos;
        float angle;
        float lifetime = 0.3f;
        float maxLifetime = 0.3f;
    };

    struct DynamicLine {
        sf::Vector2f currentPos{ 640.f, 410.f };
        float currentAngle = 0.f;
        float currentAlpha = 220.f;
        float hitPulse = 0.f;
    };

    void createNote(std::size_t eventIndex);
    float easeInOutCubic(float t) const;

    std::vector<ActiveNote> activeNotes_;
    std::vector<HitEffect> hitEffects_;
    std::vector<DynamicLine> lines_;

    std::size_t nextEventIndex_ = 0;
    float currentMusicTime_ = 0.f;
    float globalSceneGlow_ = 0.f; // 🌟 场景全局光照联动系数
};