#include "Classes/Integration/Engine/CocosAudioSink.h"

#if USE_COCOS_ENGINE
#include "audio/include/AudioEngine.h"

int CocosAudioSink::Play(const std::string& clip_id, bool loop) {
    return cocos2d::experimental::AudioEngine::play2d(clip_id, loop);
}

void CocosAudioSink::Stop(int handle) {
    cocos2d::experimental::AudioEngine::stop(handle);
}

void CocosAudioSink::StopAll() {
    cocos2d::experimental::AudioEngine::stopAll();
}

#else

int CocosAudioSink::Play(const std::string& /*clip_id*/, bool /*loop*/) {
    return -1;
}

void CocosAudioSink::Stop(int /*handle*/) {}

void CocosAudioSink::StopAll() {}

#endif
