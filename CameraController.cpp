#include "CameraController.hpp"


CameraController::CameraController(
    sf::Vector2f initialCenter,
    sf::Vector2f viewSize)
    : camera(initialCenter, viewSize)
{
}

void CameraController::update(
    float deltaTime,
    float musicTime,
    sf::Vector2f playerPosition)
{
    // 音乐时间现在由事件总线处理；保留参数以兼容现有调用。
    (void)musicTime;

    // 玩家越过屏幕中线后，镜头才开始向右跟随。
    const float cameraX = std::max(minimumCameraX, playerPosition.x);
    camera.setCenter({ cameraX, 360.f });

    // 每帧平滑靠近最近一个音乐事件指定的目标角度。
    const float maximumStep = activeRotationSpeed * deltaTime;
    if (currentAngle < targetAngle) {
        currentAngle = std::min(currentAngle + maximumStep, targetAngle);
    }
    else if (currentAngle > targetAngle) {
        currentAngle = std::max(currentAngle - maximumStep, targetAngle);
    }

    // SFML 3 写法。
    camera.setRotation(sf::degrees(currentAngle));


}

void CameraController::onMusicEvent(const MusicRhythmEvent& event)
{
    constexpr float cameraAccentThreshold = 1.0f;

    if (event.intensity < cameraAccentThreshold) {
        return;
    }

    // direction 为 0 的节点不增加转角，只让镜头运动放缓。
    if (event.direction == 0) {
        activeRotationSpeed = rotationSpeed * 0.5f;
        return;
    }

    constexpr float angleStep = 180.0f;

    // 每次事件固定旋转90°
    targetAngle += static_cast<float>(event.direction) * angleStep;

    // 使用较低的速度，让镜头缓慢转到目标角度
    activeRotationSpeed = rotationSpeed * 1.0f;


}

const sf::View& CameraController::getView() const
{
    return camera;
}