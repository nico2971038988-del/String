#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <vector>

// 管理时间轴中的所有节奏线。
// 同一时刻可以存在多条线，每个事件只触发一次。
class RhythmLine
{
public:
    RhythmLine() = default;

    void update(float deltaTime, float musicTimeSeconds);
    void draw(sf::RenderWindow& window) const;

    // 重新播放音乐或主动重置关卡时调用。
    void reset();

private:
    struct ActiveLine
    {
        sf::Vector2f start{};
        sf::Vector2f direction{};

        float currentLength = 0.f;
        float maximumLength = 0.f;
        float growthSpeed = 0.f;
        float elapsedTime = 0.f;
        float duration = 0.f;
        float thickness = 1.f;

        sf::Color color = sf::Color::White;
    };

    void createLine(std::size_t eventIndex, float initialElapsedTime = 0.f);

    std::vector<ActiveLine> activeLines_;
    std::size_t nextEventIndex_ = 0;
    float previousMusicTime_ = 0.f;
};