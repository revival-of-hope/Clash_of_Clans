// Source: Stage 5 AudioManager requirements (Audio sink interface)
#ifndef Contract_ENGINE_AUDIOSINK_H_
#define Contract_ENGINE_AUDIOSINK_H_

#include <string>

// Engine-agnostic audio sink for playback requests issued by AudioManager.
class IAudioSink {
public:
    virtual ~IAudioSink() = default;

    // Plays the requested clip identifier or path. Returns an opaque handle if supported,
    // otherwise returns a non-negative token unique per call.
    virtual int Play(const std::string& clip_id, bool loop = false) = 0;

    // Stops a previously started clip by handle; implementations may ignore unknown handles.
    virtual void Stop(int handle) = 0;

    // Stops all playing clips.
    virtual void StopAll() = 0;
};

#endif  // Contract_ENGINE_AUDIOSINK_H_
