#ifndef MOCKS_COCOSSHIM_COCOSSHIM_H_
#define MOCKS_COCOSSHIM_COCOSSHIM_H_

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define CREATE_FUNC(__TYPE__)                          \
    static __TYPE__* create() {                        \
        auto* obj = new __TYPE__();                    \
        if (obj && obj->init()) {                      \
            return obj;                                \
        }                                              \
        delete obj;                                    \
        return nullptr;                                \
    }

namespace cocos2d {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
    Vec2() = default;
    Vec2(float x_value, float y_value) : x(x_value), y(y_value) {}
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    Rect() = default;
    Rect(float x_value, float y_value, float width_value, float height_value)
        : x(x_value), y(y_value), width(width_value), height(height_value) {}
};

class Node {
public:
    static Node* create() { return new Node(); }
    virtual ~Node() = default;

    virtual bool init() { return true; }
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void update(float /*dt*/) {}

    void addChild(Node* /*child*/, int /*z_order*/ = 0) {}

    void setPosition(const Vec2& position) { position_ = position; }
    const Vec2& getPosition() const { return position_; }

  private:
      Vec2 position_;
  };

  class Scene : public Node {};
  class Sprite : public Node {};
  class TMXTiledMap : public Node {};
  class Layer : public Node {};
  class DrawNode : public Node {};
class Label : public Node {
public:
    static Label* createWithSystemFont(const std::string& /*text*/,
                                       const std::string& /*font*/,
                                       int /*font_size*/) {
        return new Label();
    }

    void setString(const std::string& text) { text_ = text; }

private:
    std::string text_;
};

class StringUtils {
public:
    static std::string format(const char* format_string, ...) {
        char buffer[256];
        va_list args;
        va_start(args, format_string);
        std::vsnprintf(buffer, sizeof(buffer), format_string, args);
        va_end(args);
        return std::string(buffer);
    }
};

inline void log(const std::string& /*message*/) {}
inline void log(const char* /*message*/) {}

template <class T>
class Vector {
public:
    Vector() = default;

    void pushBack(const T& value) { data_.push_back(value); }
    std::size_t size() const { return data_.size(); }

    typename std::vector<T>::iterator begin() { return data_.begin(); }
    typename std::vector<T>::iterator end() { return data_.end(); }
    typename std::vector<T>::const_iterator begin() const { return data_.begin(); }
    typename std::vector<T>::const_iterator end() const { return data_.end(); }

    T& operator[](std::size_t index) { return data_[index]; }
    const T& operator[](std::size_t index) const { return data_[index]; }

private:
    std::vector<T> data_;
};

}  // namespace cocos2d

namespace cocos2d {

class GLView {};

class Director {
 public:
  static Director* getInstance() {
    static Director instance;
    return &instance;
  }

  GLView* getOpenGLView() const { return gl_view_.get(); }
  void setOpenGLView(std::unique_ptr<GLView> view) { gl_view_ = std::move(view); }
  void setAnimationInterval(double /*interval*/) {}
  void runWithScene(Scene* /*scene*/) {}
  void replaceScene(Scene* /*scene*/) {}
  void stopAnimation() {}
  void startAnimation() {}

 private:
  std::unique_ptr<GLView> gl_view_ = std::make_unique<GLView>();
};

class Application {
 public:
  Application() = default;
  virtual ~Application() = default;

  virtual bool applicationDidFinishLaunching() { return true; }
  virtual void applicationDidEnterBackground() {}
  virtual void applicationWillEnterForeground() {}
};

}  // namespace cocos2d

#endif  // MOCKS_COCOSSHIM_COCOSSHIM_H_
