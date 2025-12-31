#include "Classes/Managers/AudioManager/AudioManager.h"

namespace {

struct ClipMapping {
    const char* key;
    const char* clip_id;
    bool loop = false;

    constexpr ClipMapping(const char* in_key, const char* in_clip_id, bool in_loop)
        : key(in_key), clip_id(in_clip_id), loop(in_loop) {}
};

const ClipMapping kClipMappings[] = {
        {"menu_music", "Resources/music/Background Music/Home music 1.mp3", true},
        {"battle_start", "Resources/music/UI effects/start_up.mp3", false},
        {"battle_bgm", "Resources/music/Background Music/shorter part2.mp3", true},
        {"battle_end", "Resources/music/UI effects/builder-base-combat-end.mp3", false},
        {"ui_click", "Resources/music/UI effects/ui_click.mp3", false},
        {"projectile_fired", "Resources/music/Combat effects/archer tower.mp3", false},
        {"projectile_hit", "Resources/music/Combat effects/arrow-hit.mp3", false},
        {"entity_destroyed_building", "Resources/music/Combat effects/building destroyed.mp3", false},
        {"entity_destroyed_unit", "Resources/music/Combat effects/barbarian-death-cry.mp3", false},
};

const ClipMapping* LookupClip(const std::string& key) {
    for (const auto& mapping : kClipMappings) {
        if (key == mapping.key) {
            return &mapping;
        }
    }
    return nullptr;
}

}  // namespace

AudioManager::AudioManager(Gameplay::GameEventManager* event_manager, IAudioSink* sink)
        : event_manager_(event_manager), sink_(sink) {
    if (event_manager_) {
        event_manager_->AddListener(this);
    }
}

AudioManager::~AudioManager() {
    if (event_manager_) {
        event_manager_->RemoveListener(this);
    }
}

void AudioManager::OnProjectileFired(const Gameplay::ProjectileEvent& /*evt*/) {
    PlayMappedClip("projectile_fired");
}

void AudioManager::OnProjectileHit(const Gameplay::ProjectileHitEvent& /*evt*/) {
    PlayMappedClip("projectile_hit");
}

void AudioManager::OnBattleStarted(const Gameplay::BattleStartEvent& /*evt*/) {
    PlayMappedClip("battle_start");
    StartLoopedClip("battle_bgm", &battle_bgm_handle_);
}

void AudioManager::OnBattleEnded(const Gameplay::BattleEndEvent& /*evt*/) {
    StopHandle(&battle_bgm_handle_);
    PlayMappedClip("battle_end");
}

void AudioManager::OnEntityDestroyed(const Gameplay::EntityDestroyEvent& evt) {
    if (evt.is_building) {
        PlayMappedClip("entity_destroyed_building");
        return;
    }
    PlayMappedClip("entity_destroyed_unit");
}

void AudioManager::PlayMenuMusic() {
    StartLoopedClip("menu_music", &menu_music_handle_);
}

void AudioManager::PlayUiClick() {
    PlayMappedClip("ui_click");
}

int AudioManager::StartLoopedClip(const std::string& key, int* handle) {
    if (!sink_) {
        return 0;
    }
    const ClipMapping* mapping = LookupClip(key);
    if (!mapping) {
        return 0;
    }
    StopHandle(handle);
    if (!handle) {
        return 0;
    }
    *handle = sink_->Play(mapping->clip_id, true);
    return *handle;
}

void AudioManager::StopHandle(int* handle) {
    if (!sink_ || !handle) {
        return;
    }
    if (*handle <= 0) {
        return;
    }
    sink_->Stop(*handle);
    *handle = 0;
}

int AudioManager::PlayMappedClip(const std::string& key, bool loop) {
    if (!sink_) {
        return 0;
    }
    const ClipMapping* mapping = LookupClip(key);
    if (!mapping) {
        return 0;
    }
    return sink_->Play(mapping->clip_id, loop || mapping->loop);
}
