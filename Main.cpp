#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>

#include "CameraController.hpp"
#include "Lighting.hpp"
#include "Map.hpp"
#include "MusicRhythmEventBus.hpp"
#include "PlayerAssets.hpp"
#include "RhythmLine.hpp"

int main()
{
    // =====================================================
    // 1. 窗口设置
    // =====================================================

    constexpr unsigned int windowWidth = 1280;
    constexpr unsigned int windowHeight = 720;

    constexpr float playerSpeed = 200.f;
    constexpr float playerScale = 0.15f;

    constexpr float screenCenterX =
        static_cast<float>(windowWidth) / 2.f;

    constexpr float screenCenterY =
        static_cast<float>(windowHeight) / 2.f;

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode({
            windowWidth,
            windowHeight
            }),
        "My Game",
        sf::Style::Titlebar | sf::Style::Close,
        sf::State::Windowed,
        settings
    );

    window.setFramerateLimit(60);

    // =====================================================
    // 2. 加载玩家走路动画
    // =====================================================

    PlayerTextures playerTextures;

    if (!loadWalkTextures(playerTextures) ||
        playerTextures.walkFrames.empty())
    {
        return 1;
    }

    sf::Sprite playerSprite(
        playerTextures.walkFrames[0]
    );

    playerSprite.setScale({
        playerScale,
        playerScale
        });

    playerSprite.setPosition({
        100.f,
        480.f
        });

    // =====================================================
    // 3. 玩家动画设置
    // =====================================================

    std::size_t walkFrameIndex = 0;

    // 每张动画图片显示0.15秒
    constexpr float walkFrameDuration = 0.15f;

    sf::Clock animationClock;

    // =====================================================
    // 4. 加载地图
    // =====================================================

    Map gameMap;

    if (!gameMap.load())
    {
        return 1;
    }

    // =====================================================
    // 5. 加载光照
    // =====================================================

    Lighting lighting;

    if (!lighting.load())
    {
        return 1;
    }

    // =====================================================
    // 6. 加载音乐和节奏事件
    // =====================================================

    sf::Music backgroundMusic;
    MusicRhythmEventBus rhythmBus;

    if (!initializeMusic(
        backgroundMusic,
        "Assets/Arts/Audio/1.ogg"))
    {
        return 1;
    }

    rhythmBus.reset();

    // 如果 initializeMusic() 只负责加载音乐，
    // 没有在函数内部播放音乐，则启用下一行：
    // backgroundMusic.play();

    // =====================================================
    // 7. 创建节奏线和摄像机
    // =====================================================

    RhythmLine rhythmLine;

    CameraController cameraController(
        {
            screenCenterX,
            screenCenterY
        },
        {
            static_cast<float>(windowWidth),
            static_cast<float>(windowHeight)
        }
        );

    // 摄像机和节奏线共同接收音乐段落事件
    rhythmBus.subscribe(
        [&cameraController, &rhythmLine](
            const MusicRhythmEvent& event)
        {
            cameraController.onMusicEvent(event);
           
        }
    );

    sf::Clock frameClock;

    // =====================================================
    // 8. 游戏主循环
    // =====================================================

    while (window.isOpen())
    {
        const float deltaTime =
            frameClock.restart().asSeconds();

        // 避免卡顿后角色突然移动很远
        const float safeDeltaTime =
            std::min(deltaTime, 0.1f);

        // -------------------------------------------------
        // 8.1 处理窗口事件
        // -------------------------------------------------

        while (const std::optional event =
            window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (const auto* keyPressed =
                event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code ==
                    sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
            }
        }

        // -------------------------------------------------
        // 8.2 玩家自动向右移动
        // -------------------------------------------------

        const sf::Vector2f movement{
            playerSpeed * safeDeltaTime,
            0.f
        };

        playerSprite.move(movement);

        // -------------------------------------------------
        // 8.3 在run1和run2之间循环切换
        // -------------------------------------------------

        if (animationClock
            .getElapsedTime()
            .asSeconds() >= walkFrameDuration)
        {
            walkFrameIndex =
                (walkFrameIndex + 1) %
                playerTextures.walkFrames.size();

            playerSprite.setTexture(
                playerTextures.walkFrames[
                    walkFrameIndex
                ],
                true
            );

            animationClock.restart();
        }

        // -------------------------------------------------
        // 8.4 限制玩家左边界
        // -------------------------------------------------

        if (playerSprite.getPosition().x < 0.f)
        {
            playerSprite.setPosition({
                0.f,
                playerSprite.getPosition().y
                });
        }

        // -------------------------------------------------
        // 8.5 计算玩家中心位置
        // -------------------------------------------------

        const sf::FloatRect playerBounds =
            playerSprite.getGlobalBounds();

        const sf::Vector2f playerCenter{
            playerBounds.position.x +
                playerBounds.size.x / 2.f,

            playerBounds.position.y +
                playerBounds.size.y / 2.f
        };

        // -------------------------------------------------
        // 8.6 更新地图和光照
        // -------------------------------------------------

        gameMap.update(playerCenter.x);

        // 月亮和PNG光晕固定在屏幕上，
        // 不再接收玩家位置
        lighting.update();

        // -------------------------------------------------
        // 8.7 更新音乐节奏事件
        // -------------------------------------------------

        rhythmBus.update(backgroundMusic);

        const float musicTime =
            backgroundMusic
            .getPlayingOffset()
            .asSeconds();

        // 根据BPM和音乐播放位置更新四周节奏线
        rhythmLine.update(
            safeDeltaTime,
            musicTime
        );

        // -------------------------------------------------
        // 8.8 更新摄像机
        // -------------------------------------------------

        cameraController.update(
            safeDeltaTime,
            musicTime,
            playerCenter
        );

        const sf::View gameView =
            cameraController.getView();

        // -------------------------------------------------
        // 8.9 绘制
        // -------------------------------------------------

        window.clear(sf::Color::Black);

        // -------------------------------------------------
        // 先绘制固定在屏幕背景中的月亮和PNG光晕
        // -------------------------------------------------
        gameMap.drawSky(window);

        window.setView(
            window.getDefaultView()
        );

        lighting.drawMoon(window);

        // -------------------------------------------------
        // 绘制游戏世界
        // -------------------------------------------------

        window.setView(gameView);

        gameMap.draw(window);

        // 如果Lighting::draw()负责绘制黑暗遮罩，
        // 则保留这个调用
        lighting.draw(window);

        // 绘制玩家
        window.draw(playerSprite);

        // -------------------------------------------------
        // 最后绘制屏幕空间中的节奏线
        // -------------------------------------------------

        window.setView(
            window.getDefaultView()
        );

        rhythmLine.draw(window);

        window.display();
    }

    return 0;
}