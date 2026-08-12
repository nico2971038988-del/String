#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>

// Renders texture-alpha silhouettes as clean white outer contours on black.
// Put objects that should keep independent contours in separate layers.
class OutlineRenderer {
public:
    using DrawLayer = std::function<void(sf::RenderTarget&)>;

    bool create(sf::Vector2u size, const std::string& shaderDirectory);
    void resize(sf::Vector2u size);

    void setLineWidth(float pixels);       // Recommended: 1.5f - 3.0f
    void setEffectAmount(float amount);    // 0 = normal scene, 1 = outline mode
    void setNormalScene(const sf::Texture* texture);

    void clearLayers();
    void addLayer(DrawLayer layer);
    void render(sf::RenderTarget& target);

private:
    sf::Vector2u size_{};
    float lineWidth_ = 2.f;
    float effectAmount_ = 1.f;
    const sf::Texture* normalScene_ = nullptr;
    std::vector<DrawLayer> layers_;

    sf::RenderTexture silhouette_;
    sf::RenderTexture outlines_;
    sf::Shader alphaMaskShader_;
    sf::Shader contourShader_;
    sf::Shader mixShader_;
};

