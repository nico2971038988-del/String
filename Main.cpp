#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>

#include "CameraController.hpp"
#include "Lighting.hpp"
#include "Map.hpp"
#include "MusicRhythmEventBus.hpp"
#include "PlayerAssets.hpp"
#include "RhythmLine.hpp"
#include "StageCutLight.hpp"

int main()
{
    constexpr unsigned int windowWidth = 1280;
    constexpr unsigned int windowHeight = 720;
    constexpr float playerSpeed = 200.f;
    constexpr float playerScale = 0.15f;
    constexpr float walkFrameDuration = 0.15f;
    constexpr float lightPulseDecay = 3.f;

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode({ windowWidth, windowHeight }),
        "Shader Test - S: Shader  L: Darkness  Space: Pulse",
        sf::Style::Titlebar | sf::Style::Close,
        sf::State::Windowed,
        settings
    );
    window.setFramerateLimit(60);

    PlayerTextures playerTextures;
    if (!loadWalkTextures(playerTextures) ||
        playerTextures.walkFrames.empty())
    {
        std::cerr << "Failed to load player textures.\n";
        return 1;
    }

    sf::Sprite playerSprite(playerTextures.walkFrames[0]);
    playerSprite.setScale({ playerScale, playerScale });
    playerSprite.setPosition({ 100.f, 480.f });

    std::size_t walkFrameIndex = 0;
    sf::Clock animationClock;

    Map gameMap;
    if (!gameMap.load())
    {
        std::cerr << "Failed to load map.\n";
        return 1;
    }

    Lighting lighting;
    if (!lighting.load())
    {
        std::cerr << "Failed to load lighting.\n";
        return 1;
    }

    sf::Music backgroundMusic;
    MusicRhythmEventBus rhythmBus;
    if (!initializeMusic(backgroundMusic, "Assets/Arts/Audio/1.ogg"))
    {
        std::cerr << "Failed to load background music.\n";
        return 1;
    }
    rhythmBus.reset();

    RhythmLine rhythmLine;
    CameraController cameraController(
        { windowWidth / 2.f, windowHeight / 2.f },
        { static_cast<float>(windowWidth),
         static_cast<float>(windowHeight) }
    );

    rhythmBus.subscribe(
        [&cameraController](const MusicRhythmEvent& event)
        {
            cameraController.onMusicEvent(event);
        }
    );

    StageCutLight cutLight;
    const bool cutLightLoaded = cutLight.load(
        "Assets/Shaders/stage_cut_light.frag",
        window.getSize()
    );

    std::cout << std::boolalpha
        << "cutLightLoaded = " << cutLightLoaded << '\n';

    if (!cutLightLoaded)
    {
        std::cerr << "Shader load/compile failed; normal map drawing will be used.\n";
    }
    else
    {
        cutLight.setCenter({ 0.6f, 0.52f });
        cutLight.setAngleDegrees(-40.f);
        cutLight.setWidth(0.2f);
        cutLight.setEdgeSoftness(0.1f);
        cutLight.setIntensity(0.92f);
        cutLight.setBeatPulse(0.f);
    }

    bool shaderEnabled = cutLightLoaded;

    // 测试时默认关闭黑暗遮罩，避免它盖住 Shader 效果。
    bool darknessEnabled = false;
    float lightPulse = 0.f;

    std::cout << "S = toggle shader\n"
        << "L = toggle darkness overlay\n"
        << "Space = light pulse\n"
        << "Escape = quit\n";

    sf::Clock frameClock;

    while (window.isOpen())
    {
        const float safeDeltaTime = std::min(
            frameClock.restart().asSeconds(),
            0.1f
        );

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (const auto* keyPressed =
                event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
                else if (keyPressed->code == sf::Keyboard::Key::Space)
                {
                    lightPulse = 1.f;
                }
                else if (keyPressed->code == sf::Keyboard::Key::S)
                {
                    if (cutLightLoaded)
                    {
                        shaderEnabled = !shaderEnabled;
                        std::cout << "shaderEnabled = "
                            << shaderEnabled << '\n';
                    }
                }
                else if (keyPressed->code == sf::Keyboard::Key::L)
                {
                    darknessEnabled = !darknessEnabled;
                    std::cout << "darknessEnabled = "
                        << darknessEnabled << '\n';
                }
            }
        }

        lightPulse = std::max(
            0.f,
            lightPulse - safeDeltaTime * lightPulseDecay
        );

        if (cutLightLoaded)
        {
            cutLight.setBeatPulse(lightPulse);
        }

        playerSprite.move({ playerSpeed * safeDeltaTime, 0.f });

        if (animationClock.getElapsedTime().asSeconds() >=
            walkFrameDuration)
        {
            walkFrameIndex =
                (walkFrameIndex + 1) % playerTextures.walkFrames.size();
            playerSprite.setTexture(
                playerTextures.walkFrames[walkFrameIndex],
                true
            );
            animationClock.restart();
        }

        if (playerSprite.getPosition().x < 0.f)
        {
            playerSprite.setPosition({ 0.f, playerSprite.getPosition().y });
        }

        const sf::FloatRect playerBounds =
            playerSprite.getGlobalBounds();
        const sf::Vector2f playerCenter{
            playerBounds.position.x + playerBounds.size.x / 2.f,
            playerBounds.position.y + playerBounds.size.y / 2.f
        };

        gameMap.update(playerCenter.x);
        lighting.update();
        rhythmBus.update(backgroundMusic);

        const float musicTime =
            backgroundMusic.getPlayingOffset().asSeconds();

        rhythmLine.update(safeDeltaTime, musicTime);
        cameraController.update(
            safeDeltaTime,
            musicTime,
            playerCenter
        );

        const sf::View gameView = cameraController.getView();

        window.clear(sf::Color::Black);

        // 先准备 Shader
        sf::RenderStates cutLightStates =
            sf::RenderStates::Default;

        if (cutLightLoaded)
        {
            cutLightStates.shader = &cutLight.shader();
        }

        // 1. 天空：使用游戏视图并接受 Shader
        window.setView(gameView);

        if (shaderEnabled && cutLightLoaded)
        {
            gameMap.drawSky(window, cutLightStates);
        }
        else
        {
            gameMap.drawSky(window);
        }

        // 2. 月亮：使用默认视图，固定在屏幕上，不接受 Shader
        window.setView(window.getDefaultView());
        lighting.drawMoon(window);

        // 3. 房屋：切回游戏视图并接受 Shader
        // 房屋后绘制，因此可以遮挡月亮
        window.setView(gameView);

        if (shaderEnabled && cutLightLoaded)
        {
            gameMap.draw(window, cutLightStates);
        }
        else
        {
            gameMap.draw(window);
        }

        // 4. 玩家：使用游戏视图且不接受 Shader
        window.draw(playerSprite);

        // 5. 节奏线：固定在屏幕上
        window.setView(window.getDefaultView());
        rhythmLine.draw(window);

        window.display();
    }

    return 0;
}