#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class StageCutLight {
public:
    bool load(const std::string& shaderPath, sf::Vector2u renderSize);
    void resize(sf::Vector2u renderSize);

    void setCenter(sf::Vector2f normalizedPosition);
    void setAngleDegrees(float degrees);
    void setWidth(float normalizedWidth);
    void setEdgeSoftness(float normalizedSoftness);
    void setIntensity(float value);
    void setBeatPulse(float value);

    void apply(sf::RenderTarget& target, const sf::Sprite& sceneLayer);
    sf::Shader& shader() { return shader_; }

private:
    void updateUniforms();

    sf::Shader shader_;
    sf::Vector2u size_{1280u, 720u};
    sf::Vector2f center_{0.55f, 0.54f};
    float angleRadians_ = -0.72f;
    float width_ = 0.22f;
    float softness_ = 0.018f;
    float intensity_ = 1.f;
    float beatPulse_ = 0.f;
};
