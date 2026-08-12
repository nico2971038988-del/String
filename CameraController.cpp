#include "CameraController.hpp"

#include <algorithm>

CameraController::CameraController(
    const sf::Vector2f initialCenter,
    const sf::Vector2f viewSize)
    : camera(initialCenter, viewSize)
{
}

void CameraController::update(
    const float deltaTime,
    const float musicTime,
    const sf::Vector2f playerPosition)
{
    (void)musicTime;

    const float cameraX =
        std::max(minimumCameraX, playerPosition.x);
    camera.setCenter({ cameraX, 360.f });

    const float maximumStep = activeRotationSpeed * deltaTime;

    if (currentAngle < targetAngle)
    {
        currentAngle =
            std::min(currentAngle + maximumStep, targetAngle);
    }
    else if (currentAngle > targetAngle)
    {
        currentAngle =
            std::max(currentAngle - maximumStep, targetAngle);
    }

    // 结束旋转后仍保留目标角度，不重置为 0。
    camera.setRotation(sf::degrees(currentAngle));
}

void CameraController::onMusicEvent(const MusicRhythmEvent& event)
{
    constexpr float cameraAccentThreshold = 1.f;

    if (event.intensity < cameraAccentThreshold)
    {
        return;
    }

    if (event.direction == 0)
    {
        activeRotationSpeed = rotationSpeed * 0.5f;
        return;
    }

    // 每个时间轴事件都在当前目标角度上继续旋转 90 度。
    // +1 顺时针累加，-1 逆时针累加。
    // 不能把目标固定为 0/180，否则连续同方向事件不会再旋转。
    constexpr float angleStep = 180.f;
    targetAngle += static_cast<float>(event.direction) * angleStep;
    activeRotationSpeed = rotationSpeed;
}

const sf::View& CameraController::getView() const
{
    return camera;
}