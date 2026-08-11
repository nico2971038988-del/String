#include "Map.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>

// =============================================================
// 加载地图纹理并创建平铺精灵
// =============================================================

bool Map::load()
{
    // ---------------------------------------------------------
    // 1. 加载天空层
    // ---------------------------------------------------------

    if (!skyTexture.loadFromFile(
        "Assets/Arts/Map/"
        "chinese_ancient_painting_sky_2073x758_seamless.png"))
    {
        std::cerr << "Failed to load sky texture.\n";
        return false;
    }

    // ---------------------------------------------------------
    // 2. 加载远景层
    // ---------------------------------------------------------

    if (!farTexture.loadFromFile(
        "Assets/Arts/Map/"
        "chinese_distant_palace_mountains_forest_2073x758_seamless.png"))
    {
        std::cerr << "Failed to load far background texture.\n";
        return false;
    }

    // ---------------------------------------------------------
    // 3. 加载近景层
    // ---------------------------------------------------------

    if (!sceneTexture.loadFromFile(
        "Assets/Arts/Map/14.png"))
    {
        std::cerr << "Failed to load scene texture.\n";
        return false;
    }

    // ---------------------------------------------------------
    // 4. 获取各层纹理宽度
    // ---------------------------------------------------------

    skyTileWidth =
        static_cast<float>(
            skyTexture.getSize().x
            );

    farTileWidth =
        static_cast<float>(
            farTexture.getSize().x
            );

    sceneTileWidth =
        static_cast<float>(
            sceneTexture.getSize().x
            );

    if (skyTileWidth <= 0.f ||
        farTileWidth <= 0.f ||
        sceneTileWidth <= 0.f)
    {
        std::cerr << "Invalid map texture width.\n";
        return false;
    }

    // ---------------------------------------------------------
    // 5. 清除旧精灵
    // ---------------------------------------------------------

    skyTiles.clear();
    farTiles.clear();
    sceneTiles.clear();

    skyTiles.reserve(tileCount);
    farTiles.reserve(tileCount);
    sceneTiles.reserve(tileCount);

    // ---------------------------------------------------------
    // 6. 创建循环平铺精灵
    // ---------------------------------------------------------

    for (std::size_t i = 0; i < tileCount; ++i)
    {
        const float tileIndex =
            static_cast<float>(i);

        // 天空层
        skyTiles.emplace_back(skyTexture);

        skyTiles.back().setPosition({
            tileIndex * skyTileWidth,
            skyMapY
            });

        // 远景层
        farTiles.emplace_back(farTexture);

        farTiles.back().setPosition({
            tileIndex * farTileWidth,
            farMapY
            });

        // 近景层
        sceneTiles.emplace_back(sceneTexture);

        sceneTiles.back().setPosition({
            tileIndex * sceneTileWidth,
            sceneMapY
            });
    }

    return true;
}

// =============================================================
// 根据摄像机位置更新远景和近景平铺
// =============================================================

void Map::update(float cameraX)
{
    const float cameraLeft =
        cameraX - windowWidth / 2.f;


    const float cameraTravel =
        std::max(
            0.f,
            cameraX - windowWidth / 2.f
        );

    // =========================================================
   // 1. 天空层循环平铺
  // =========================================================

    const int firstSkyIndex =
        static_cast<int>(
            std::floor(cameraLeft / skyTileWidth)
            ) - 1;

    for (std::size_t i = 0;
        i < skyTiles.size();
        ++i)
    {
        const int worldTileIndex =
            firstSkyIndex +
            static_cast<int>(i);

        skyTiles[i].setPosition({
            static_cast<float>(worldTileIndex) *
                skyTileWidth,
            skyMapY
            });
    }


    // ---------------------------------------------------------
    // 1. 更新近景层
    // ---------------------------------------------------------

  //  const float cameraLeft =
  //      cameraX - windowWidth / 2.f;

    const int firstSceneIndex =
        static_cast<int>(
            std::floor(
                cameraLeft / sceneTileWidth
            )
            ) - 1;

    for (std::size_t i = 0;
        i < sceneTiles.size();
        ++i)
    {
        const int worldTileIndex =
            firstSceneIndex +
            static_cast<int>(i);

        sceneTiles[i].setPosition({
            static_cast<float>(worldTileIndex) *
                sceneTileWidth,
            sceneMapY
            });
    }

    // ---------------------------------------------------------
    // 2. 更新远景视差层
    // ---------------------------------------------------------

    const float farOffset =
        cameraTravel * farParallax;

    const float farLayerLeft =
        cameraTravel - farOffset;

    const int firstFarIndex =
        static_cast<int>(
            std::floor(
                farOffset / farTileWidth
            )
            ) - 1;

    for (std::size_t i = 0;
        i < farTiles.size();
        ++i)
    {
        const int farTileIndex =
            firstFarIndex +
            static_cast<int>(i);

        farTiles[i].setPosition({
            farLayerLeft +
                static_cast<float>(farTileIndex) *
                    farTileWidth,
            farMapY
            });
    }

    // 天空层不在这里更新，保持原有屏幕位置。
}

// =============================================================
// 绘制天空层
// 天空不使用 StageCutLight Shader
// =============================================================

void Map::drawSky(
    sf::RenderWindow& window,
    const sf::RenderStates& states) const
{
    for (const sf::Sprite& tile : skyTiles)
    {
        window.draw(tile, states);
    }
}

// =============================================================
// 绘制远景和近景
// states 中可以携带 StageCutLight Shader
// =============================================================

void Map::draw(
    sf::RenderWindow& window,
    const sf::RenderStates& states) const
{
    // 远景层使用传入的 Shader
    for (const sf::Sprite& tile : farTiles)
    {
        window.draw(tile, states);
    }

    // 近景层使用传入的 Shader
    for (const sf::Sprite& tile : sceneTiles)
    {
        window.draw(tile, states);
    }
}