#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>

#include "CameraController.hpp"
#include "EdgeJitterEffect.hpp"
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
        "Rhythm Game - 0: Outline Mode",
        sf::Style::Titlebar | sf::Style::Close,
        sf::State::Windowed,
        settings);
    window.setFramerateLimit(60);

    PlayerTextures playerTextures;
    if (!loadWalkTextures(playerTextures) || playerTextures.walkFrames.empty())
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
        { static_cast<float>(windowWidth), static_cast<float>(windowHeight) });

    StageCutLight cutLight;
    const bool cutLightLoaded = cutLight.load(
        "Assets/Shaders/stage_cut_light.frag", window.getSize());

    if (cutLightLoaded)
    {
        cutLight.setCenter({ 0.6f, 0.52f });
        cutLight.setAngleDegrees(-40.f);
        cutLight.setWidth(0.2f);
        cutLight.setEdgeSoftness(0.1f);
        cutLight.setIntensity(0.92f);
        cutLight.setBeatPulse(0.f);
    }

    bool shaderEnabled = cutLightLoaded;
    bool darknessEnabled = false;
    float lightPulse = 0.f;

    EdgeJitterEffect edgeJitter;
    const bool edgeJitterLoaded = edgeJitter.initialize(
        window.getSize(), "Assets/Shaders/edge_jitter.frag");

    if (edgeJitterLoaded)
    {
        edgeJitter.setStrength(1.0f);
        edgeJitter.setSpeed(1.0f);
        edgeJitter.setChromaticOffset(1.8f);
        edgeJitter.setBurstAmount(1.0f);
    }

    // 镜头监听音乐时间轴事件。
    rhythmBus.subscribe(
        [&cameraController](const MusicRhythmEvent& event)
        {
            cameraController.onMusicEvent(event);
        });

    // Outline Mode 监听同一个音乐时间轴事件。
    // 14.000f 的第一个事件被广播时，与镜头旋转同帧进入 Outline。
    rhythmBus.subscribe(
        [&gameMap, &edgeJitter, edgeJitterLoaded](
            const MusicRhythmEvent& event)
        {
            (void)event;

            const bool enteringOutline = !gameMap.isOutlineMode();
            gameMap.setOutlineMode(true);

            if (enteringOutline && edgeJitterLoaded)
            {
                edgeJitter.restartClock();
            }
        });

    std::cout << std::boolalpha
        << "0 = toggle normal/outline map\n"
        << "S = toggle shader\n"
        << "L = toggle darkness overlay\n"
        << "Space = light pulse\n"
        << "Escape = quit\n";

    sf::Clock frameClock;

    while (window.isOpen())
    {
        const float safeDeltaTime =
            std::min(frameClock.restart().asSeconds(), 0.1f);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num0)
                {
                    gameMap.setOutlineMode(!gameMap.isOutlineMode());
                    if (gameMap.isOutlineMode() && edgeJitterLoaded)
                    {
                        edgeJitter.restartClock();
                    }
                    std::cout << "outlineMode = "
                        << gameMap.isOutlineMode() << '\n';
                    shaderEnabled = !shaderEnabled;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Space)
                {
                    lightPulse = 1.f;
                }
                else if (keyPressed->code == sf::Keyboard::Key::S &&
                    cutLightLoaded)
                {
                    shaderEnabled = !shaderEnabled;
                }
                else if (keyPressed->code == sf::Keyboard::Key::L)
                {
                    darknessEnabled = !darknessEnabled;
                }
            }
        }

        lightPulse = std::max(
            0.f, lightPulse - safeDeltaTime * lightPulseDecay);
        if (cutLightLoaded)
        {
            cutLight.setBeatPulse(lightPulse);
        }

        playerSprite.move({ playerSpeed * safeDeltaTime, 0.f });

        if (animationClock.getElapsedTime().asSeconds() >= walkFrameDuration)
        {
            walkFrameIndex =
                (walkFrameIndex + 1) % playerTextures.walkFrames.size();
            playerSprite.setTexture(playerTextures.walkFrames[walkFrameIndex], true);
            animationClock.restart();
        }

        if (playerSprite.getPosition().x < 0.f)
        {
            playerSprite.setPosition({ 0.f, playerSprite.getPosition().y });
        }

        const sf::FloatRect playerBounds = playerSprite.getGlobalBounds();
        const sf::Vector2f playerCenter{
            playerBounds.position.x + playerBounds.size.x / 2.f,
            playerBounds.position.y + playerBounds.size.y / 2.f };

        gameMap.update(playerCenter.x);
        lighting.update();
        rhythmBus.update(backgroundMusic);

        const float musicTime =
            backgroundMusic.getPlayingOffset().asSeconds();

        rhythmLine.update(safeDeltaTime, musicTime);
        cameraController.update(safeDeltaTime, musicTime, playerCenter);

        const sf::View gameView = cameraController.getView();
        window.clear(sf::Color::Black);

        sf::RenderStates cutLightStates = sf::RenderStates::Default;
        if (cutLightLoaded)
        {
            cutLightStates.shader = &cutLight.shader();
        }

        // 普通天空只在普通模式下绘制。
        if (!gameMap.isOutlineMode())
        {
            window.setView(gameView);
            if (shaderEnabled && cutLightLoaded)
            {
                gameMap.drawSky(window, cutLightStates);
            }
            else
            {
                gameMap.drawSky(window);
            }
        }

        // 月亮/红日永远先画，后续的普通或线框建筑会遮挡它。
        window.setView(window.getDefaultView());
        lighting.drawMoon(window);

        // Outline Mode: 先把线框远景和近景画到透明效果层，
        // 再通过 edge_jitter.frag 合成到主窗口。
        if (gameMap.isOutlineMode() && edgeJitterLoaded)
        {
            edgeJitter.beginFrame();
            edgeJitter.layer().setView(gameView);
            gameMap.draw(edgeJitter.layer());
            edgeJitter.endFrame();

            window.setView(window.getDefaultView());
            edgeJitter.display(window);
        }
        else
        {
            // 普通模式，或抖动 shader 加载失败时的回退绘制。
            window.setView(gameView);
            if (!gameMap.isOutlineMode() && shaderEnabled && cutLightLoaded)
            {
                gameMap.draw(window, cutLightStates);
            }
            else
            {
                gameMap.draw(window);
            }
        }

        window.setView(gameView);
        window.draw(playerSprite);

        window.setView(window.getDefaultView());
        rhythmLine.draw(window);

        window.display();
    }

    return 0;
}