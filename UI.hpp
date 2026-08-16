#pragma once

// 完整 UI 接口：本文件必须在工程中命名为 UI.hpp。

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics.hpp>

#include <cstdint>
#include <string>

enum class JudgementType
{
    Perfect,
    Great,
    Miss
};

class UI
{
public:
    UI();

    bool loadProgressBar(
        const std::string& lineTexturePath,
        const std::string& sliderTexturePath);
    bool loadFont(const std::string& fontPath);

    void bindMusic(sf::Music& music);
    void setSongName(const std::string& songName);
    void setProgressBarPosition(const sf::Vector2f& position);
    void setProgressBarSize(const sf::Vector2f& size);
    void setProgressSliderSize(const sf::Vector2f& size);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update();
    void draw(sf::RenderTarget& target);

    // 显示判定并结算分数；count 可用于一次结算多个超时 Miss。
    void showJudgement(JudgementType judgement, int count = 1);

    std::int64_t score() const { return score_; }
    int combo() const { return combo_; }
    bool isResultVisible() const { return resultVisible_; }

    float musicProgress() const;
    bool isProgressBarDragging() const { return progressBarDragging_; }

    void togglePause();
    void pause();
    void resume();
    bool isPaused() const { return isPaused_; }

private:
    float progressFromMouse(const sf::Vector2f& mousePosition) const;
    bool progressBarContains(const sf::Vector2f& mousePosition) const;
    void seekMusic(float normalizedProgress);
    void updateProgressSlider(float normalizedProgress);

    void drawPauseOverlay(sf::RenderTarget& target);
    void drawJudgement(sf::RenderTarget& target);
    void drawScore(sf::RenderTarget& target);
    void drawResultScreen(sf::RenderTarget& target);
    void refreshScoreText();
    static std::string formatTime(float seconds);

    sf::Texture progressLineTexture_;
    sf::Texture progressSliderTexture_;
    sf::Sprite progressLineSprite_;
    sf::Sprite progressSliderSprite_;

    sf::Font font_;
    bool fontLoaded_ = false;
    sf::Text pauseText_;
    sf::Text judgementText_;
    sf::Text scoreText_;
    sf::Text comboText_;
    sf::Text resultTitleText_;
    sf::Text resultSongText_;
    sf::Text resultScoreText_;
    sf::Text resultTimeText_;

    sf::Clock judgementClock_;
    bool judgementVisible_ = false;
    sf::Color judgementBaseColor_{ sf::Color::White };
    float judgementDuration_ = 0.65f;

    std::int64_t score_ = 0;
    int combo_ = 0;

    std::string songName_{ "UNKNOWN SONG" };
    bool musicHasStarted_ = false;
    bool resultVisible_ = false;
    float furthestMusicOffset_ = 0.f;

    sf::Music* music_ = nullptr;
    sf::Vector2f progressBarPosition_{ 0.f, 0.f };
    sf::Vector2f progressBarSize_{ 500.f, 8.f };
    bool progressBarLoaded_ = false;
    bool progressBarDragging_ = false;

    bool isPaused_ = false;
};