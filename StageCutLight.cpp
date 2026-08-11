#include "StageCutLight.hpp"

#include <algorithm>
#include <cmath>

namespace {
constexpr float Pi = 3.14159265358979323846f;
}

bool StageCutLight::load(
    const std::string& shaderPath,
    sf::Vector2u renderSize)
{
    if (!sf::Shader::isAvailable() ||
        !shader_.loadFromFile(shaderPath, sf::Shader::Type::Fragment))
    {
        return false;
    }

    size_ = renderSize;

    shader_.setUniform("texture", sf::Shader::CurrentTexture);
    shader_.setUniform(
        "shadowTint",
        sf::Glsl::Vec3(0.055f, 0.018f, 0.022f)
    );
    shader_.setUniform(
        "lightTint",
        sf::Glsl::Vec3(1.00f, 0.72f, 0.28f)
    );

    shader_.setUniform("shadowExposure", 0.14f);
    shader_.setUniform("lightExposure", 1.35f);
    shader_.setUniform("saturation", 1.10f);

    updateUniforms();
    return true;
}

void StageCutLight::resize(sf::Vector2u renderSize) {
    size_ = renderSize;
    updateUniforms();
}

void StageCutLight::setCenter(sf::Vector2f normalizedPosition) {
    center_ = normalizedPosition;
    updateUniforms();
}

void StageCutLight::setAngleDegrees(float degrees) {
    angleRadians_ = degrees * Pi / 180.f;
    updateUniforms();
}

void StageCutLight::setWidth(float normalizedWidth) {
    width_ = std::clamp(normalizedWidth, 0.001f, 1.5f);
    updateUniforms();
}

void StageCutLight::setEdgeSoftness(float normalizedSoftness) {
    softness_ = std::clamp(normalizedSoftness, 0.0005f, 0.25f);
    updateUniforms();
}

void StageCutLight::setIntensity(float value) {
    intensity_ = std::clamp(value, 0.f, 1.f);
    updateUniforms();
}

void StageCutLight::setBeatPulse(float value) {
    beatPulse_ = std::clamp(value, 0.f, 1.f);
    updateUniforms();
}

void StageCutLight::apply(sf::RenderTarget& target, const sf::Sprite& sceneLayer) {
    target.draw(sceneLayer, &shader_);
}

void StageCutLight::updateUniforms() {
    shader_.setUniform("beamCenter", sf::Glsl::Vec2(center_));
    shader_.setUniform("beamAngle", angleRadians_);
    shader_.setUniform("beamWidth", width_);
    shader_.setUniform("edgeSoftness", softness_);
    shader_.setUniform("intensity", intensity_);
    shader_.setUniform("beatPulse", beatPulse_);
    shader_.setUniform("resolution", sf::Glsl::Vec2(
        static_cast<float>(size_.x), static_cast<float>(size_.y)));
}
