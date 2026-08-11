#include "Lighting.hpp"

#include <iostream>



bool Lighting::load()
{
    if (!lightTexture.loadFromFile(
        "Assets/Arts/effects/radial-light-texture-512.png"))
    {
        std::cerr << "Failed to load light texture\n";
        return false;
    }

    // 关键修复：true 表示重置为整张图片的尺寸
    lightSprite.setTexture(lightTexture, true);

    lightTexture.setSmooth(true);

    const sf::Vector2u size = lightTexture.getSize();

    lightSprite.setOrigin({
        static_cast<float>(size.x) / 2.f,
        static_cast<float>(size.y) / 2.f
        });

    lightSprite.setScale({ 0.45f, 0.45f });
    lightSprite.setColor(sf::Color(255, 235, 200, 75));

    darkness.setSize({ 100000.f, 720.f });
    darkness.setPosition({ -1000.f, 0.f });
    darkness.setFillColor(sf::Color(10, 15, 30, 150));

    return true;
}

Lighting::Lighting()
{
    // 月亮光晕
  //  moonGlow.setRadius(200.f);
   // moonGlow.setOrigin({ 75.f, 175.f });
  //  moonGlow.setPosition({ 180.f, 130.f });

  //  moonGlow.setFillColor(
   //     sf::Color(180, 210, 255, 35)
  //  );

    // 月亮主体
    moon.setRadius(250.f);
    moon.setOrigin({ 42.f, 100.f });
    moon.setPosition({ 50.f, 180.f });
    moon.setPointCount(240);  // 默认较低，提高圆周精度
    moon.setFillColor(
        sf::Color(255, 204, 66, 200 )
    );
}


void Lighting::update()
{
    // 光源跟随玩家中心
    ///lightSprite.setPosition(playerPosition);

}

void Lighting::draw(sf::RenderWindow& window) const
{
    // 先让整体场景变暗
 //  window.draw(darkness);
    //window.draw(lightSprite);
    // 再叠加光照纹理
  // window.draw(
   //     lightSprite,
   //     sf::RenderStates(sf::BlendAdd)
   // );


}

void Lighting::drawMoon(
    sf::RenderWindow& window) const
{
    window.draw(
        moonGlow,
        sf::RenderStates(sf::BlendAdd)
    );
    window.draw(moon);
   
}

