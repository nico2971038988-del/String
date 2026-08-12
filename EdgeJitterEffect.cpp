#include "EdgeJitterEffect.hpp"

#include <algorithm>
#include <iostream>

bool EdgeJitterEffect::initialize(
    const sf::Vector2u size,
    const std::filesystem::path& fragmentShaderPath)
{
    initialized_ = false;
    resultSprite_.reset();

    if (!sf::Shader::isAvailable())
    {
        std::cerr << "Shaders are not supported by this GPU/OpenGL context.\n";
        return false;
    }

    if (!shader_.loadFromFile(fragmentShaderPath, sf::Shader::Type::Fragment))
    {
        std::cerr << "Failed to load shader: "
            << fragmentShaderPath.string() << '\n';
        return false;
    }

    if (!resize(size))
        return false;

    shader_.setUniform("texture", sf::Shader::CurrentTexture);
    initialized_ = true;
    clock_.restart();
    updateUniforms();
    return true;
}

bool EdgeJitterEffect::resize(const sf::Vector2u size)
{
    if (size.x == 0u || size.y == 0u)
    {
        std::cerr << "EdgeJitterEffect size cannot be zero.\n";
        return false;
    }

    if (!renderTexture_.resize(size))
    {
        std::cerr <<
            "Failed to create edge-jitter render texture.\n";
        return false;
    }

    size_ = size;

    // 直接使用RenderTexture纹理，不再手动垂直翻转
    resultSprite_.emplace(renderTexture_.getTexture());

    if (initialized_)
    {
        updateUniforms();
    }

    return true;
}

void EdgeJitterEffect::beginFrame()
{
    if (!resultSprite_)
        return;

    renderTexture_.clear(sf::Color::Transparent);
}

sf::RenderTexture& EdgeJitterEffect::layer()
{
    return renderTexture_;
}

void EdgeJitterEffect::endFrame()
{
    if (!resultSprite_)
        return;

    renderTexture_.display();
}

void EdgeJitterEffect::display(sf::RenderTarget& target)
{
    if (!initialized_ || !resultSprite_)
        return;

    updateUniforms();

    sf::RenderStates states;
    states.shader = &shader_;
    states.blendMode = sf::BlendAlpha;
    target.draw(*resultSprite_, states);
}

void EdgeJitterEffect::setEnabled(bool enabled)
{
    enabled_ = enabled;
}

bool EdgeJitterEffect::isEnabled() const
{
    return enabled_;
}

void EdgeJitterEffect::setStrength(float strength)
{
    strength_ = std::max(0.0f, strength);
}

void EdgeJitterEffect::setSpeed(float speed)
{
    speed_ = std::max(0.0f, speed);
}

void EdgeJitterEffect::setChromaticOffset(float pixels)
{
    chromaticOffset_ = std::max(0.0f, pixels);
}

void EdgeJitterEffect::setBurstAmount(float amount)
{
    burstAmount_ = std::max(0.0f, amount);
}

void EdgeJitterEffect::restartClock()
{
    clock_.restart();
}

void EdgeJitterEffect::updateUniforms()
{
    shader_.setUniform("time", clock_.getElapsedTime().asSeconds());
    shader_.setUniform(
        "resolution",
        sf::Vector2f{ static_cast<float>(size_.x),
                     static_cast<float>(size_.y) });
    shader_.setUniform("strength", strength_);
    shader_.setUniform("speed", speed_);
    shader_.setUniform("chromaticOffset", chromaticOffset_);
    shader_.setUniform("burstAmount", burstAmount_);
    shader_.setUniform("enabled", enabled_ ? 1.0f : 0.0f);
}