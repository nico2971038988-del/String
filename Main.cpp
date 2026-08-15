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
#include "UI.hpp"

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
        "Rhythm Game - Outline Mode & Beat Elimination",
        sf::Style::Titlebar | sf::Style::Close,
        sf::State::Windowed,
        settings);
    window.setFramerateLimit(60);

    // 1. 资源与玩家加载
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

    // 2. 地图与灯光系统初始化
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

    // 3. 音频与节奏事件总线
    sf::Music backgroundMusic;
    MusicRhythmEventBus rhythmBus;
    if (!initializeMusic(backgroundMusic, "Assets/Arts/Audio/1.ogg"))
    {
        std::cerr << "Failed to load background music.\n";
        return 1;
    }
    rhythmBus.reset();

    // 4. UI 界面设置
    UI gameUI;
    if (!gameUI.loadProgressBar(
        "Assets/Arts/UI/progress_line.png",
        "Assets/Arts/UI/bar.png"))
    {
        std::cerr << "Failed to load progress bar UI.\n";
        return 1;
    }

    gameUI.bindMusic(backgroundMusic);
    gameUI.setProgressBarPosition({ 100.f, -5.f });
    gameUI.setProgressBarSize({ 1000.f, 40.f });
    gameUI.setProgressSliderSize({ 20.f, 20.f });

    // 5. 节奏线与镜头控制器
    RhythmLine rhythmLine;
    CameraController cameraController(
        { windowWidth / 2.f, windowHeight / 2.f },
        { static_cast<float>(windowWidth), static_cast<float>(windowHeight) });

    // 6. Shader 特效组件
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

    // 7. 镜头与场景反色事件订阅
    rhythmBus.subscribe(
        [&cameraController, &gameMap, &edgeJitter, edgeJitterLoaded](
            const MusicRhythmEvent& event)
        {
            cameraController.onMusicEvent(event);

            const bool enableOutline = !gameMap.isOutlineMode();
            gameMap.setOutlineMode(enableOutline);

            if (enableOutline && edgeJitterLoaded)
            {
                edgeJitter.restartClock();
            }
        });

    sf::Clock frameClock;

    // =================================================================
    // 🔄 主游戏循环 Main Game Loop
    // =================================================================
    while (window.isOpen())
    {
        const float safeDeltaTime =
            std::min(frameClock.restart().asSeconds(), 0.1f);

        // -------------------------------------------------------------
        // 📥 事件处理 Event Handling
        // -------------------------------------------------------------
        while (const std::optional event = window.pollEvent())
        {
            gameUI.handleEvent(*event, window);

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
                    shaderEnabled = !shaderEnabled;
                }
                else if (keyPressed->code == sf::Keyboard::Key::S && cutLightLoaded)
                {
                    // 注意：这里按 S 键如果触发全局 Shader 切换，可能会和向下击打（S键）冲突。
                    // 建议后期改用其他按键切换 Shader，此处保留原逻辑
                }
                else if (keyPressed->code == sf::Keyboard::Key::L)
                {
                    darknessEnabled = !darknessEnabled;
                }

                // 🎮 4轨道击打判定：捕捉玩家输入的【绝对屏幕方向】
                int screenDirection = -1;

                if (keyPressed->code == sf::Keyboard::Key::S || keyPressed->code == sf::Keyboard::Key::Down)
                {
                    screenDirection = 0; // 屏幕下方
                }
                else if (keyPressed->code == sf::Keyboard::Key::W || keyPressed->code == sf::Keyboard::Key::Up)
                {
                    screenDirection = 1; // 屏幕上方
                }
                else if (keyPressed->code == sf::Keyboard::Key::A || keyPressed->code == sf::Keyboard::Key::Left)
                {
                    screenDirection = 2; // 屏幕左方
                }
                else if (keyPressed->code == sf::Keyboard::Key::D || keyPressed->code == sf::Keyboard::Key::Right)
                {
                    screenDirection = 3; // 屏幕右方
                }

                // 按下了绑定的 4 方向击打键
                if (screenDirection != -1)
                {
                    // 🌟 关键修改：传入【屏幕按键方向】和【当前镜头实时角度】进行逆向旋转映射
                    const float currentAngle = cameraController.getCurrentAngle();
                    const HitResult result = rhythmLine.onPlayerPressDirection(screenDirection, currentAngle);

                    if (result == HitResult::Perfect)
                    {
                        std::cout << "✨ [PERFECT] 绝对方向 " << screenDirection << " 音符完美击切！\n";
                        lightPulse = 1.0f;
                    }
                    else if (result == HitResult::Great)
                    {
                        std::cout << "👍 [GREAT] 绝对方向 " << screenDirection << " 音符击切！\n";
                        lightPulse = 0.6f;
                    }
                    else
                    {
                        std::cout << "💨 [MISS] 绝对方向 " << screenDirection << " 空击/偏差过大！\n";
                    }
                }
            }
        }

        // -------------------------------------------------------------
        // ⚙️ 逻辑更新 Logical Update
        // -------------------------------------------------------------
        if (!gameUI.isPaused())
        {
            // 打击光衰减
            lightPulse = std::max(
                0.f, lightPulse - safeDeltaTime * lightPulseDecay);
            if (cutLightLoaded)
            {
                cutLight.setBeatPulse(lightPulse);
            }

            // 角色移动与动画
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

            // 系统轮询更新
            gameMap.update(playerCenter.x);
            lighting.update();
            rhythmBus.update(backgroundMusic);
            gameUI.update();

            const float musicTime =
                backgroundMusic.getPlayingOffset().asSeconds();

            // 镜头控制器更新
            cameraController.update(safeDeltaTime, musicTime, playerCenter);

            // 关键同步：同步角度并更新 4 轨道状态
            rhythmLine.setCameraAngle(cameraController.getCurrentAngle());
            rhythmLine.update(safeDeltaTime, musicTime);
        }

        // -------------------------------------------------------------
        // 🎨 渲染绘制 Graphics Rendering
        // -------------------------------------------------------------
        const sf::View gameView = cameraController.getView();
        window.clear(sf::Color::Black);

        sf::RenderStates cutLightStates = sf::RenderStates::Default;
        if (cutLightLoaded)
        {
            cutLightStates.shader = &cutLight.shader();
        }

        // 1. 绘制普通天空 (Game World View)
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

        // 2. 绘制月亮/太阳 (Default UI View 固定视口)
        window.setView(window.getDefaultView());
        lighting.drawMoon(window);

        // 3. 绘制场景与建筑 (支持 EdgeJitter 描边抖动模式)
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

        // 4. 绘制玩家角色 (Game World View)
        window.setView(gameView);
        window.draw(playerSprite);

        // 5. 绘制节奏判定线与音符 (使用包含镜头旋转的 gameView，确保视觉同步)
        window.setView(gameView);
        rhythmLine.draw(window);

        // 6. 绘制固定的顶部 UI (Default UI View - 固定于屏幕前端)
        window.setView(window.getDefaultView());
        gameUI.draw(window);

        window.display();
    }

    return 0;
}