#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <vector>

class Map
{
public:
    // 加载天空、远景和前景图片，并创建循环图块。
    bool load();

    // 根据摄像机的 X 坐标更新三个图层的位置。
    void update(float cameraX);

    // 按照“天空 -> 远景 -> 前景”的顺序绘制。
    void draw(sf::RenderWindow& window) const;
    void drawSky(sf::RenderWindow& window) const;

private:
    sf::Texture skyTexture;
    std::vector<sf::Sprite> skyTiles;

    sf::Texture farTexture;
    std::vector<sf::Sprite> farTiles;

    sf::Texture sceneTexture;
    std::vector<sf::Sprite> sceneTiles;

    float skyTileWidth = 0.f;
    float farTileWidth = 0.f;
    float sceneTileWidth = 0.f;

    float skyMapY = 0.f;
    float farMapY = -40.f;
    float sceneMapY = 550.f;

    // 天空与远景使用完全相同的移动速度。
    static constexpr float farParallax = 0.05f;

    static constexpr float windowWidth = 1280.f;
    static constexpr std::size_t tileCount = 4;
};