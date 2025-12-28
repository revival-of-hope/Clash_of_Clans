// Source: Stage 5 AudioManager requirements (Cocos AudioEngine adapter)
#ifndef INTEGRATION_ENGINE_COCOSAUDIOSINK_H_
#define INTEGRATION_ENGINE_COCOSAUDIOSINK_H_

#include <string>

#include "Classes/Contract/Engine/AudioSink.h"

class CocosAudioSink : public IAudioSink {
public:
    int Play(const std::string& clip_id, bool loop = false) override;
    void Stop(int handle) override;
    void StopAll() override;
};

#endif  // INTEGRATION_ENGINE_COCOSAUDIOSINK_H_
