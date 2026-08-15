#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics.hpp>

#include <string>

enum class JudgementType {
    Perfect,
    Great,
    Miss
};

// 游戏屏幕 UI 的统一入口。
// 包含音频进度条、暂停控制以及暂停 UI Overlay 渲染。
class UI {
public:
    UI();

    bool loadProgressBar(
        const std::string& lineTexturePath,
        const std::string& sliderTexturePath
    );

    // 可选：加载用于 Pause 界面的字体
    bool loadFont(const std::string& fontPath);

    void bindMusic(sf::Music& music);
    void setProgressBarPosition(const sf::Vector2f& position);
    void setProgressBarSize(const sf::Vector2f& size);
    void setProgressSliderSize(const sf::Vector2f& size);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update();
    void draw(sf::RenderTarget& target);
    void showJudgement(JudgementType judgement);

    float musicProgress() const;
    bool isProgressBarDragging() const { return progressBarDragging_; }

    // --- 暂停功能接口 ---
    void togglePause();
    void pause();
    void resume();
    bool isPaused() const { return isPaused_; }

private:
    float progressFromMouse(const sf::Vector2f& mousePosition) const;
    bool progressBarContains(const sf::Vector2f& mousePosition) const;
    void seekMusic(float normalizedProgress);
    void updateProgressSlider(float normalizedProgress);

    // 绘制暂停界面 overlay
    void drawPauseOverlay(sf::RenderTarget& target);
    void drawJudgement(sf::RenderTarget& target);

    sf::Texture progressLineTexture_;
    sf::Texture progressSliderTexture_;
    sf::Sprite progressLineSprite_;
    sf::Sprite progressSliderSprite_;

    sf::Font font_;
    bool fontLoaded_ = false;
    sf::Text pauseText_;
    sf::Text judgementText_;
    sf::Clock judgementClock_;
    bool judgementVisible_ = false;
    sf::Color judgementBaseColor_{ sf::Color::White };
    float judgementDuration_ = 0.65f;

    sf::Music* music_ = nullptr;
    sf::Vector2f progressBarPosition_{ 0.f, 0.f };
    sf::Vector2f progressBarSize_{ 500.f, 8.f };
    bool progressBarLoaded_ = false;
    bool progressBarDragging_ = false;

    // 暂停状态标志
    bool isPaused_ = false;
};