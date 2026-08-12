#pragma once

#include <SFML/Graphics.hpp>

#include "MusicRhythmEventBus.hpp"

class CameraController
{
public:
    CameraController(sf::Vector2f initialCenter, sf::Vector2f viewSize);

    void update(
        float deltaTime,
        float musicTime,
        sf::Vector2f playerPosition);

    void onMusicEvent(const MusicRhythmEvent& event);

    [[nodiscard]] const sf::View& getView() const;

private:
    sf::View camera;

    float minimumCameraX = 640.f;
    float currentAngle = 0.f;
    float targetAngle = 0.f;

    float rotationSpeed = 600.f;
    float activeRotationSpeed = 150.f;
};