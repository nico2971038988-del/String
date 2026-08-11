#pragma once

#include <SFML/Graphics.hpp>

#include "MusicRhythmEventBus.hpp"

class CameraController
{
public:
    CameraController(
        sf::Vector2f initialCenter,
        sf::Vector2f viewSize
    );

    void update(
        float deltaTime,
        float musicTime,
        sf::Vector2f playerPosition
    );

    // 由 MusicRhythmEventBus 的订阅回调调用。
    void onMusicEvent(const MusicRhythmEvent& event);

    const sf::View& getView() const;

private:
    sf::View camera;

    // 镜头跟随玩家时允许到达的最小横坐标。
    float minimumCameraX = 640.f;

    // 当前显示角度与最近音乐事件要求到达的角度（单位：度）。
    float currentAngle = 0.f;
    float targetAngle = 0.f;

    // 基础旋转速度；事件强度越高，实际速度越快。
    float rotationSpeed = 600.f;
    float activeRotationSpeed = 150.f;
};