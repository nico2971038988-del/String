#pragma once

#include <SFML/Audio/Music.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

// 音乐节点的类型标签。
// 它只描述音乐本身，不包含镜头、线条或玩家的具体行为。
enum class MusicCue {
    IntroAccent, // 前奏重音
    LayerChange, // 配器或音层变化
    Build,       // 蓄力、推进
    PreDrop,     // Drop 前的预备点
    Drop,        // 高能段正式进入
    PhraseTurn,  // 乐句转折
    Breakdown,   // 能量下降、进入间奏
    Fill,        // 过门重音
    Climax,      // 高潮重音
    Suona,
    Outro        // 尾声开始
};

// Music 模块向外广播的通用音乐事件。
// 各订阅者收到事件后，自行决定如何响应。
struct MusicRhythmEvent {
    float timeSeconds; // 事件在音乐中的时间，单位：秒
    MusicCue cue;      // 事件类型
    int direction;     // 音乐运动方向：-1、0、+1
    float intensity;   // 音乐强度：0.0～1.0
};

// 加载并播放音乐。
// music 由 main() 或 Game 类长期持有，本函数不会创建临时 sf::Music。
bool initializeMusic(sf::Music& music, const std::string& filePath);

class MusicRhythmEventBus {
public:
    using Listener = std::function<void(const MusicRhythmEvent&)>;

    // 注册事件订阅者。当前版本要求订阅者与事件总线生命周期一致。
    void subscribe(Listener listener);

    // 每帧调用：读取 sf::Music 当前播放时间并分发已跨过的事件。
    void update(const sf::Music& music);

    // 便于测试或接入其他音频系统的时间更新接口。
    void update(float nowSeconds);

    // 主动跳转音乐后调用，只同步游标，不补发被跳过的事件。
    void seek(float seconds);

    // 音乐从头播放前调用。
    void reset();

private:
    void broadcast(const MusicRhythmEvent& event);

    // 非 static 成员函数；声明与 .cpp 定义末尾都带 const。
    std::size_t firstEventAfter(float seconds) const;

    std::vector<Listener> listeners_; // 已注册的订阅者
    std::size_t nextEvent_ = 0;       // 下一个尚未广播的事件
    float previousSeconds_ = 0.0f;    // 上一帧的音乐时间
    bool initialized_ = false;        // 是否已建立时间基准
};