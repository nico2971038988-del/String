#include "RhythmLine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace
{
    constexpr float Pi = 3.14159265358979323846f;
    constexpr float ScreenWidth = 1280.f;
    constexpr float ScreenHeight = 720.f;
    constexpr float HorizontalLineY = ScreenHeight / 2.f;
    constexpr float VerticalLineX = ScreenWidth / 2.f;
    constexpr float HorizontalLineLength = ScreenWidth;
    constexpr float VerticalLineLength = ScreenHeight;
    constexpr float OutlinePadding = 1.5f;
    constexpr sf::Color OutlineColor{ 70, 45, 35, 210 };

    enum class ScreenSide
    {
        Left,
        Right,
        Top,
        Bottom
    };

    struct RhythmLineEvent
    {
        float time;
        ScreenSide side;
        float position;       // 左右线使用 Y，上下线使用 X
        float angleOffset;    // 相对默认方向的角度偏移
        float length;
        float growthSpeed;
        float thickness;
        float duration;
        sf::Color color;
    };

    // 必须按出现时间从小到大排列；相同时间的事件会在同一帧全部创建。
    constexpr std::array RhythmTimeline{
        RhythmLineEvent{
            1.00f,
            ScreenSide::Left,
            HorizontalLineY - 270.f, // Y=90，处于屏幕上方
            0.f,
            HorizontalLineLength,
            1000.f,
            3.f,
            120.f,
            sf::Color(245, 205, 125, 255)
        },
        RhythmLineEvent{
            2.00f,
            ScreenSide::Right,
            HorizontalLineY + 270.f, // Y=630，处于屏幕下方
            0.f,
            HorizontalLineLength,
            1000.f,
            3.f,
            120.f,
            sf::Color(245, 205, 125, 255)
        },
        RhythmLineEvent{
            3.00f,
            ScreenSide::Bottom,
            VerticalLineX + 600.f, // X=1240，靠近屏幕右侧
            0.f,
            VerticalLineLength,
            2000.f,
            3.f,
            100.f,
            sf::Color(245, 205, 125, 255)
        },
        RhythmLineEvent{
            3.00f,
            ScreenSide::Top,
            VerticalLineX - 600.f, // X=1240，靠近屏幕右侧
            0.f,
            VerticalLineLength,
            2000.f,
            3.f,
            100.f,
            sf::Color(245, 205, 125, 255)
        }
    };

    sf::Vector2f directionFromAngle(ScreenSide side, float angleOffset)
    {
        float baseAngle = 0.f;

        switch (side)
        {
        case ScreenSide::Left:   baseAngle = 0.f;   break;
        case ScreenSide::Right:  baseAngle = 180.f; break;
        case ScreenSide::Top:    baseAngle = 90.f;  break;
        case ScreenSide::Bottom: baseAngle = -90.f; break;
        }

        const float radians = (baseAngle + angleOffset) * Pi / 180.f;
        return { std::cos(radians), std::sin(radians) };
    }

    sf::Vector2f startPosition(ScreenSide side, float position)
    {
        switch (side)
        {
        case ScreenSide::Left:   return { 0.f, position };
        case ScreenSide::Right:  return { ScreenWidth, position };
        case ScreenSide::Top:    return { position, 0.f };
        case ScreenSide::Bottom: return { position, ScreenHeight };
        }

        return {};
    }
} // namespace

void RhythmLine::reset()
{
    activeLines_.clear();
    nextEventIndex_ = 0;
    previousMusicTime_ = 0.f;
}

void RhythmLine::update(float deltaTime, float musicTimeSeconds)
{
    deltaTime = std::max(deltaTime, 0.f);
    musicTimeSeconds = std::max(musicTimeSeconds, 0.f);

    // 音乐回退或重新播放时，清空旧实例并按新时间重新建立状态。
    if (musicTimeSeconds < previousMusicTime_)
    {
        reset();
    }

    // while 会创建所有已经到点的事件，包括多个相同时间的事件。
    while (nextEventIndex_ < RhythmTimeline.size() &&
        musicTimeSeconds >= RhythmTimeline[nextEventIndex_].time)
    {
        const auto& event = RhythmTimeline[nextEventIndex_];
        const float elapsedSinceEvent = musicTimeSeconds - event.time;

        // 跳转音乐位置时，只重建仍处于有效期内的线。
        if (elapsedSinceEvent < event.duration)
        {
            createLine(nextEventIndex_, elapsedSinceEvent);
        }

        ++nextEventIndex_;
    }

    for (ActiveLine& line : activeLines_)
    {
        line.elapsedTime += deltaTime;
        line.currentLength = std::min(
            line.maximumLength,
            line.currentLength + line.growthSpeed * deltaTime);
    }

    activeLines_.erase(
        std::remove_if(
            activeLines_.begin(),
            activeLines_.end(),
            [](const ActiveLine& line)
            {
                return line.elapsedTime >= line.duration;
            }),
        activeLines_.end());

    previousMusicTime_ = musicTimeSeconds;
}

void RhythmLine::createLine(std::size_t eventIndex, float initialElapsedTime)
{
    const RhythmLineEvent& event = RhythmTimeline[eventIndex];

    ActiveLine line;
    line.start = startPosition(event.side, event.position);
    line.direction = directionFromAngle(event.side, event.angleOffset);
    line.maximumLength = event.length;
    line.growthSpeed = event.growthSpeed;
    line.elapsedTime = std::clamp(initialElapsedTime, 0.f, event.duration);
    line.currentLength = std::min(
        event.length,
        event.growthSpeed * line.elapsedTime);
    line.duration = event.duration;
    line.thickness = event.thickness;
    line.color = event.color;

    activeLines_.push_back(line);
}

void RhythmLine::draw(sf::RenderWindow& window) const
{
    for (const ActiveLine& line : activeLines_)
    {
        if (line.duration <= 0.f || line.currentLength <= 0.f)
        {
            continue;
        }

        const float progress = std::clamp(
            line.elapsedTime / line.duration,
            0.f,
            1.f);

        // 前 12% 淡入，剩余时间逐渐淡出。
        const float alphaFactor = progress < 0.12f
            ? progress / 0.12f
            : 1.f - (progress - 0.12f) / 0.88f;

        const float angleDegrees =
            std::atan2(line.direction.y, line.direction.x) * 180.f / Pi;

        const auto scaledAlpha = [alphaFactor](std::uint8_t alpha)
            {
                return static_cast<std::uint8_t>(
                    static_cast<float>(alpha) *
                    std::clamp(1.f, 0.f, 1.f));
            };

        // 先画较粗的深棕描边。描边和主体共用同一起点、长度与旋转中心，
        // 因此无论线从哪个屏幕边缘伸长，都不会发生错位。
        const float outlineThickness =
            line.thickness + OutlinePadding * 2.f;
        sf::RectangleShape outline(
            { line.currentLength, outlineThickness });
        outline.setOrigin({ 0.f, outlineThickness / 2.f });
        outline.setPosition(line.start);
        outline.setRotation(sf::degrees(angleDegrees));

        sf::Color outlineColor = OutlineColor;
        outlineColor.a = scaledAlpha(OutlineColor.a);
        outline.setFillColor(outlineColor);
        window.draw(outline, sf::RenderStates(sf::BlendAlpha));

        // 再在描边中央绘制统一的暖金色主体。
        sf::RectangleShape shape({ line.currentLength, line.thickness });
        shape.setOrigin({ 0.f, line.thickness / 2.f });
        shape.setPosition(line.start);
        shape.setRotation(sf::degrees(angleDegrees));

        sf::Color mainColor = line.color;
        mainColor.a = scaledAlpha(line.color.a);
        shape.setFillColor(mainColor);

        window.draw(shape, sf::RenderStates(sf::BlendAlpha));
    }
}