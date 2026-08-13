#include "RhythmLine.hpp"


#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdint>

namespace
{
    constexpr float Pi = 3.14159265358979323846f;
    constexpr float ScreenWidth = 1280.f;
    constexpr float ScreenHeight = 720.f;

    // 🌟 判定线默认基准点：屏幕底部
    const sf::Vector2f BottomCenter{ ScreenWidth / 2.f, ScreenHeight - 100.f };

    struct NoteEvent {
        float hitTime;
        int lineIndex;        // 0: 主线, 1: 副线
        float positionOnLine; // -0.8(最左) ~ 0.8(最右)
    };

    // =================================================================
    // 🎵 211 秒（3分31秒）真正的全曲完整谱面
    // =================================================================
    const std::vector<NoteEvent> Timeline = {
        // --- 1. Intro 前奏 (0s - 14s) ---
        { 2.00f, 0,  0.0f }, { 4.00f, 0, -0.4f }, { 6.00f, 0,  0.4f },
        { 8.00f, 0, -0.2f }, { 9.50f, 0,  0.2f }, { 11.00f, 0, -0.5f }, { 12.50f, 0,  0.5f },

        // --- 2. Melody 旋律进场 (14s - 28s) ---
        { 14.00f, 0,  0.0f }, { 14.75f, 0, -0.3f }, { 15.50f, 0,  0.3f }, { 17.00f, 0, -0.5f },
        { 18.50f, 0,  0.5f }, { 20.00f, 0, -0.2f }, { 20.75f, 0,  0.2f }, { 21.50f, 0, -0.4f },
        { 23.00f, 0,  0.4f }, { 24.50f, 0, -0.6f }, { 26.00f, 0,  0.0f }, { 27.00f, 0,  0.6f },

        // --- 3. BuildUp 1 (28s - 34s) ---
        { 28.00f, 0, -0.4f }, { 28.50f, 0, -0.2f }, { 29.00f, 0,  0.0f }, { 29.50f, 0,  0.2f },
        { 30.00f, 0,  0.4f }, { 30.33f, 0, -0.3f }, { 30.66f, 0,  0.0f }, { 31.00f, 0,  0.3f },
        { 31.25f, 0, -0.5f }, { 31.50f, 0, -0.2f }, { 31.75f, 0,  0.2f }, { 32.00f, 0,  0.5f },
        { 32.50f, 0, -0.6f }, { 32.75f, 0, -0.3f }, { 33.00f, 0,  0.3f }, { 33.25f, 0,  0.6f },

        // --- 4. 💥 FIRST DROP (34s - 50s) ---
        { 34.00f, 0, -0.5f }, { 34.00f, 0,  0.5f }, { 34.50f, 0, -0.2f }, { 34.75f, 0,  0.2f },
        { 35.50f, 0, -0.6f }, { 36.00f, 1,  0.0f }, { 36.50f, 0,  0.4f }, { 37.00f, 0, -0.4f },
        { 37.50f, 1, -0.3f }, { 37.50f, 1,  0.3f }, { 38.25f, 0,  0.0f }, { 39.00f, 0, -0.7f },
        { 39.00f, 0,  0.7f }, { 40.00f, 0, -0.3f }, { 40.33f, 0,  0.0f }, { 40.66f, 0,  0.3f },
        { 41.50f, 1, -0.5f }, { 42.00f, 1,  0.5f }, { 42.50f, 0, -0.2f }, { 42.75f, 0,  0.2f },
        { 43.50f, 0, -0.6f }, { 43.50f, 1,  0.6f }, { 44.25f, 0,  0.0f }, { 45.00f, 0, -0.4f },
        { 45.50f, 0,  0.4f }, { 46.25f, 1,  0.0f }, { 47.00f, 0, -0.5f }, { 47.00f, 0,  0.5f },
        { 48.00f, 0, -0.2f }, { 48.50f, 0,  0.2f }, { 49.25f, 0,  0.0f },

        // --- 5. Break 1 (50s - 66s) ---
        { 51.00f, 0, -0.3f }, { 53.00f, 0,  0.3f }, { 55.00f, 0, -0.5f }, { 57.00f, 0,  0.5f },
        { 59.00f, 0, -0.2f }, { 61.00f, 0,  0.2f }, { 63.00f, 0,  0.0f }, { 65.00f, 0, -0.4f },

        // --- 6. BuildUp 2 (66s - 72s) ---
        { 66.00f, 0,  0.4f }, { 67.00f, 0, -0.3f }, { 68.00f, 0,  0.3f }, { 69.00f, 0, -0.5f },
        { 70.00f, 0,  0.5f }, { 70.66f, 0,  0.2f }, { 71.25f, 0, -0.3f }, { 71.75f, 0,  0.6f },

        // --- 7. 💥 SECOND DROP (72s - 88s) ---
        { 72.00f, 0, -0.6f }, { 72.00f, 0,  0.6f }, { 72.50f, 1, -0.3f }, { 72.75f, 1,  0.3f },
        { 73.50f, 0,  0.0f }, { 74.00f, 0, -0.5f }, { 74.50f, 0,  0.5f }, { 75.25f, 1,  0.0f },
        { 76.00f, 0, -0.4f }, { 76.00f, 1,  0.4f }, { 77.00f, 0, -0.2f }, { 77.66f, 0,  0.2f },
        { 78.50f, 0, -0.7f }, { 78.50f, 0,  0.7f }, { 80.00f, 1, -0.4f }, { 80.50f, 1,  0.4f },
        { 81.25f, 0,  0.0f }, { 82.00f, 0, -0.3f }, { 82.50f, 0,  0.3f }, { 83.25f, 1, -0.5f },
        { 84.00f, 0, -0.6f }, { 84.50f, 0,  0.6f }, { 86.00f, 0, -0.4f }, { 87.25f, 0,  0.0f },

        // --- 8. Bridge 桥段变奏 (88s - 128s) 🎹 情绪沉淀与长线过渡 ---
        { 88.50f, 0, -0.2f }, { 90.00f, 0,  0.2f }, { 92.00f, 0, -0.4f }, { 94.00f, 0,  0.4f },
        { 96.00f, 0, -0.3f }, { 98.00f, 0,  0.3f }, { 100.00f, 0, -0.5f }, { 102.00f, 0,  0.5f },
        { 104.00f, 0, -0.2f }, { 106.00f, 0,  0.2f }, { 108.00f, 0, -0.4f }, { 110.00f, 0,  0.4f },
        { 112.00f, 0, -0.3f }, { 114.00f, 0,  0.3f }, { 116.00f, 0, -0.5f }, { 118.00f, 0,  0.5f },
        { 120.00f, 0, -0.2f }, { 122.00f, 0,  0.2f }, { 124.00f, 0, -0.4f }, { 126.00f, 0,  0.4f },

        // --- 9. BuildUp 3 第三次极速上升 (128s - 134s) ---
        { 128.00f, 0, -0.4f }, { 128.50f, 0, -0.2f }, { 129.00f, 0,  0.0f }, { 129.50f, 0,  0.2f },
        { 130.00f, 0,  0.4f }, { 130.33f, 0, -0.3f }, { 130.66f, 0,  0.0f }, { 131.00f, 0,  0.3f },
        { 131.25f, 0, -0.5f }, { 131.50f, 0, -0.2f }, { 131.75f, 0,  0.2f }, { 132.00f, 0,  0.5f },
        { 132.50f, 0, -0.6f }, { 132.75f, 0, -0.3f }, { 133.00f, 0,  0.3f }, { 133.25f, 0,  0.6f },
        { 133.50f, 0,  0.0f }, { 133.75f, 0,  0.0f },

        // --- 10. 💥💥 FINAL DROP 终极爆发段 (134s - 166s) 全曲最高高潮！ ---
        { 134.00f, 0, -0.6f }, { 134.00f, 1,  0.6f }, // 跨线双打！
        { 134.50f, 0, -0.3f }, { 134.75f, 0,  0.3f },
        { 135.50f, 1, -0.5f }, { 136.00f, 0,  0.0f },
        { 136.50f, 0,  0.4f }, { 137.00f, 1, -0.4f },
        { 137.50f, 0, -0.3f }, { 137.50f, 1,  0.3f },
        { 138.25f, 0,  0.0f }, { 139.00f, 0, -0.7f }, { 139.00f, 1,  0.7f },
        { 140.00f, 0, -0.3f }, { 140.33f, 0,  0.0f }, { 140.66f, 0,  0.3f },
        { 141.50f, 1, -0.5f }, { 142.00f, 0,  0.5f },
        { 142.50f, 0, -0.2f }, { 142.75f, 1,  0.2f },
        { 143.50f, 0, -0.6f }, { 143.50f, 1,  0.6f },
        { 144.25f, 0,  0.0f }, { 145.00f, 0, -0.4f },
        { 145.50f, 1,  0.4f }, { 146.25f, 0,  0.0f },
        { 147.00f, 0, -0.5f }, { 147.00f, 1,  0.5f },
        { 148.00f, 0, -0.2f }, { 148.50f, 0,  0.2f },
        { 149.25f, 1,  0.0f },

        // Final Drop 第二小节 (150s - 166s)
        { 150.00f, 0, -0.6f }, { 150.00f, 0,  0.6f },
        { 150.50f, 1, -0.3f }, { 150.75f, 1,  0.3f },
        { 151.50f, 0,  0.0f }, { 152.00f, 0, -0.5f },
        { 152.50f, 0,  0.5f }, { 153.25f, 1,  0.0f },
        { 154.00f, 0, -0.4f }, { 154.00f, 1,  0.4f },
        { 155.00f, 0, -0.2f }, { 155.33f, 0,  0.0f }, { 155.66f, 0,  0.2f },
        { 156.50f, 0, -0.7f }, { 156.50f, 1,  0.7f },
        { 158.00f, 1, -0.4f }, { 158.50f, 1,  0.4f },
        { 159.25f, 0,  0.0f }, { 160.00f, 0, -0.3f },
        { 160.50f, 0,  0.3f }, { 161.25f, 1, -0.5f }, { 161.25f, 1,  0.5f },
        { 162.00f, 0, -0.6f }, { 162.50f, 0,  0.6f },
        { 163.25f, 0,  0.0f }, { 164.00f, 0, -0.4f },
        { 164.50f, 0,  0.4f }, { 165.25f, 0,  0.0f },

        // --- 11. Long Outro (166s - 211s) 3分31秒完整收尾 ---
        { 166.50f, 0, -0.2f }, { 168.00f, 0,  0.2f }, { 170.00f, 0, -0.4f }, { 172.00f, 0,  0.4f },
        { 174.00f, 0, -0.3f }, { 176.00f, 0,  0.3f }, { 178.00f, 0, -0.5f }, { 180.00f, 0,  0.5f },
        { 183.00f, 0, -0.2f }, { 186.00f, 0,  0.2f }, { 189.00f, 0, -0.4f }, { 192.00f, 0,  0.4f },
        { 195.00f, 0, -0.2f }, { 198.00f, 0,  0.2f }, { 202.00f, 0,  0.0f },
        { 206.00f, 0,  0.0f },
        { 210.00f, 0,  0.0f }  // 210s 最后一击定音！
    };

    // =================================================================
    // 💃 3分31秒（211秒）全曲判定线舞步动画
    // =================================================================
    const std::vector<RhythmLine::LineEvent> Line0Events = {
        { 0.0f, 3.0f, BottomCenter, BottomCenter, 0.f, 0.f, 0.f, 255.f },
        { 14.0f, 20.0f, BottomCenter, BottomCenter, 0.f, -8.f, 255.f, 255.f },
        { 20.0f, 28.0f, BottomCenter, BottomCenter, -8.f, 8.f, 255.f, 255.f },
        { 28.0f, 34.0f, BottomCenter, { BottomCenter.x, BottomCenter.y - 70.f }, 8.f, 0.f, 255.f, 255.f },
        // Drop 1
        { 34.0f, 40.0f, { BottomCenter.x, BottomCenter.y - 70.f }, { BottomCenter.x, BottomCenter.y - 40.f }, 0.f, -20.f, 255.f, 255.f },
        { 40.0f, 50.0f, { BottomCenter.x, BottomCenter.y - 40.f }, BottomCenter, -20.f, 0.f, 255.f, 255.f },
        // Bridge 1
        { 50.0f, 66.0f, BottomCenter, BottomCenter, 0.f, 0.f, 255.f, 200.f },
        { 66.0f, 72.0f, BottomCenter, { BottomCenter.x, BottomCenter.y - 80.f }, 0.f, -15.f, 200.f, 255.f },
        // Drop 2
        { 72.0f, 88.0f, { BottomCenter.x, BottomCenter.y - 80.f }, BottomCenter, -15.f, 0.f, 255.f, 255.f },
        // Bridge 2
        { 88.0f, 128.0f, BottomCenter, BottomCenter, 0.f, 0.f, 255.f, 180.f },
        { 128.0f, 134.0f, BottomCenter, { BottomCenter.x, BottomCenter.y - 90.f }, 0.f, -25.f, 180.f, 255.f },
        // 💥 FINAL DROP (134s - 166s) 判定线大幅跳跃旋转
        { 134.0f, 150.0f, { BottomCenter.x, BottomCenter.y - 90.f }, { BottomCenter.x, BottomCenter.y - 50.f }, -25.f, 35.f, 255.f, 255.f },
        { 150.0f, 166.0f, { BottomCenter.x, BottomCenter.y - 50.f }, BottomCenter, 35.f, 0.f, 255.f, 255.f },
        // Long Outro
        { 166.0f, 200.0f, BottomCenter, BottomCenter, 0.f, 0.f, 255.f, 150.f },
        { 200.0f, 211.0f, BottomCenter, BottomCenter, 0.f, 0.f, 150.f, 0.f }
    };

    const std::vector<RhythmLine::LineEvent> Line1Events = {
        { 0.0f, 34.0f, BottomCenter, BottomCenter, 0.f, 0.f, 0.f, 0.f },
        // Drop 1 出现
        { 34.0f, 48.0f, { BottomCenter.x, BottomCenter.y - 100.f }, { BottomCenter.x, BottomCenter.y - 80.f }, 45.f, 90.f, 0.f, 220.f },
        { 48.0f, 50.0f, { BottomCenter.x, BottomCenter.y - 80.f }, BottomCenter, 90.f, 0.f, 220.f, 0.f },
        { 50.0f, 72.0f, BottomCenter, BottomCenter, 0.f, 0.f, 0.f, 0.f },
        // Drop 2 出现
        { 72.0f, 86.0f, { BottomCenter.x + 80.f, BottomCenter.y - 90.f }, BottomCenter, -45.f, 0.f, 0.f, 255.f },
        { 86.0f, 134.0f, BottomCenter, BottomCenter, 0.f, 0.f, 0.f, 0.f },
        // 💥 FINAL DROP 再次震撼入场 (134s - 166s)
        { 134.0f, 164.0f, { BottomCenter.x - 100.f, BottomCenter.y - 100.f }, BottomCenter, 60.f, 0.f, 0.f, 255.f },
        { 164.0f, 211.0f, BottomCenter, BottomCenter, 0.f, 0.f, 255.f, 0.f }
    };
} // namespace
float RhythmLine::easeInOutCubic(float t) const
{
    return t < 0.5f ? 4.f * t * t * t : 1.f - std::pow(-2.f * t + 2.f, 3.f) / 2.f;
}

void RhythmLine::reset()
{
    activeNotes_.clear();
    hitEffects_.clear();
    nextEventIndex_ = 0;
    currentMusicTime_ = 0.f;
    globalSceneGlow_ = 0.f;
    lines_.assign(2, DynamicLine{});
}

void RhythmLine::update(float deltaTime, float musicTimeSeconds)
{
    currentMusicTime_ = std::max(musicTimeSeconds, 0.f);

    if (lines_.size() < 2) lines_.resize(2);

    // 1. 更新判定线编舞动画
    auto updateLineAnim = [this](int lineIdx, const std::vector<LineEvent>& events) {
        for (const auto& ev : events)
        {
            if (currentMusicTime_ >= ev.startTime && currentMusicTime_ <= ev.endTime)
            {
                float duration = ev.endTime - ev.startTime;
                float rawProgress = (currentMusicTime_ - ev.startTime) / duration;
                float progress = easeInOutCubic(rawProgress);

                lines_[lineIdx].currentPos = ev.startPos + (ev.endPos - ev.startPos) * progress;
                lines_[lineIdx].currentAngle = ev.startAngle + (ev.endAngle - ev.startAngle) * progress;
                lines_[lineIdx].currentAlpha = ev.startAlpha + (ev.endAlpha - ev.startAlpha) * progress;
                break;
            }
        }
        };

    updateLineAnim(0, Line0Events);
    updateLineAnim(1, Line1Events);

    globalSceneGlow_ = std::max(0.f, globalSceneGlow_ - deltaTime * 4.f);
    for (auto& line : lines_) {
        line.hitPulse = std::max(0.f, line.hitPulse - deltaTime * 8.f);
    }

    for (auto& fx : hitEffects_) {
        fx.lifetime -= deltaTime;
    }
    hitEffects_.erase(
        std::remove_if(hitEffects_.begin(), hitEffects_.end(), [](const HitEffect& fx) {
            return fx.lifetime <= 0.f;
            }),
        hitEffects_.end());

    // 2. 音符生成逻辑
    constexpr float travelDuration = 1.0f;

    while (nextEventIndex_ < Timeline.size())
    {
        const auto& event = Timeline[nextEventIndex_];
        const float spawnTime = event.hitTime - travelDuration;

        if (currentMusicTime_ >= spawnTime)
        {
            createNote(nextEventIndex_);
            ++nextEventIndex_;
        }
        else
        {
            break;
        }
    }

    activeNotes_.erase(
        std::remove_if(
            activeNotes_.begin(),
            activeNotes_.end(),
            [this](const ActiveNote& note) {
                return note.isHit || (currentMusicTime_ - note.hitTime > 0.2f);
            }),
        activeNotes_.end());
}

void RhythmLine::createNote(std::size_t eventIndex)
{
    const NoteEvent& event = Timeline[eventIndex];

    ActiveNote note;
    note.hitTime = event.hitTime;
    note.travelDuration = 1.0f;
    note.spawnTime = event.hitTime - note.travelDuration;
    note.lineIndex = event.lineIndex;
    note.positionOnLine = event.positionOnLine;

    activeNotes_.push_back(note);
}

HitResult RhythmLine::onPlayerPressSpace()
{
    HitResult bestResult = HitResult::None;
    int targetIndex = -1;
    float minError = 999.f;

    for (std::size_t i = 0; i < activeNotes_.size(); ++i)
    {
        auto& note = activeNotes_[i];
        if (note.isHit) continue;

        float timeError = std::abs(currentMusicTime_ - note.hitTime);
        if (timeError < minError)
        {
            minError = timeError;
            targetIndex = static_cast<int>(i);
        }
    }

    if (targetIndex != -1)
    {
        constexpr float perfectWindow = 0.15f;
        constexpr float greatWindow = 0.30f;

        if (minError <= perfectWindow)
        {
            bestResult = HitResult::Perfect;
            auto& note = activeNotes_[targetIndex];
            note.isHit = true;
            lines_[note.lineIndex].hitPulse = 1.0f;
            globalSceneGlow_ = 1.0f;

            sf::Vector2f lineCenter = lines_[note.lineIndex].currentPos;
            float angleRad = lines_[note.lineIndex].currentAngle * Pi / 180.f;
            float lineOffset = note.positionOnLine * 500.f;

            sf::Vector2f hitPos = lineCenter + sf::Vector2f{
                lineOffset * std::cos(angleRad),
                lineOffset * std::sin(angleRad)
            };

            hitEffects_.push_back({ hitPos, lines_[note.lineIndex].currentAngle, 0.3f, 0.3f });
        }
    }

    return bestResult;
}

void RhythmLine::draw(sf::RenderWindow& window) const
{
    sf::RenderStates addStates;
    addStates.blendMode = sf::BlendAdd;

    const sf::Color lineCyanGlow(180, 240, 255);
    const sf::Color noteCoreWhite(240, 250, 255);

    // -----------------------------------------------------------------
    // 1. 🌌 绘制向上延伸的下落轨道面 (Track Floor)
    // -----------------------------------------------------------------
    for (const auto& line : lines_)
    {
        if (line.currentAlpha <= 5.f) continue;

        float lineAngleRad = line.currentAngle * Pi / 180.f;
        // 🌟 3. 法线方向取负 (-Pi/2)，让下落轨道向【上方】延伸展开
        float normalAngleRad = lineAngleRad - Pi / 2.f;
        float trackLen = 850.f;
        float trackHeight = 500.f; // 向上延伸至屏幕中上方

        sf::Vector2f lineDir{ std::cos(lineAngleRad), std::sin(lineAngleRad) };
        sf::Vector2f normalDir{ std::cos(normalAngleRad), std::sin(normalAngleRad) };

        sf::VertexArray trackFloor(sf::PrimitiveType::TriangleStrip, 4);

        sf::Color baseColor = lineCyanGlow;
        baseColor.a = static_cast<std::uint8_t>(line.currentAlpha * 0.18f + line.hitPulse * 50.f);

        sf::Color fadeColor = lineCyanGlow;
        fadeColor.a = 0; // 向上上方消融

        sf::Vector2f p1 = line.currentPos - lineDir * (trackLen / 2.f);
        sf::Vector2f p2 = line.currentPos + lineDir * (trackLen / 2.f);
        sf::Vector2f p3 = p1 + normalDir * trackHeight;
        sf::Vector2f p4 = p2 + normalDir * trackHeight;

        trackFloor[0].position = p1; trackFloor[0].color = baseColor;
        trackFloor[1].position = p2; trackFloor[1].color = baseColor;
        trackFloor[2].position = p3; trackFloor[2].color = fadeColor;
        trackFloor[3].position = p4; trackFloor[3].color = fadeColor;

        window.draw(trackFloor, addStates);
    }

    // -----------------------------------------------------------------
    // 2. 🎨 绘制位于底部的发光判定线
    // -----------------------------------------------------------------
    for (const auto& line : lines_)
    {
        if (line.currentAlpha <= 5.f) continue;

        float lineLength = 950.f + line.hitPulse * 120.f;

        // 外部晕光
        sf::RectangleShape glowShape({ lineLength, 6.f + line.hitPulse * 4.f });
        glowShape.setOrigin({ lineLength / 2.f, 3.f + line.hitPulse * 2.f });
        glowShape.setPosition(line.currentPos);
        glowShape.setRotation(sf::degrees(line.currentAngle));

        sf::Color glowColor = lineCyanGlow;
        glowColor.a = static_cast<std::uint8_t>(line.currentAlpha * 0.4f);
        glowShape.setFillColor(glowColor);
        window.draw(glowShape, addStates);

        // 核心亮线
        sf::RectangleShape coreShape({ lineLength, 2.0f + line.hitPulse * 1.5f });
        coreShape.setOrigin({ lineLength / 2.f, 1.0f + line.hitPulse * 0.75f });
        coreShape.setPosition(line.currentPos);
        coreShape.setRotation(sf::degrees(line.currentAngle));

        sf::Color coreColor = noteCoreWhite;
        coreColor.a = static_cast<std::uint8_t>(std::clamp(line.currentAlpha, 0.f, 255.f));
        coreShape.setFillColor(coreColor);
        window.draw(coreShape, addStates);
    }

    // -----------------------------------------------------------------
    // 3. 🎨 绘制【从上方往落向底部】的方块音符与拖尾
    // -----------------------------------------------------------------
    for (const ActiveNote& note : activeNotes_)
    {
        if (note.isHit) continue;
        if (note.lineIndex >= lines_.size()) continue;

        const auto& targetLine = lines_[note.lineIndex];
        if (targetLine.currentAlpha <= 10.f) continue;

        float progress = (currentMusicTime_ - note.spawnTime) / note.travelDuration;
        progress = std::clamp(progress, 0.f, 1.2f);

        // 🌟 4. 距离递减：从上方 (450px 高处) 向下滑动到 0px (判定线处)
        float distance = (1.f - progress) * 450.f;
        float lineAngleRad = targetLine.currentAngle * Pi / 180.f;

        // 🌟 5. 取负法线 (-Pi/2)，代表“上方的天空方向”
        float normalAngleRad = lineAngleRad - Pi / 2.f;

        sf::Vector2f lineOffset{
            note.positionOnLine * 500.f * std::cos(lineAngleRad),
            note.positionOnLine * 500.f * std::sin(lineAngleRad)
        };

        sf::Vector2f dropOffset{
            distance * std::cos(normalAngleRad),
            distance * std::sin(normalAngleRad)
        };

        sf::Vector2f worldPos = targetLine.currentPos + lineOffset + dropOffset;

        // 音符向上方的拖尾
        float trailLen = 50.f * (1.f - progress);
        sf::RectangleShape trail({ note.size * 2.0f, trailLen });
        trail.setOrigin({ note.size * 1.0f, 0.f });
        trail.setPosition(worldPos);
        trail.setRotation(sf::degrees(targetLine.currentAngle + 90.f)); // 拖尾朝上

        sf::Color trailColor = lineCyanGlow;
        trailColor.a = static_cast<std::uint8_t>(targetLine.currentAlpha * 0.45f);
        trail.setFillColor(trailColor);
        window.draw(trail, addStates);

        // 方块音符核心
        sf::RectangleShape noteShape({ note.size * 2.2f, note.size * 0.6f });
        noteShape.setOrigin({ note.size * 1.1f, note.size * 0.3f });
        noteShape.setPosition(worldPos);
        noteShape.setRotation(sf::degrees(targetLine.currentAngle));

        sf::Color noteColor = noteCoreWhite;
        noteColor.a = static_cast<std::uint8_t>(targetLine.currentAlpha);
        noteShape.setFillColor(noteColor);

        window.draw(noteShape, addStates);
    }

    // -----------------------------------------------------------------
    // 4. 🎨 击中瞬间的爆炸波纹
    // -----------------------------------------------------------------
    for (const auto& fx : hitEffects_)
    {
        float progress = 1.0f - (fx.lifetime / fx.maxLifetime);
        float width = 40.f + progress * 180.f;
        float height = 20.f * (1.0f - progress);

        sf::RectangleShape wave({ width, height });
        wave.setOrigin({ width / 2.f, height / 2.f });
        wave.setPosition(fx.pos);
        wave.setRotation(sf::degrees(fx.angle));

        sf::Color waveColor = lineCyanGlow;
        waveColor.a = static_cast<std::uint8_t>((1.0f - progress) * 255);
        wave.setFillColor(waveColor);

        window.draw(wave, addStates);
    }
}