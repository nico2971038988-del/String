#include "UI.hpp"

#include <algorithm>
#include <cmath>

UI::UI()
    : progressLineSprite_(progressLineTexture_),
    progressSliderSprite_(progressSliderTexture_),
    pauseText_(font_),
    judgementText_(font_),
    scoreText_(font_),
    comboText_(font_)
{
}

bool UI::loadProgressBar(const std::string& lineTexturePath,
    const std::string& sliderTexturePath) {
    if (!progressLineTexture_.loadFromFile(lineTexturePath) ||
        !progressSliderTexture_.loadFromFile(sliderTexturePath)) {
        progressBarLoaded_ = false;
        return false;
    }

    progressLineTexture_.setSmooth(true);
    progressSliderTexture_.setSmooth(true);
    progressLineSprite_.setTexture(progressLineTexture_, true);
    progressSliderSprite_.setTexture(progressSliderTexture_, true);

    const sf::Vector2u sliderSize = progressSliderTexture_.getSize();

    progressSliderSprite_.setOrigin(sf::Vector2f{
        static_cast<float>(sliderSize.x) * 0.5f,
        static_cast<float>(sliderSize.y) * 0.5f
        });

    progressBarLoaded_ = true;
    setProgressBarSize(progressBarSize_);
    setProgressBarPosition(progressBarPosition_);
    updateProgressSlider(0.f);
    return true;
}

bool UI::loadFont(const std::string& fontPath) {
    if (font_.openFromFile(fontPath)) {
        fontLoaded_ = true;
        pauseText_.setString("PAUSED");
        pauseText_.setCharacterSize(48);
        pauseText_.setFillColor(sf::Color::White);
        pauseText_.setStyle(sf::Text::Bold);

        sf::FloatRect bounds = pauseText_.getLocalBounds();
        pauseText_.setOrigin({ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f });

        judgementText_.setCharacterSize(38);
        judgementText_.setStyle(sf::Text::Bold);
        judgementText_.setOutlineColor(sf::Color(92, 58, 25, 230));
        judgementText_.setOutlineThickness(1.5f);

        // 顶部计分 UI，沿用判定文字的暖金配色。
        scoreText_.setCharacterSize(26);
        scoreText_.setStyle(sf::Text::Bold);
        scoreText_.setFillColor(sf::Color(245, 220, 170));
        scoreText_.setOutlineColor(sf::Color(45, 28, 15, 230));
        scoreText_.setOutlineThickness(1.5f);

        comboText_.setCharacterSize(26);
        comboText_.setStyle(sf::Text::Bold);
        comboText_.setFillColor(sf::Color(255, 232, 170));
        comboText_.setOutlineColor(sf::Color(45, 28, 15, 230));
        comboText_.setOutlineThickness(1.5f);
        refreshScoreText();
        return true;
    }
    fontLoaded_ = false;
    return false;
}

void UI::bindMusic(sf::Music& music) {
    music_ = &music;
    update();
}

void UI::setProgressBarPosition(const sf::Vector2f& position) {
    progressBarPosition_ = position;
    progressLineSprite_.setPosition(progressBarPosition_);
    updateProgressSlider(musicProgress());
}

void UI::setProgressBarSize(const sf::Vector2f& size) {
    progressBarSize_.x = std::max(1.f, size.x);
    progressBarSize_.y = std::max(1.f, size.y);

    if (!progressBarLoaded_) {
        return;
    }

    const sf::Vector2u textureSize = progressLineTexture_.getSize();
    progressLineSprite_.setScale(sf::Vector2f{
        progressBarSize_.x / static_cast<float>(textureSize.x),
        progressBarSize_.y / static_cast<float>(textureSize.y)
        });
    updateProgressSlider(musicProgress());
}

void UI::setProgressSliderSize(const sf::Vector2f& size) {
    if (!progressBarLoaded_) {
        return;
    }

    const sf::Vector2u textureSize = progressSliderTexture_.getSize();
    if (textureSize.x == 0 || textureSize.y == 0) return;

    progressSliderSprite_.setScale(sf::Vector2f{
        size.x / static_cast<float>(textureSize.x),
        size.y / static_cast<float>(textureSize.y)
        });
}

// --- 暂停控制实现 ---
void UI::togglePause() {
    if (isPaused_) {
        resume();
    }
    else {
        pause();
    }
}

void UI::pause() {
    isPaused_ = true;
    if (music_ != nullptr) {
        music_->pause();
    }
}

void UI::resume() {
    isPaused_ = false;
    if (music_ != nullptr) {
        music_->play();
    }
}

void UI::handleEvent(const sf::Event& event,
    const sf::RenderWindow& window) {

    // 监听键盘按 Tab 切换暂停
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Tab) {
            togglePause();
            return;
        }
    }

    if (!progressBarLoaded_ || music_ == nullptr) {
        return;
    }

    // 鼠标左键按下
    if (const auto* pressed =
        event.getIf<sf::Event::MouseButtonPressed>();
        pressed != nullptr &&
        pressed->button == sf::Mouse::Button::Left) {

        const sf::Vector2f mouse = window.mapPixelToCoords(
            pressed->position,
            window.getDefaultView()
        );

        if (progressBarContains(mouse)) {
            progressBarDragging_ = true;
            seekMusic(progressFromMouse(mouse));
        }
    }

    // 鼠标移动
    else if (const auto* moved =
        event.getIf<sf::Event::MouseMoved>();
        moved != nullptr && progressBarDragging_) {

        const sf::Vector2f mouse = window.mapPixelToCoords(
            moved->position,
            window.getDefaultView()
        );

        seekMusic(progressFromMouse(mouse));
    }

    // 鼠标左键松开
    else if (const auto* released =
        event.getIf<sf::Event::MouseButtonReleased>();
        released != nullptr &&
        released->button == sf::Mouse::Button::Left) {

        if (progressBarDragging_) {
            const sf::Vector2f mouse = window.mapPixelToCoords(
                released->position,
                window.getDefaultView()
            );

            seekMusic(progressFromMouse(mouse));
        }

        progressBarDragging_ = false;
    }
}

void UI::update() {
    if (!progressBarDragging_) {
        updateProgressSlider(musicProgress());
    }
}

void UI::showJudgement(JudgementType judgement, int count) {
    count = std::max(1, count);

    // PERFECT +1000，GREAT +600，MISS -300。
    // 连续命中时，每次额外获得 (Combo - 1) * 25 分，最高 2500 分。
    for (int i = 0; i < count; ++i) {
        if (judgement == JudgementType::Miss) {
            score_ = std::max<std::int64_t>(0, score_ - 300);
            combo_ = 0;
        }
        else {
            ++combo_;
            const int baseScore = judgement == JudgementType::Perfect ? 1000 : 600;
            const int comboBonus = std::min((combo_ - 1) * 25, 2500);
            score_ += baseScore + comboBonus;
        }
    }
    refreshScoreText();

    if (!fontLoaded_) return;

    switch (judgement) {
    case JudgementType::Perfect:
        judgementText_.setString("PERFECT");
        judgementBaseColor_ = sf::Color(255, 232, 170);
        break;
    case JudgementType::Great:
        judgementText_.setString("GREAT");
        judgementBaseColor_ = sf::Color(242, 207, 145);
        break;
    case JudgementType::Miss:
        judgementText_.setString("MISS");
        judgementBaseColor_ = sf::Color(205, 168, 105);
        break;
    }

    const sf::FloatRect bounds = judgementText_.getLocalBounds();
    judgementText_.setOrigin({
        bounds.position.x + bounds.size.x * 0.5f,
        bounds.position.y + bounds.size.y * 0.5f
        });
    judgementVisible_ = true;
    judgementClock_.restart();
}

void UI::refreshScoreText() {
    scoreText_.setString("SCORE  " + std::to_string(score_));
    comboText_.setString("COMBO  " + std::to_string(combo_));
}

void UI::drawScore(sf::RenderTarget& target) {
    if (!fontLoaded_) return;

    // Main.cpp 绘制 UI 前已经切换到 DefaultView，直接使用固定屏幕坐标。
    // 每帧重新写入字符串，保证即使字体稍后才加载也一定有显示内容。
    refreshScoreText();
    scoreText_.setOrigin({ 0.f, 0.f });
    scoreText_.setScale({ 1.f, 1.f });
    scoreText_.setPosition({ 28.f, 48.f });

    const sf::FloatRect comboBounds = comboText_.getLocalBounds();
    comboText_.setOrigin({ comboBounds.position.x + comboBounds.size.x, 0.f });
    comboText_.setScale({ 1.f, 1.f });
    comboText_.setPosition({ 1252.f, 48.f });

    target.draw(scoreText_);
    target.draw(comboText_);
}

void UI::drawJudgement(sf::RenderTarget& target) {
    if (!fontLoaded_ || !judgementVisible_) return;

    const float elapsed = judgementClock_.getElapsedTime().asSeconds();
    if (elapsed >= judgementDuration_) {
        judgementVisible_ = false;
        return;
    }

    const float progress = std::clamp(elapsed / judgementDuration_, 0.f, 1.f);
    const float fade = progress < 0.55f
        ? 1.f
        : 1.f - (progress - 0.55f) / 0.45f;
    const float popScale = 1.f + 0.18f * std::exp(-elapsed * 12.f);

    sf::Color color = judgementBaseColor_;
    color.a = static_cast<std::uint8_t>(255.f * std::clamp(fade, 0.f, 1.f));
    judgementText_.setFillColor(color);
    judgementText_.setScale({ popScale, popScale });

    const sf::View& view = target.getView();
    const sf::Vector2f viewCenter = view.getCenter();
    const sf::Vector2f viewSize = view.getSize();
    judgementText_.setPosition({
        viewCenter.x,
        viewCenter.y + viewSize.y * 0.5f - 80.f
        });
    target.draw(judgementText_);
}

void UI::drawPauseOverlay(sf::RenderTarget& target) {
    sf::Vector2f targetSize = target.getView().getSize();

    // 1. 全屏半透明黑色背景遮罩
    sf::RectangleShape overlay(targetSize);
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    target.draw(overlay);

    sf::Vector2f center = targetSize * 0.5f;

    // 2. 如果加载了字体，使用文字绘制；若没有，绘制矢量暂停图形 (||)
    if (fontLoaded_) {
        pauseText_.setPosition(center);
        target.draw(pauseText_);
    }
    else {
        // 绘制矢量暂停图标双竖杠 ||
        constexpr float barWidth = 14.f;
        constexpr float barHeight = 48.f;
        constexpr float barSpacing = 16.f;

        sf::RectangleShape bar1({ barWidth, barHeight });
        sf::RectangleShape bar2({ barWidth, barHeight });

        bar1.setFillColor(sf::Color(240, 240, 240, 220));
        bar2.setFillColor(sf::Color(240, 240, 240, 220));

        bar1.setOrigin({ barWidth / 2.f, barHeight / 2.f });
        bar2.setOrigin({ barWidth / 2.f, barHeight / 2.f });

        bar1.setPosition({ center.x - barSpacing / 2.f, center.y });
        bar2.setPosition({ center.x + barSpacing / 2.f, center.y });

        target.draw(bar1);
        target.draw(bar2);
    }
}

void UI::draw(sf::RenderTarget& target) {
    // 1. 绘制进度条 UI
    if (progressBarLoaded_) {
        target.draw(progressLineSprite_);
        target.draw(progressSliderSprite_);
    }

    // 2. 绘制屏幕下方的判定提示。
    drawJudgement(target);

    // 3. 最后绘制顶部总分与 Combo，防止被其他 UI 覆盖。
    drawScore(target);

    // 4. 若处于暂停状态，绘制暂停 UI Overlay
    if (isPaused_) {
        drawPauseOverlay(target);
    }
}

float UI::musicProgress() const {
    if (music_ == nullptr) {
        return 0.f;
    }

    const float duration = music_->getDuration().asSeconds();
    if (!std::isfinite(duration) || duration <= 0.f) {
        return 0.f;
    }

    return std::clamp(music_->getPlayingOffset().asSeconds() / duration,
        0.f, 1.f);
}

float UI::progressFromMouse(const sf::Vector2f& mousePosition) const {
    return std::clamp(
        (mousePosition.x - progressBarPosition_.x) / progressBarSize_.x,
        0.f, 1.f);
}

bool UI::progressBarContains(const sf::Vector2f& mousePosition) const {
    const float sliderHalfHeight =
        progressSliderSprite_.getGlobalBounds().size.y * 0.5f;
    const float hitPadding = std::max(12.f, sliderHalfHeight);

    return mousePosition.x >= progressBarPosition_.x - hitPadding &&
        mousePosition.x <=
        progressBarPosition_.x + progressBarSize_.x + hitPadding &&
        mousePosition.y >= progressBarPosition_.y - hitPadding &&
        mousePosition.y <=
        progressBarPosition_.y + progressBarSize_.y + hitPadding;
}

void UI::seekMusic(float normalizedProgress) {
    if (music_ == nullptr) {
        return;
    }

    const float duration = music_->getDuration().asSeconds();
    if (!std::isfinite(duration) || duration <= 0.f) {
        return;
    }

    normalizedProgress = std::clamp(normalizedProgress, 0.f, 1.f);
    music_->setPlayingOffset(sf::seconds(duration * normalizedProgress));
    updateProgressSlider(normalizedProgress);
}

void UI::updateProgressSlider(float normalizedProgress) {
    normalizedProgress = std::clamp(normalizedProgress, 0.f, 1.f);
    progressSliderSprite_.setPosition(sf::Vector2f{
     progressBarPosition_.x +
         progressBarSize_.x * normalizedProgress,

     progressBarPosition_.y +
         progressBarSize_.y * 0.5f
        });
}