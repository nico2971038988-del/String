#include "OutlineRenderer.hpp"
#include <algorithm>

bool OutlineRenderer::create(sf::Vector2u size, const std::string& shaderDirectory) {
    if (!sf::Shader::isAvailable()) return false;

    const bool shadersLoaded =
        alphaMaskShader_.loadFromFile(shaderDirectory + "/alpha_mask.frag", sf::Shader::Type::Fragment) &&
        contourShader_.loadFromFile(shaderDirectory + "/outer_contour.frag", sf::Shader::Type::Fragment) &&
        mixShader_.loadFromFile(shaderDirectory + "/mix_outline.frag", sf::Shader::Type::Fragment);

    if (!shadersLoaded) return false;
    resize(size);
    return silhouette_.getSize() == size_ && outlines_.getSize() == size_;
}

void OutlineRenderer::resize(sf::Vector2u size) {
    size_ = size;
    silhouette_.resize(size);
    outlines_.resize(size);
    silhouette_.setSmooth(false);
    outlines_.setSmooth(false);
}

void OutlineRenderer::setLineWidth(float pixels) {
    lineWidth_ = std::clamp(pixels, 1.f, 8.f);
}

void OutlineRenderer::setEffectAmount(float amount) {
    effectAmount_ = std::clamp(amount, 0.f, 1.f);
}

void OutlineRenderer::setNormalScene(const sf::Texture* texture) {
    normalScene_ = texture;
}

void OutlineRenderer::clearLayers() {
    layers_.clear();
}

void OutlineRenderer::addLayer(DrawLayer layer) {
    layers_.push_back(std::move(layer));
}

void OutlineRenderer::render(sf::RenderTarget& target) {
    outlines_.clear(sf::Color::Transparent);

    contourShader_.setUniform("texture", sf::Shader::CurrentTexture);
    contourShader_.setUniform("texelSize", sf::Glsl::Vec2(
        1.f / static_cast<float>(size_.x),
        1.f / static_cast<float>(size_.y)));
    contourShader_.setUniform("lineWidth", lineWidth_);

    // Each layer is outlined separately. This preserves a mountain contour behind
    // a palace while still removing all colour/detail inside each texture.
    for (const auto& drawLayer : layers_) {
        silhouette_.clear(sf::Color::Transparent);
        drawLayer(silhouette_);
        silhouette_.display();

        sf::Sprite maskSprite(silhouette_.getTexture());
        outlines_.draw(maskSprite, &contourShader_);
    }
    outlines_.display();

    sf::RectangleShape screen(sf::Vector2f(
        static_cast<float>(size_.x), static_cast<float>(size_.y)));
    screen.setFillColor(sf::Color::Black);
    target.draw(screen);

    if (effectAmount_ >= 0.999f || normalScene_ == nullptr) {
        target.draw(sf::Sprite(outlines_.getTexture()));
        return;
    }

    mixShader_.setUniform("texture", *normalScene_);
    mixShader_.setUniform("outlineTexture", outlines_.getTexture());
    mixShader_.setUniform("amount", effectAmount_);
    target.draw(sf::Sprite(*normalScene_), &mixShader_);
}

