// Source: Stage 5 AudioManager requirements (mock audio sink)
#ifndef MOCKS_AUDIO_AUDIOSINKMOCK_H_
#define MOCKS_AUDIO_AUDIOSINKMOCK_H_

#include <string>
#include <vector>

#include "Classes/Contract/Engine/AudioSink.h"

struct RecordedClip {
    std::string clip_id;
    bool loop = false;
    int handle = 0;
};

class AudioSinkMock : public IAudioSink {
public:
    int Play(const std::string& clip_id, bool loop = false) override {
        ++next_handle_;
        plays_.push_back({clip_id, loop, next_handle_});
        return next_handle_;
    }

    void Stop(int handle) override { stops_.push_back(handle); }

    void StopAll() override { stop_all_calls_++; }

    const std::vector<RecordedClip>& GetPlays() const { return plays_; }
    const std::vector<int>& GetStops() const { return stops_; }
    int GetStopAllCalls() const { return stop_all_calls_; }

    void Reset() {
        plays_.clear();
        stops_.clear();
        stop_all_calls_ = 0;
        next_handle_ = 0;
    }

private:
    std::vector<RecordedClip> plays_{};
    std::vector<int> stops_{};
    int stop_all_calls_ = 0;
    int next_handle_ = 0;
};

#endif  // MOCKS_AUDIO_AUDIOSINKMOCK_H_
