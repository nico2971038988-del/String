#include "Map.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace
{
    constexpr const char* SkyPath =
        "Assets/Arts/Map/chinese_ancient_painting_sky_2073x758_seamless.png";
    constexpr const char* FarPath =
        "Assets/Arts/Map/chinese_distant_palace_mountains_forest_2073x758_seamless.png";
    constexpr const char* ScenePath =
        "Assets/Arts/Map/14.png";

    // Change only these two paths if your outline PNG names are different.
    constexpr const char* OutlineFarPath =
        "Assets/Arts/Map/outline_far.png";
    constexpr const char* OutlineScenePath =
        "Assets/Arts/Map/outline_scene.png";
}

bool Map::load()
{
    if (!skyTexture.loadFromFile(SkyPath))
    {
        std::cerr << "Failed to load sky texture: " << SkyPath << '\n';
        return false;
    }
    if (!farTexture.loadFromFile(FarPath))
    {
        std::cerr << "Failed to load far texture: " << FarPath << '\n';
        return false;
    }
    if (!sceneTexture.loadFromFile(ScenePath))
    {
        std::cerr << "Failed to load scene texture: " << ScenePath << '\n';
        return false;
    }
    if (!outlineFarTexture.loadFromFile(OutlineFarPath))
    {
        std::cerr << "Failed to load outline far texture: "
            << OutlineFarPath << '\n';
        return false;
    }
    if (!outlineSceneTexture.loadFromFile(OutlineScenePath))
    {
        std::cerr << "Failed to load outline scene texture: "
            << OutlineScenePath << '\n';
        return false;
    }

    skyTileWidth = static_cast<float>(skyTexture.getSize().x);
    farTileWidth = static_cast<float>(farTexture.getSize().x);
    sceneTileWidth = static_cast<float>(sceneTexture.getSize().x);
    outlineFarTileWidth =
        static_cast<float>(outlineFarTexture.getSize().x);
    outlineSceneTileWidth =
        static_cast<float>(outlineSceneTexture.getSize().x);

    if (skyTileWidth <= 0.f || farTileWidth <= 0.f ||
        sceneTileWidth <= 0.f || outlineFarTileWidth <= 0.f ||
        outlineSceneTileWidth <= 0.f)
    {
        std::cerr << "One or more map textures have an invalid width.\n";
        return false;
    }

    skyTiles.clear();
    farTiles.clear();
    sceneTiles.clear();
    outlineFarTiles.clear();
    outlineSceneTiles.clear();

    skyTiles.reserve(tileCount);
    farTiles.reserve(tileCount);
    sceneTiles.reserve(tileCount);
    outlineFarTiles.reserve(tileCount);
    outlineSceneTiles.reserve(tileCount);

    for (std::size_t i = 0; i < tileCount; ++i)
    {
        const float index = static_cast<float>(i);

        skyTiles.emplace_back(skyTexture);
        skyTiles.back().setPosition({ index * skyTileWidth, skyMapY });

        farTiles.emplace_back(farTexture);
        farTiles.back().setPosition({ index * farTileWidth, farMapY });

        sceneTiles.emplace_back(sceneTexture);
        sceneTiles.back().setPosition({ index * sceneTileWidth, sceneMapY });

        outlineFarTiles.emplace_back(outlineFarTexture);
        outlineFarTiles.back().setPosition(
            { index * outlineFarTileWidth, farMapY });

        outlineSceneTiles.emplace_back(outlineSceneTexture);
        outlineSceneTiles.back().setPosition(
            { index * outlineSceneTileWidth, sceneMapY });
    }

    return true;
}

void Map::setOutlineMode(bool enabled)
{
    outlineMode = enabled;
}

bool Map::isOutlineMode() const
{
    return outlineMode;
}

void Map::update(float cameraX)
{
    const float cameraLeft = cameraX - windowWidth / 2.f;
    const float cameraTravel = std::max(0.f, cameraLeft);

    const auto updateWorldLayer = [cameraLeft](
        std::vector<sf::Sprite>& tiles,
        float tileWidth,
        float y)
        {
            const int firstIndex =
                static_cast<int>(std::floor(cameraLeft / tileWidth)) - 1;

            for (std::size_t i = 0; i < tiles.size(); ++i)
            {
                const int tileIndex = firstIndex + static_cast<int>(i);
                tiles[i].setPosition(
                    { static_cast<float>(tileIndex) * tileWidth, y });
            }
        };

    const auto updateFarLayer = [cameraTravel](
        std::vector<sf::Sprite>& tiles,
        float tileWidth,
        float y)
        {
            const float farOffset = cameraTravel * farParallax;
            const float farLayerLeft = cameraTravel - farOffset;
            const int firstIndex =
                static_cast<int>(std::floor(farOffset / tileWidth)) - 1;

            for (std::size_t i = 0; i < tiles.size(); ++i)
            {
                const int tileIndex = firstIndex + static_cast<int>(i);
                tiles[i].setPosition(
                    { farLayerLeft + static_cast<float>(tileIndex) * tileWidth, y });
            }
        };

    // Sky follows the existing world-view positioning in normal mode.
    updateWorldLayer(skyTiles, skyTileWidth, skyMapY);

    // Normal and outline counterparts use the same movement formula.
    updateFarLayer(farTiles, farTileWidth, farMapY);
    updateFarLayer(outlineFarTiles, outlineFarTileWidth, farMapY);
    updateWorldLayer(sceneTiles, sceneTileWidth, sceneMapY);
    updateWorldLayer(outlineSceneTiles, outlineSceneTileWidth, sceneMapY);
}

void Map::drawSky(
    sf::RenderTarget& target,
    const sf::RenderStates& states) const
{
    if (outlineMode)
    {
        return;
    }

    for (const sf::Sprite& tile : skyTiles)
    {
        target.draw(tile, states);
    }
}

void Map::draw(
    sf::RenderTarget& target,
    const sf::RenderStates& states) const
{
    if (outlineMode)
    {
        for (const sf::Sprite& tile : outlineFarTiles)
        {
            target.draw(tile, states);
        }
        for (const sf::Sprite& tile : outlineSceneTiles)
        {
            target.draw(tile, states);
        }
        return;
    }

    for (const sf::Sprite& tile : farTiles)
    {
        target.draw(tile, states);
    }
    for (const sf::Sprite& tile : sceneTiles)
    {
        target.draw(tile, states);
    }
}