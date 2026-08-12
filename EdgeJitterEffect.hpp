#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <optional>

// Draw only the transparent PNG line-art that should be distorted into layer().
// Then call display() once to composite the result onto the main window.
class EdgeJitterEffect
{
public:
    EdgeJitterEffect() = default;

    [[nodiscard]] bool initialize(
        sf::Vector2u size,
        const std::filesystem::path& fragmentShaderPath);

    [[nodiscard]] bool resize(sf::Vector2u size);

    void beginFrame();
    [[nodiscard]] sf::RenderTexture& layer();
    void endFrame();
    void display(sf::RenderTarget& target);

    void setEnabled(bool enabled);
    [[nodiscard]] bool isEnabled() const;

    void setStrength(float strength);           // Recommended: 0.6 - 1.4
    void setSpeed(float speed);                 // Recommended: 0.7 - 1.5
    void setChromaticOffset(float pixels);      // Recommended: 1.0 - 2.5
    void setBurstAmount(float amount);          // 0 disables large tears

    void restartClock();

private:
    void updateUniforms();

    sf::RenderTexture renderTexture_;
    std::optional<sf::Sprite> resultSprite_;
    sf::Shader shader_;
    sf::Clock clock_;
    sf::Vector2u size_{};

    bool initialized_{ false };
    bool enabled_{ true };
    float strength_{ 1.0f };
    float speed_{ 1.0f };
    float chromaticOffset_{ 1.8f };
    float burstAmount_{ 1.0f };
};