#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <vector>

class Map
{
public:
    bool load();
    void update(float cameraX);

    // true: only draw the outline far/scene layers.
    // false: draw the normal sky/far/scene layers.
    void setOutlineMode(bool enabled);
    [[nodiscard]] bool isOutlineMode() const;

    void drawSky(
        sf::RenderTarget& target,
        const sf::RenderStates& states = sf::RenderStates::Default
    ) const;

    void draw(
        sf::RenderTarget& target,
        const sf::RenderStates& states = sf::RenderStates::Default
    ) const;

private:
    sf::Texture skyTexture;
    sf::Texture farTexture;
    sf::Texture sceneTexture;
    sf::Texture outlineFarTexture;
    sf::Texture outlineSceneTexture;

    std::vector<sf::Sprite> skyTiles;
    std::vector<sf::Sprite> farTiles;
    std::vector<sf::Sprite> sceneTiles;
    std::vector<sf::Sprite> outlineFarTiles;
    std::vector<sf::Sprite> outlineSceneTiles;

    float skyTileWidth = 0.f;
    float farTileWidth = 0.f;
    float sceneTileWidth = 0.f;
    float outlineFarTileWidth = 0.f;
    float outlineSceneTileWidth = 0.f;

    float skyMapY = -400.f;
    float farMapY = -40.f;
    float sceneMapY = 550.f;

    bool outlineMode = false;

    static constexpr float farParallax = 0.05f;
    static constexpr float windowWidth = 1280.f;
    static constexpr std::size_t tileCount = 4;
};