#include "UI.hpp"

#include <algorithm>
#include <cmath>

UI::UI()
    : progressLineSprite_(progressLineTexture_),
    progressSliderSprite_(progressSliderTexture_),
    pauseText_(font_) // 👈 显式传入 font_ 进行初始化，解决 SFML 3 没有默认构造的问题
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

    // 2. 若处于暂停状态，绘制暂停 UI Overlay
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