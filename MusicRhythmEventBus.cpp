#include "MusicRhythmEventBus.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <utility>

namespace {

    // 音频播放位置允许出现的轻微回抖范围。
    // 小于或等于 10 ms 的回抖不会被误判为用户向后跳转。
    constexpr float RewindTolerance = 0.010f;

    // 音乐事件时间轴，必须按 timeSeconds 从小到大排列。
    // 每项依次为：触发时间、音乐事件类型、方向、强度。
    //
    // 注意：这里没有任何镜头角度或旋转时长。
    // 镜头系统收到这些通用事件后，应在自己的函数中决定如何旋转。
    constexpr std::array<MusicRhythmEvent, 14> Timeline{ {
    {  14.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 00:15
    {  25.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 00:26
    {  36.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 00:38
    {  47.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 00:49
    {  59.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 01:00
    {  71.000f, MusicCue::PhraseTurn, -1, 2.00f }, // 01:12
    {  82.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 01:23
    {  93.000f, MusicCue::PhraseTurn, -1, 2.00f }, // 01:35
    { 105.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 01:46
    { 116.000f, MusicCue::PhraseTurn, -1, 2.00f }, // 01:57
    { 138.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 02:19
    { 150.000f, MusicCue::PhraseTurn, -1, 2.00f }, // 02:31
    { 161.000f, MusicCue::PhraseTurn, +1, 2.00f }, // 02:42
    { 172.000f, MusicCue::PhraseTurn, -1, 2.00f }, // 02:53
} };


} // namespace

bool initializeMusic(sf::Music& music, const std::string& filePath) {
    // sf::Music 以流式方式打开长音频，不会一次性将整个文件载入内存。
    if (!music.openFromFile(filePath)) {
        std::cerr << "Failed to open music file: " << filePath << '\n';
        return false;
    }

    music.setVolume(60.0f);
    music.play();
    return true;
}

void MusicRhythmEventBus::subscribe(Listener listener) {
    listeners_.push_back(std::move(listener));
}

void MusicRhythmEventBus::update(const sf::Music& music) {
    update(music.getPlayingOffset().asSeconds());
}

void MusicRhythmEventBus::update(float nowSeconds) {
    nowSeconds = std::max(0.0f, nowSeconds);





    // 第一次调用只同步当前位置，不广播当前位置以前的历史事件。
    if (!initialized_) {
        seek(nowSeconds);
        return;
    }





    // 检测音乐重播或向后跳转。
    // seek() 重新定位事件游标，并且不会倒序广播旧事件。
    if (nowSeconds + RewindTolerance < previousSeconds_) {
        seek(nowSeconds);
        return;
    }

    // 使用“上一帧时间 < 事件时间 <= 当前帧时间”的跨越检测。
    // 因此不要求某一帧刚好落在事件时间点，掉帧时也不会漏事件。
    // 如果一帧跨过多个事件，将按照时间轴顺序依次广播。
    while (nextEvent_ < Timeline.size() &&
        Timeline[nextEvent_].timeSeconds <= nowSeconds) {
        if (Timeline[nextEvent_].timeSeconds > previousSeconds_) {

            const auto& event = Timeline[nextEvent_];

            std::cout
                << "[Music node "
                << (nextEvent_ + 1)
                << "/"
                << Timeline.size()
                << "] time="
                << event.timeSeconds
                << "s, direction="
                << event.direction
                << ", intensity="
                << event.intensity
                << std::endl;


            broadcast(Timeline[nextEvent_]);
        }
        ++nextEvent_;
    }

    previousSeconds_ = nowSeconds;
}

void MusicRhythmEventBus::seek(float seconds) {
    previousSeconds_ = std::max(0.0f, seconds);
    nextEvent_ = firstEventAfter(previousSeconds_);
    initialized_ = true;
}

void MusicRhythmEventBus::reset() {
    previousSeconds_ = 0.0f;
    nextEvent_ = firstEventAfter(0.0f);
    initialized_ = true;
}

std::size_t MusicRhythmEventBus::firstEventAfter(float seconds) const {
    // upper_bound 返回第一个严格晚于 seconds 的事件。
    // 这样 seek 到某个事件的准确时间时，不会再次触发该事件。
    const auto iterator = std::upper_bound(
        Timeline.begin(),
        Timeline.end(),
        seconds,
        [](float value, const MusicRhythmEvent& event) {
            return value < event.timeSeconds;
        });

    return static_cast<std::size_t>(iterator - Timeline.begin());
}

void MusicRhythmEventBus::broadcast(const MusicRhythmEvent& event) {
    // 同一事件按订阅者的注册顺序同步分发。
    for (const auto& listener : listeners_) {
        listener(event);
    }
}