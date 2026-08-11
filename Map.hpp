#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <vector>

class Map
{
public:
    // 加载天空、远景和前景纹理，
    // 并创建用于循环平铺的精灵。
    bool load();

    // 根据摄像机的 X 坐标，
    // 更新远景和前景图层的位置。
    void update(float cameraX);

    // 单独绘制天空层。
    // 天空层不使用 StageCutLight Shader。
    void drawSky(
        sf::RenderWindow& window,
        const sf::RenderStates& states =
        sf::RenderStates::Default
    ) const;

    // 绘制远景和前景。
    // RenderStates 可以携带 StageCutLight Shader。
    //
    // 默认参数允许继续使用：
    // gameMap.draw(window);
    //
    // 传入 Shader 时使用：
    // gameMap.draw(window, cutLightStates);
    void draw(
        sf::RenderWindow& window,
        const sf::RenderStates& states =
        sf::RenderStates::Default
    ) const;

private:
    // =========================================================
    // 天空层
    // =========================================================

    sf::Texture skyTexture;
    std::vector<sf::Sprite> skyTiles;

    // =========================================================
    // 远景层
    // =========================================================

    sf::Texture farTexture;
    std::vector<sf::Sprite> farTiles;

    // =========================================================
    // 前景层
    // =========================================================

    sf::Texture sceneTexture;
    std::vector<sf::Sprite> sceneTiles;

    // =========================================================
    // 各层单个图块的宽度
    // =========================================================

    float skyTileWidth = 0.f;
    float farTileWidth = 0.f;
    float sceneTileWidth = 0.f;

    // =========================================================
    // 各层的世界坐标 Y 位置
    // =========================================================

    float skyMapY = -400.f;
    float farMapY = -40.f;
    float sceneMapY = 550.f;

    // =========================================================
    // 地图参数
    // =========================================================

    // 远景视差移动系数。
    // 数值越小，远景移动得越慢。
    static constexpr float farParallax = 0.05f;

    // 用于计算摄像机左边界。
    static constexpr float windowWidth = 1280.f;

    // 每层创建的循环图块数量。
    static constexpr std::size_t tileCount = 4;
};