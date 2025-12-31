// Source: Stage 5 AudioManager (event → sound mapping) plan
#ifndef MANAGERS_AUDIOMANAGER_AUDIOMANAGER_H_
#define MANAGERS_AUDIOMANAGER_AUDIOMANAGER_H_

#include <string>

#include "Classes/Contract/Engine/AudioSink.h"
#include "Classes/Contract/Gameplay/GameEvents.h"

class AudioManager : public Gameplay::IGameEventListener {
public:
    AudioManager(Gameplay::GameEventManager* event_manager, IAudioSink* sink);
    ~AudioManager() override;

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void OnProjectileFired(const Gameplay::ProjectileEvent& evt) override;
    void OnProjectileHit(const Gameplay::ProjectileHitEvent& evt) override;
    void OnBattleStarted(const Gameplay::BattleStartEvent& evt) override;
    void OnBattleEnded(const Gameplay::BattleEndEvent& evt) override;
    void OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) override;

    void PlayMenuMusic();
    void PlayUiClick();

private:
    int PlayMappedClip(const std::string& key, bool loop = false);
    int StartLoopedClip(const std::string& key, int* handle);
    void StopHandle(int* handle);

    Gameplay::GameEventManager* event_manager_ = nullptr;
    IAudioSink* sink_ = nullptr;
    int menu_music_handle_ = 0;
    int battle_bgm_handle_ = 0;
};

#endif  // MANAGERS_AUDIOMANAGER_AUDIOMANAGER_H_
