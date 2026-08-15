#include "RhythmLine.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
    constexpr float Pi = 3.14159265358979323846f;
    constexpr float ScreenWidth = 1280.f;
    constexpr float ScreenHeight = 720.f;

    // 轨道离屏幕边缘的初始距离。
    // 上下轨使用 100px，左右轨仍使用 80px。
    constexpr float VerticalMargin = 100.f;
    constexpr float HorizontalMargin = 80.f;

    struct NoteEvent {
        float hitTime;
        int lineIndex;        // 0: 下, 1: 上, 2: 左, 3: 右
        float positionOnLine; // -0.8 ~ 0.8 (线上的相对偏移位置)
    };

    // 四轨统一色相，避免四色抢夺场景主体。
    // 中国风使用宣纸月白，避开金黄色 UI 与红色太阳。
    const sf::Color ChineseLineColors[4] = {
        sf::Color(232, 226, 211),
        sf::Color(232, 226, 211),
        sf::Color(232, 226, 211),
        sf::Color(232, 226, 211)
    };

    // 赛博风恢复冷白主线；可读性由黑色衬底线保证，而不是依赖艳色。
    const sf::Color CyberLineColors[4] = {
        sf::Color(232, 236, 240),
        sf::Color(232, 236, 240),
        sf::Color(232, 236, 240),
        sf::Color(232, 236, 240)
    };

    // 🎵 Phigros 风格四向谱面 Timeline (3分31秒全长音频精准对齐)
    const std::vector<NoteEvent> Timeline = {
        // 1. 前奏引入 (0.0s - 35.0s)
        { 3.00f, 0,  0.0f },  { 6.00f, 1,  0.0f },  { 9.00f, 2, -0.2f }, { 12.00f, 3,  0.2f },
        { 15.00f, 0, -0.3f }, { 15.00f, 0,  0.3f }, { 18.00f, 1,  0.0f },  { 21.00f, 2,  0.3f },
        { 24.00f, 3, -0.3f }, { 27.00f, 0, -0.4f }, { 27.00f, 1,  0.4f }, { 30.00f, 2,  0.0f },
        { 32.00f, 3,  0.0f }, { 34.00f, 0,  0.0f },

        // 2. 主歌律动 (35.0s - 75.0s)
        { 36.00f, 0, -0.2f }, { 37.00f, 0,  0.2f }, { 38.50f, 1, -0.3f }, { 39.50f, 1,  0.3f },
        { 41.00f, 2,  0.0f }, { 42.00f, 3,  0.0f }, { 43.50f, 0, -0.4f }, { 44.00f, 0,  0.0f },
        { 44.50f, 0,  0.4f }, { 46.00f, 1,  0.4f }, { 46.50f, 1,  0.0f }, { 47.00f, 1, -0.4f },
        { 49.00f, 2, -0.3f }, { 49.50f, 2,  0.3f }, { 51.00f, 3, -0.3f }, { 51.50f, 3,  0.3f },
        { 53.00f, 0, -0.2f }, { 54.00f, 1,  0.2f }, { 55.00f, 2, -0.2f }, { 56.00f, 3,  0.2f },
        { 58.00f, 0, -0.3f }, { 58.40f, 2,  0.0f }, { 58.80f, 1,  0.3f }, { 59.20f, 3,  0.0f },
        { 60.50f, 0,  0.0f }, { 61.00f, 0, -0.4f }, { 61.00f, 0,  0.4f }, { 63.00f, 1,  0.0f },
        { 63.50f, 1, -0.4f }, { 63.50f, 1,  0.4f }, { 65.00f, 2, -0.3f }, { 65.50f, 3,  0.3f },
        { 66.00f, 2,  0.3f }, { 66.50f, 3, -0.3f }, { 68.00f, 0, -0.5f }, { 68.30f, 0, -0.2f },
        { 68.60f, 0,  0.2f }, { 68.90f, 0,  0.5f }, { 71.00f, 1, -0.5f }, { 71.30f, 1, -0.2f },
        { 71.60f, 1,  0.2f }, { 71.90f, 1,  0.5f }, { 73.50f, 0,  0.0f }, { 74.00f, 1,  0.0f },
        { 74.50f, 2,  0.0f }, { 74.75f, 3,  0.0f },

        // 3. 第一波 Drop 高潮 (75.0s - 115.0s)
        { 75.50f, 0,  0.0f }, { 75.85f, 1,  0.0f }, { 76.20f, 2, -0.2f }, { 76.55f, 3,  0.2f },
        { 77.50f, 0, -0.4f }, { 77.50f, 0,  0.4f }, { 78.50f, 1, -0.4f }, { 78.50f, 1,  0.4f },
        { 80.00f, 0, -0.3f }, { 80.25f, 2,  0.0f }, { 80.50f, 1,  0.3f }, { 80.75f, 3,  0.0f },
        { 81.50f, 0,  0.0f }, { 81.85f, 0, -0.4f }, { 81.85f, 0,  0.4f }, { 83.00f, 2, -0.4f },
        { 83.30f, 2,  0.0f }, { 83.60f, 2,  0.4f }, { 84.50f, 3,  0.4f }, { 84.80f, 3,  0.0f },
        { 85.10f, 3, -0.4f }, { 86.50f, 0, -0.2f }, { 86.80f, 1,  0.2f }, { 87.10f, 0,  0.4f },
        { 87.40f, 1, -0.4f }, { 89.00f, 0,  0.0f }, { 89.00f, 1,  0.0f }, { 89.00f, 2,  0.0f },
        { 89.00f, 3,  0.0f }, { 90.50f, 0, -0.3f }, { 90.85f, 0,  0.3f }, { 92.00f, 1, -0.3f },
        { 92.35f, 1,  0.3f }, { 93.50f, 2,  0.0f }, { 94.00f, 3,  0.0f }, { 94.50f, 0,  0.0f },
        { 95.00f, 1,  0.0f }, { 97.00f, 0, -0.4f }, { 97.25f, 0,  0.0f }, { 97.50f, 0,  0.4f },
        { 98.50f, 1, -0.4f }, { 98.75f, 1,  0.0f }, { 99.00f, 1,  0.4f }, { 100.50f, 2, -0.3f },
        { 101.00f, 3,  0.3f }, { 101.50f, 2,  0.3f }, { 102.00f, 3, -0.3f }, { 103.50f, 0, -0.2f },
        { 104.00f, 1,  0.2f }, { 104.50f, 0, -0.4f }, { 104.50f, 0,  0.4f }, { 106.00f, 2,  0.0f },
        { 107.00f, 3,  0.0f }, { 108.00f, 0,  0.0f }, { 109.00f, 1,  0.0f }, { 111.00f, 0, -0.3f },
        { 112.00f, 1,  0.3f }, { 113.00f, 2,  0.0f }, { 114.00f, 3,  0.0f },

        // 4. 中场间奏 (115.0s - 150.0s)
        { 116.00f, 0,  0.0f }, { 119.00f, 1,  0.0f }, { 122.00f, 2, -0.3f }, { 123.50f, 2,  0.3f },
        { 125.00f, 3, -0.3f }, { 126.50f, 3,  0.3f }, { 128.50f, 0, -0.2f }, { 130.00f, 1,  0.2f },
        { 132.00f, 0,  0.0f }, { 133.50f, 0,  0.0f }, { 136.00f, 2, -0.4f }, { 137.00f, 3,  0.4f },
        { 138.50f, 0, -0.3f }, { 139.50f, 1,  0.3f }, { 141.00f, 0,  0.0f }, { 142.00f, 0, -0.3f },
        { 142.00f, 0,  0.3f }, { 144.00f, 1,  0.0f }, { 145.00f, 1, -0.3f }, { 145.00f, 1,  0.3f },
        { 147.00f, 2,  0.0f }, { 148.00f, 3,  0.0f }, { 149.00f, 0,  0.0f }, { 149.50f, 1,  0.0f },

        // 5. 终极高潮 Drop 2 (150.0s - 190.0s)
        { 150.50f, 0, -0.4f }, { 150.50f, 1,  0.4f }, { 151.00f, 2, -0.4f }, { 151.00f, 3,  0.4f },
        { 152.00f, 0, -0.5f }, { 152.20f, 0, -0.2f }, { 152.40f, 0,  0.1f }, { 152.60f, 0,  0.4f },
        { 153.20f, 1,  0.5f }, { 153.40f, 1,  0.2f }, { 153.60f, 1, -0.1f }, { 153.80f, 1, -0.4f },
        { 155.00f, 0, -0.3f }, { 155.25f, 2, -0.3f }, { 155.50f, 1,  0.3f }, { 155.75f, 3,  0.3f },
        { 156.50f, 0,  0.3f }, { 156.75f, 2,  0.3f }, { 157.00f, 1, -0.3f }, { 157.25f, 3, -0.3f },
        { 158.50f, 0, -0.3f }, { 158.50f, 1,  0.3f }, { 159.00f, 2, -0.3f }, { 159.00f, 3,  0.3f },
        { 159.50f, 0,  0.0f }, { 159.50f, 1,  0.0f }, { 159.50f, 2,  0.0f }, { 159.50f, 3,  0.0f },
        { 161.00f, 0, -0.5f }, { 161.25f, 0, -0.2f }, { 161.50f, 0,  0.2f }, { 161.75f, 0,  0.5f },
        { 162.50f, 1, -0.5f }, { 162.75f, 1, -0.2f }, { 163.00f, 1,  0.2f }, { 163.25f, 1,  0.5f },
        { 165.00f, 2, -0.4f }, { 165.25f, 2,  0.0f }, { 165.50f, 2,  0.4f }, { 166.50f, 3, -0.4f },
        { 166.75f, 3,  0.0f }, { 167.00f, 3,  0.4f }, { 168.50f, 0, -0.4f }, { 168.85f, 0,  0.4f },
        { 170.00f, 1, -0.4f }, { 170.35f, 1,  0.4f }, { 171.50f, 2,  0.0f }, { 172.00f, 3,  0.0f },
        { 172.50f, 0,  0.0f }, { 173.00f, 1,  0.0f }, { 175.00f, 0, -0.3f }, { 175.20f, 2,  0.0f },
        { 175.40f, 1,  0.3f }, { 175.60f, 3,  0.0f }, { 176.50f, 0,  0.3f }, { 176.70f, 2,  0.0f },
        { 176.90f, 1, -0.3f }, { 177.10f, 3,  0.0f }, { 179.00f, 0, -0.4f }, { 179.00f, 1, -0.4f },
        { 179.50f, 0,  0.4f }, { 179.50f, 1,  0.4f }, { 180.50f, 2,  0.0f }, { 181.00f, 3,  0.0f },
        { 182.50f, 0, -0.3f }, { 182.85f, 0,  0.3f }, { 183.20f, 1, -0.3f }, { 183.55f, 1,  0.3f },
        { 185.00f, 2, -0.4f }, { 185.30f, 2,  0.0f }, { 185.60f, 2,  0.4f }, { 187.00f, 3, -0.4f },
        { 187.30f, 3,  0.0f }, { 187.60f, 3,  0.4f }, { 189.00f, 0,  0.0f }, { 189.50f, 1,  0.0f },

        // 6. 尾声余音 (190.0s - 211.0s)
        { 191.50f, 0,  0.0f }, { 194.00f, 1,  0.0f }, { 196.50f, 2,  0.0f }, { 199.00f, 3,  0.0f },
        { 202.00f, 0, -0.3f }, { 202.00f, 0,  0.3f }, { 205.00f, 1,  0.0f }, { 208.50f, 0,  0.0f }
    };

    // 🎛️ 计算 180° 对调时的轨迹镜头平移向量
    sf::Vector2f getSingleLineShift(size_t lineIdx, float cameraAngle) {
        float rad = cameraAngle * Pi / 180.f;
        float progress = (1.0f - std::cos(rad)) * 0.5f; // 0° -> 0.0, 180° -> 1.0

        // 位移距离必须与构造函数中的初始位置使用同一组边距。
        // 上下轨：620 - 100 = 520px；左右轨：1200 - 80 = 1120px。
        const float verticalDist = ScreenHeight - 2.f * VerticalMargin;
        const float horizontalDist = ScreenWidth - 2.f * HorizontalMargin;

        switch (lineIdx) {
        case 0: return { 0.f, -verticalDist * progress };   // 下轨 -> 上移
        case 1: return { 0.f, verticalDist * progress };    // 上轨 -> 下移
        case 2: return { horizontalDist * progress, 0.f };  // 左轨 -> 右移
        case 3: return { -horizontalDist * progress, 0.f }; // 右轨 -> 左移
        default: return { 0.f, 0.f };
        }
    }

    // 🛠️ 核心绘图辅助函数：绘制指定线宽的多边形轮廓（彻底解决 SFML 线条无粗细问题）
    void drawPolygonThick(sf::RenderWindow& window, sf::Vector2f center, float radius, int sides, float rotationDegrees, float thickness, sf::Color color, const sf::RenderStates& states) {
        float angleStep = 2.0f * Pi / sides;
        float rotRad = rotationDegrees * Pi / 180.f;

        for (int i = 0; i < sides; ++i) {
            float a1 = i * angleStep + rotRad;
            float a2 = (i + 1) * angleStep + rotRad;

            sf::Vector2f p1(center.x + std::cos(a1) * radius, center.y + std::sin(a1) * radius);
            sf::Vector2f p2(center.x + std::cos(a2) * radius, center.y + std::sin(a2) * radius);

            sf::Vector2f dir = p2 - p1;
            float len = std::hypot(dir.x, dir.y);
            if (len <= 0.001f) continue;

            sf::RectangleShape seg({ len, thickness });
            seg.setOrigin({ 0.f, thickness / 2.f });
            seg.setPosition(p1);
            seg.setRotation(sf::degrees(std::atan2(dir.y, dir.x) * 180.f / Pi));
            seg.setFillColor(color);

            window.draw(seg, states);
        }
    }
}

RhythmLine::RhythmLine()
{
    lines_.resize(4);

    // 0. 下线（Bottom）：y = 720 - 100 = 620
    lines_[0].startPos = { -100.f, ScreenHeight - VerticalMargin };
    lines_[0].endPos = { ScreenWidth + 100.f, ScreenHeight - VerticalMargin };
    lines_[0].baseAngle = 0.f;
    lines_[0].normalDir = { 0.f, -1.f };

    // 1. 上线（Top）：y = 100
    lines_[1].startPos = { ScreenWidth + 100.f, VerticalMargin };
    lines_[1].endPos = { -100.f, VerticalMargin };
    lines_[1].baseAngle = 180.f;
    lines_[1].normalDir = { 0.f, 1.f };

    // 2. 左线（Left）
    lines_[2].startPos = { HorizontalMargin, -100.f };
    lines_[2].endPos = { HorizontalMargin, ScreenHeight + 100.f };
    lines_[2].baseAngle = 90.f;
    lines_[2].normalDir = { 1.f, 0.f };

    // 3. 右线（Right）
    lines_[3].startPos = { ScreenWidth - HorizontalMargin, ScreenHeight + 100.f };
    lines_[3].endPos = { ScreenWidth - HorizontalMargin, -100.f };
    lines_[3].baseAngle = 270.f;
    lines_[3].normalDir = { -1.f, 0.f };

    reset();
}

void RhythmLine::reset()
{
    activeNotes_.clear();
    hitEffects_.clear();
    nextEventIndex_ = 0;
    currentMusicTime_ = 0.f;
    globalSceneGlow_ = 0.f;
    cameraAngle_ = 0.f;
    targetCameraAngle_ = 0.f;
    lineExtendProgress_ = 0.f;
    updateLineLayout();
}

void RhythmLine::setCameraAngle(float angleDegrees)
{
    targetCameraAngle_ = angleDegrees;
}

void RhythmLine::setCyberStyle(bool enabled)
{
    cyberStyle_ = enabled;
}

void RhythmLine::updateLineLayout()
{
    for (size_t i = 0; i < lines_.size(); ++i) {
        lines_[i].currentAngle = lines_[i].baseAngle;
        lines_[i].currentAlpha = 220.f;
    }
}

void RhythmLine::update(float deltaTime, float musicTimeSeconds)
{
    syncMusicTime(musicTimeSeconds);

    // 展开开场动画
    if (lineExtendProgress_ < 1.0f) {
        lineExtendProgress_ = std::min(1.0f, lineExtendProgress_ + deltaTime * 1.2f);
    }

    // 平滑镜头旋转
    float maxStep = lineRotationSpeed_ * deltaTime;
    if (cameraAngle_ < targetCameraAngle_) {
        cameraAngle_ = std::min(cameraAngle_ + maxStep, targetCameraAngle_);
        updateLineLayout();
    }
    else if (cameraAngle_ > targetCameraAngle_) {
        cameraAngle_ = std::max(cameraAngle_ - maxStep, targetCameraAngle_);
        updateLineLayout();
    }

    // 衰减全局光晕与击中脉冲
    globalSceneGlow_ = std::max(0.f, globalSceneGlow_ - deltaTime * 4.f);
    for (auto& line : lines_) {
        line.hitPulse = std::max(0.f, line.hitPulse - deltaTime * 8.f);
    }

    // 更新打击特效生命周期
    for (auto& fx : hitEffects_) {
        fx.lifetime -= deltaTime;
    }
    hitEffects_.erase(
        std::remove_if(hitEffects_.begin(), hitEffects_.end(), [](const HitEffect& fx) {
            return fx.lifetime <= 0.f;
            }),
        hitEffects_.end());

    // Timeline 生成机制 (提前 1.0 秒缩放入场)
    constexpr float travelDuration = 1.0f;
    while (nextEventIndex_ < Timeline.size()) {
        const auto& event = Timeline[nextEventIndex_];
        const float spawnTime = event.hitTime - travelDuration;

        if (currentMusicTime_ >= spawnTime) {
            createNote(nextEventIndex_);
            ++nextEventIndex_;
        }
        else {
            break;
        }
    }

    // 清理已被击中或过界 Miss 的 Note
    activeNotes_.erase(
        std::remove_if(activeNotes_.begin(), activeNotes_.end(), [this](const ActiveNote& note) {
            return note.isHit || (currentMusicTime_ - note.hitTime > 0.25f);
            }),
        activeNotes_.end());
}

void RhythmLine::syncMusicTime(float musicTimeSeconds)
{
    currentMusicTime_ = std::max(musicTimeSeconds, 0.f);
}

void RhythmLine::createNote(std::size_t eventIndex)
{
    const NoteEvent& event = Timeline[eventIndex];
    ActiveNote note;
    note.hitTime = event.hitTime;
    note.travelDuration = 1.0f;
    note.spawnTime = event.hitTime - note.travelDuration;
    note.lineIndex = std::clamp(event.lineIndex, 0, 3);
    note.positionOnLine = event.positionOnLine;
    activeNotes_.push_back(note);
}

HitResult RhythmLine::onPlayerPressSpace()
{
    HitResult bestResult = HitResult::None;
    int targetIndex = -1;
    float minError = 999.f;

    for (std::size_t i = 0; i < activeNotes_.size(); ++i) {
        auto& note = activeNotes_[i];
        if (note.isHit) continue;

        float timeError = std::abs(currentMusicTime_ - note.hitTime);
        if (timeError < minError) {
            minError = timeError;
            targetIndex = static_cast<int>(i);
        }
    }

    if (targetIndex != -1) {
        constexpr float perfectWindow = 0.35f; // Perfect 判定时间窗口 (±200ms)
        if (minError <= perfectWindow) {
            bestResult = HitResult::Perfect;
            auto& note = activeNotes_[targetIndex];
            note.isHit = true;
            lines_[note.lineIndex].hitPulse = 1.2f;
            globalSceneGlow_ = 1.0f;

            // 计算该 Note 击中时的真实坐标
            sf::Vector2f lineShift = getSingleLineShift(note.lineIndex, cameraAngle_);
            sf::Vector2f lineCenter = (lines_[note.lineIndex].startPos + lines_[note.lineIndex].endPos) / 2.f + lineShift;

            float lineRad = lines_[note.lineIndex].baseAngle * Pi / 180.f;
            sf::Vector2f lineTangent{ std::cos(lineRad), std::sin(lineRad) };
            sf::Vector2f notePos = lineCenter + lineTangent * (note.positionOnLine * 300.f);

            hitEffects_.push_back({ notePos, lines_[note.lineIndex].baseAngle, 0.35f, 0.35f });
        }
    }
    return bestResult;
}

HitResult RhythmLine::onPlayerPressKey(LineDirection targetDir)
{
    return onPlayerPressLine(static_cast<int>(targetDir));
}


HitResult RhythmLine::onPlayerPressDirection(int screenDirection, float cameraAngle)
{
    if (screenDirection < 0 || screenDirection > 3) {
        return HitResult::None;
    }

    // 游戏只使用 0° <-> 180° 翻转。旧逻辑会在 45° 时错误进入
    // 90° 映射，使上下按键突然对应左右轨道。
    float normalizedAngle = std::fmod(cameraAngle, 360.f);
    if (normalizedAngle < 0.f) normalizedAngle += 360.f;

    int actualLineIndex = screenDirection;
    // 轨道越过屏幕中线后才交换输入；其余时候保持原方向。
    if (normalizedAngle >= 90.f && normalizedAngle < 270.f) {
        constexpr int map180[] = { 1, 0, 3, 2 };
        actualLineIndex = map180[screenDirection];
    }

    // 保证击中特效坐标与镜头位置一致。
    cameraAngle_ = cameraAngle;
    return onPlayerPressLine(actualLineIndex);
}


HitResult RhythmLine::onPlayerPressLine(int lineIdx)
{
    if (lineIdx < 0 || lineIdx >= static_cast<int>(lines_.size())) {
        return HitResult::None;
    }

    // 原型阶段使用较宽的判定窗，优先保证输入手感和可测试性。
    constexpr float perfectWindow = 0.35f; // +/- 120 ms
    constexpr float greatWindow = 0.5f;   // +/- 250 ms

    int targetIndex = -1;
    float targetError = 0.f;

    // activeNotes_ 按时间线顺序生成。优先处理该轨最早的可判定音符，
    // 防止连续音符的时间窗口重叠时，后一个音符抢走前一个音符的按键。
    for (std::size_t i = 0; i < activeNotes_.size(); ++i) {
        const auto& note = activeNotes_[i];
        if (note.isHit || note.lineIndex != lineIdx) continue;

        const float signedError = currentMusicTime_ - note.hitTime;

        // 已经过期的音符不应阻塞后面的音符。
        if (signedError > greatWindow) continue;

        // 最早音符尚未进入判定窗，后续音符只会更早，不再继续搜索。
        if (signedError < -greatWindow) break;

        targetError = std::abs(signedError);
        if (targetError <= greatWindow) {
            targetIndex = static_cast<int>(i);
            break;
        }
    }

    if (targetIndex == -1) {
        return HitResult::None;
    }

    auto& note = activeNotes_[targetIndex];
    note.isHit = true;
    lines_[note.lineIndex].hitPulse = 1.2f;
    globalSceneGlow_ = 1.0f;

    const sf::Vector2f lineShift = getSingleLineShift(note.lineIndex, cameraAngle_);
    const sf::Vector2f lineCenter =
        (lines_[note.lineIndex].startPos + lines_[note.lineIndex].endPos) / 2.f + lineShift;

    const float lineRad = lines_[note.lineIndex].baseAngle * Pi / 180.f;
    const sf::Vector2f lineTangent{ std::cos(lineRad), std::sin(lineRad) };
    const sf::Vector2f notePos =
        lineCenter + lineTangent * (note.positionOnLine * 300.f);

    hitEffects_.push_back(
        { notePos, lines_[note.lineIndex].baseAngle, 0.35f, 0.35f });

    return targetError <= perfectWindow ? HitResult::Perfect : HitResult::Great;
}


void RhythmLine::draw(sf::RenderWindow& window) const
{
    window.setView(window.getDefaultView());

    sf::RenderStates addStates;
    addStates.blendMode = sf::BlendAdd;

    const sf::Color* lineColors =
        cyberStyle_ ? CyberLineColors : ChineseLineColors;

    // 两种风格都采用极细半透明轨道；游戏感由扫光和击中脉冲提供。
    const float baseCoreThickness = cyberStyle_ ? 1.1f : 1.2f;
    const float baseGlowThickness = 3.5f;
    const float glowAlphaScale = 0.12f;
    const float idleAlpha = cyberStyle_ ? 145.f : 125.f;

    // -----------------------------------------------------------------
    // 🌐 1. 绘制 4 条核心判定线 (带外发光 Glow)
    // -----------------------------------------------------------------
    for (size_t i = 0; i < lines_.size(); ++i) {
        const auto& line = lines_[i];
        if (line.currentAlpha <= 5.f || lineExtendProgress_ <= 0.01f) continue;

        sf::Vector2f lineShift = getSingleLineShift(i, cameraAngle_);
        sf::Vector2f currentStart = line.startPos + lineShift;
        sf::Vector2f currentEnd = line.endPos + lineShift;

        sf::Vector2f animatedEnd = currentStart + (currentEnd - currentStart) * lineExtendProgress_;
        sf::Vector2f dir = animatedEnd - currentStart;
        float currentLen = std::hypot(dir.x, dir.y);
        sf::Vector2f lineCenter = (currentStart + animatedEnd) / 2.f;

        // 核心实线
        const float coreThickness =
            baseCoreThickness + line.hitPulse * (cyberStyle_ ? 2.2f : 1.4f);
        sf::RectangleShape coreLine({ currentLen, coreThickness });
        coreLine.setOrigin({ currentLen / 2.f, coreThickness / 2.f });
        coreLine.setPosition(lineCenter);
        coreLine.setRotation(sf::degrees(line.baseAngle));

        sf::Color color = lineColors[i];
        color.a = static_cast<std::uint8_t>(std::clamp(
            idleAlpha + line.hitPulse * 70.f, 0.f, 255.f));
        coreLine.setFillColor(color);

        // 白线下方先绘制极窄黑色衬底，经过白色建筑轮廓时仍保持清晰。
        if (cyberStyle_) {
            const float backingThickness = 3.0f + line.hitPulse * 1.2f;
            sf::RectangleShape backingLine({ currentLen, backingThickness });
            backingLine.setOrigin({ currentLen / 2.f, backingThickness / 2.f });
            backingLine.setPosition(lineCenter);
            backingLine.setRotation(sf::degrees(line.baseAngle));
            backingLine.setFillColor(sf::Color(0, 0, 0,
                static_cast<std::uint8_t>(145.f + line.hitPulse * 45.f)));
            window.draw(backingLine);
        }
        window.draw(coreLine, addStates);

        // 外围发光层
        if (cyberStyle_) {
            // 赛博场景不再叠加彩色残影或宽外辉。
        }
        else {
            const float glowThickness =
                baseGlowThickness + line.hitPulse * 3.f;
            sf::RectangleShape glowLine({ currentLen, glowThickness });
            glowLine.setOrigin({ currentLen / 2.f, glowThickness / 2.f });
            glowLine.setPosition(lineCenter);
            glowLine.setRotation(sf::degrees(line.baseAngle));

            sf::Color glowColor = color;
            glowColor.a = static_cast<std::uint8_t>(color.a * glowAlphaScale);
            glowLine.setFillColor(glowColor);
            window.draw(glowLine, addStates);
        }

    }

    // -----------------------------------------------------------------
    // 🎨 2. 绘制线上神秘徽章音符 (Mystic Badge Note - 双向旋转收缩八边形)
    // -----------------------------------------------------------------
    if (lineExtendProgress_ > 0.3f) {
        const sf::Color baseGoldColor = sf::Color(255, 220, 100);

        for (const ActiveNote& note : activeNotes_) {
            if (note.isHit || note.lineIndex >= lines_.size()) continue;

            const auto& targetLine = lines_[note.lineIndex];

            // 进度: 0.0 (出现) -> 1.0 (判定点)
            float progress = std::clamp((currentMusicTime_ - note.spawnTime) / note.travelDuration, 0.f, 1.2f);
            float alphaProgress = std::min(progress / 0.15f, 1.0f); // 淡入效果

            sf::Vector2f lineShift = getSingleLineShift(note.lineIndex, cameraAngle_);
            sf::Vector2f lineCenter = (targetLine.startPos + targetLine.endPos) / 2.f + lineShift;
            float lineRad = targetLine.baseAngle * Pi / 180.f;
            sf::Vector2f lineTangent{ std::cos(lineRad), std::sin(lineRad) };
            sf::Vector2f noteCenterPos = lineCenter + lineTangent * (note.positionOnLine * 300.f);

            sf::Color noteColor = lineColors[note.lineIndex];
            noteColor.a = static_cast<std::uint8_t>(255 * alphaProgress * (1.0f - progress * 0.15f));

            // 🌟 A. 中心 45° 菱形核心（静态锚点）
            sf::RectangleShape coreSquare({ 16.f, 16.f });
            coreSquare.setOrigin({ 8.f, 8.f });
            coreSquare.setPosition(noteCenterPos);
            coreSquare.setRotation(sf::degrees(45.f));
            coreSquare.setFillColor(noteColor);
            window.draw(coreSquare, addStates);

            // 🌟 B. 缩放与双向旋转八边形环 (Outer & Inner Ring)
            float outerRadius = (1.0f - progress) * 62.f + 18.f;
            float innerRadius = (1.0f - progress) * 46.f + 14.f;
            float rotAngle = progress * 120.f;

            sf::Color glowColor = cyberStyle_ ? noteColor : baseGoldColor;
            glowColor.a = static_cast<std::uint8_t>(
                noteColor.a * (cyberStyle_ ? 0.55f : 0.25f));

            // 外八边形发光与核心轮廓
            drawPolygonThick(window, noteCenterPos, outerRadius * 1.05f, 8, rotAngle,
                cyberStyle_ ? 5.0f : 3.0f, glowColor, addStates);
            drawPolygonThick(window, noteCenterPos, outerRadius, 8, rotAngle,
                cyberStyle_ ? 2.5f : 1.5f, noteColor, addStates);

            // 内八边形反向旋转轮廓
            drawPolygonThick(window, noteCenterPos, innerRadius * 1.05f, 8, -rotAngle * 0.7f,
                cyberStyle_ ? 4.0f : 2.5f, glowColor, addStates);
            drawPolygonThick(window, noteCenterPos, innerRadius, 8, -rotAngle * 0.7f,
                cyberStyle_ ? 2.0f : 1.2f, noteColor, addStates);
        }
    }

    // -----------------------------------------------------------------
    // 🎨 3. 绘制击中消除特效 (Hit Ripple Burst)
    // -----------------------------------------------------------------
    for (const auto& fx : hitEffects_) {
        float progress = 1.0f - (fx.lifetime / fx.maxLifetime);

        // 矩形发光波纹
        float width = 60.f + progress * 200.f;
        float height = 12.f * (1.0f - progress);

        sf::RectangleShape wave({ width, height });
        wave.setOrigin({ width / 2.f, height / 2.f });
        wave.setPosition(fx.pos);
        wave.setRotation(sf::degrees(fx.angle));
        wave.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>((1.0f - progress) * 255)));
        window.draw(wave, addStates);

        // 扩散爆炸多边形线框
        float burstRadius = 20.f + progress * 80.f;
        sf::Color burstColor = cyberStyle_
            ? sf::Color(232, 236, 240, static_cast<std::uint8_t>((1.0f - progress) * 205))
            : sf::Color(242, 235, 218, static_cast<std::uint8_t>((1.0f - progress) * 185));
        drawPolygonThick(window, fx.pos, burstRadius, 8, progress * 45.f, 2.0f, burstColor, addStates);
    }
}