#ifndef CLASSES_MANAGERS_LEVELMANAGER_H_
#define CLASSES_MANAGERS_LEVELMANAGER_H_

#include <string>

// Stores selected map information for launching battles.
class LevelManager {
 public:
  static LevelManager* GetInstance() {
    static LevelManager instance;
    return &instance;
  }

  void SelectMapA() { selected_map_path_ = kMapAPath; }

  void SelectMapB() { selected_map_path_ = kMapBPath; }

  std::string GetSelectedMapPath() const { return selected_map_path_; }

  std::string GetMapAPath() const { return kMapAPath; }

  std::string GetMapBPath() const { return kMapBPath; }

  void SetSeed(int seed) { seed_ = seed; }

  int GetSeed() const { return seed_; }

  void ResetSelection() {
    selected_map_path_ = kMapAPath;
    seed_ = 0;
  }

 private:
  LevelManager() = default;

  std::string selected_map_path_ = kMapAPath;
  int seed_ = 0;

  static constexpr const char* kMapAPath = "maps/map_a.tmx";
  static constexpr const char* kMapBPath = "maps/map_b.tmx";
};

#endif  // CLASSES_MANAGERS_LEVELMANAGER_H_
