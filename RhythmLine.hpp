#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <vector>
#include <cmath>

enum class HitResult {
    None,
    Perfect,
    Great,
    Miss
};

// 四向轨道定义 (0: 下, 1: 上, 2: 左, 3: 右)
enum class LineDirection {
    Bottom = 0,
    Top = 1,
    Left = 2,
    Right = 3
};

class RhythmLine
{
public:
    RhythmLine();

    void update(float deltaTime, float musicTimeSeconds);
    void syncMusicTime(float musicTimeSeconds);
    void setCameraAngle(float angleDegrees);
    void setCyberStyle(bool enabled);
    void draw(sf::RenderWindow& window) const;
    void reset();

    // 支持针对特定轨道或通用按键触发击打
    HitResult onPlayerPressKey(LineDirection targetDir);
    HitResult onPlayerPressSpace(); // 兼容通配击打（判定距离最近的音符）
    HitResult onPlayerPressLine(int targetLineIndex);
    HitResult onPlayerPressDirection(int screenDirection, float cameraAngle);

    // 读取并清空未按键导致的超时 Miss 数量。
    int consumeMissCount();

private:
    struct ActiveNote
    {
        float hitTime = 0.f;
        float travelDuration = 1.0f;
        float spawnTime = 0.f;
        float size = 20.f;
        int lineIndex = 0;        // 0: 下, 1: 上, 2: 左, 3: 右
        float positionOnLine = 0.f; // -0.8 ~ 0.8 沿线偏移
        bool isHit = false;
    };

    struct HitEffect {
        sf::Vector2f pos;
        float angle;
        float lifetime = 0.3f;
        float maxLifetime = 0.3f;
    };

    // 4 条固定/动态判定线数据
    struct PhigrosLine {
        sf::Vector2f startPos;    // 生长起点（屏幕边缘）
        sf::Vector2f endPos;      // 生长终点（对侧边缘）
        sf::Vector2f currentPos;  // 当前中心点
        float baseAngle = 0.f;
        float currentAngle = 0.f;
        float currentAlpha = 220.f;
        float hitPulse = 0.f;
        sf::Vector2f normalDir;
    };

    void createNote(std::size_t eventIndex);
    void updateLineLayout();

    std::vector<ActiveNote> activeNotes_;
    std::vector<HitEffect> hitEffects_;
    std::vector<PhigrosLine> lines_;

    std::size_t nextEventIndex_ = 0;
    float currentMusicTime_ = 0.f;
    float cameraAngle_ = 0.f;
    float targetCameraAngle_ = 0.f;
    float lineRotationSpeed_ = 180.f; // 加快转速以适应 Phigros 的流畅感
    float globalSceneGlow_ = 0.f;
    bool cyberStyle_ = false;
    int pendingMissCount_ = 0;

    float lineExtendProgress_ = 0.f;
    // 在 RhythmLine.hpp 的 private 区域添加可调参数：
    float flipShiftX_ = 120.f; // 镜头旋转时的 水平(X轴) 最大平移距离 (像素)
    float flipShiftY_ = 60.f;  // 镜头旋转时的 垂直(Y轴) 最大平移距离 (像素)
};