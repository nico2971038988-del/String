#include "Map.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

bool Map::load()
{
    if (!skyTexture.loadFromFile(
        "Assets/Arts/Map/chinese_ancient_painting_sky_2073x758_seamless.png"))
    {
        std::cerr << "Failed to load sky.png\n";
        return false;
    }

    if (!farTexture.loadFromFile(
        "Assets/Arts/Map/chinese_distant_palace_mountains_forest_2073x758_seamless.png"))
    {
        std::cerr << "Failed to load far background\n";
        return false;
    }

    if (!sceneTexture.loadFromFile(
        "Assets/Arts/Map/14.png"))
    {
        std::cerr << "Failed to load scene.png\n";
        return false;
    }

    skyTileWidth =
        static_cast<float>(skyTexture.getSize().x);
    farTileWidth =
        static_cast<float>(farTexture.getSize().x);
    sceneTileWidth =
        static_cast<float>(sceneTexture.getSize().x);

    if (skyTileWidth <= 0.f ||
        farTileWidth <= 0.f ||
        sceneTileWidth <= 0.f)
    {
        std::cerr << "Invalid map texture width\n";
        return false;
    }

    skyTiles.clear();
    farTiles.clear();
    sceneTiles.clear();

    skyTiles.reserve(tileCount);
    farTiles.reserve(tileCount);
    sceneTiles.reserve(tileCount);

    for (std::size_t i = 0; i < tileCount; ++i)
    {
        skyTiles.emplace_back(skyTexture);
        skyTiles.back().setPosition({
            static_cast<float>(i) * skyTileWidth,
            skyMapY
            });

        farTiles.emplace_back(farTexture);
        farTiles.back().setPosition({
            static_cast<float>(i) * farTileWidth,
            farMapY
            });

        sceneTiles.emplace_back(sceneTexture);
        sceneTiles.back().setPosition({
            static_cast<float>(i) * sceneTileWidth,
            sceneMapY
            });
    }

    return true;
}

void Map::update(float cameraX)
{
    const float cameraTravel =
        std::max(
            0.f,
            cameraX - windowWidth / 2.f
        );

    // 远景的视差偏移量。
    const float farOffset =
        cameraTravel * farParallax;

    const float farLayerLeft =
        cameraTravel - farOffset;

    const float cameraLeft =
        cameraX - windowWidth / 2.f;

    const int firstSceneIndex =
        static_cast<int>(
            std::floor(cameraLeft / sceneTileWidth)
            ) - 1;

    for (std::size_t i = 0;
        i < sceneTiles.size();
        ++i)
    {
        const int worldTileIndex =
            firstSceneIndex + static_cast<int>(i);

        sceneTiles[i].setPosition({
            static_cast<float>(worldTileIndex) *
                sceneTileWidth,
            sceneMapY
            });
    }

    const int firstFarIndex =
        static_cast<int>(
            std::floor(farOffset / farTileWidth)
            ) - 1;

    for (std::size_t i = 0;
        i < farTiles.size();
        ++i)
    {
        const int farTileIndex =
            firstFarIndex + static_cast<int>(i);

        farTiles[i].setPosition({
            farLayerLeft +
                static_cast<float>(farTileIndex) *
                    farTileWidth,
            farMapY
            });
    }

    // 天空不参与更新；它始终保持加载时的屏幕坐标。
}

void Map::drawSky(sf::RenderWindow& window) const
{
    for (const sf::Sprite& tile : skyTiles)
    {
        window.draw(tile);
    }
}

void Map::draw(sf::RenderWindow& window) const
{
    for (const sf::Sprite& tile : farTiles)
    {
        window.draw(tile);
    }

    for (const sf::Sprite& tile : sceneTiles)
    {
        window.draw(tile);
    }
}