#pragma once

#include <SFML/Graphics.hpp>

class Lighting
{
public:
    // 加载光照纹理并初始化黑暗遮罩
    bool load();
    Lighting();

    // 让光源跟随玩家中心
    void update();

    // 绘制黑暗遮罩和光源
    void draw(sf::RenderWindow& window) const;

    // 使用屏幕坐标绘制月亮
    void drawMoon(sf::RenderWindow& window) const;


private:
    // 必须先声明Texture，再声明Sprite
    sf::Texture lightTexture;

    // SFML 3：创建Sprite时绑定Texture
    sf::Sprite lightSprite{ lightTexture };

    // 整体环境黑暗遮罩
    sf::RectangleShape darkness;

    //月光
    sf::CircleShape moon;
    sf::CircleShape moonGlow;

   
};