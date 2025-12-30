#ifndef CLASH_OF_CLANS_LOCAL_CLASSES_MAIN_APPDELEGATE_H_
#define CLASH_OF_CLANS_LOCAL_CLASSES_MAIN_APPDELEGATE_H_

#include "cocos2d.h"

class AppDelegate : private cocos2d::Application {
public:
    AppDelegate() = default;
    ~AppDelegate() override = default;

    bool applicationDidFinishLaunching() override;
    void applicationDidEnterBackground() override;
    void applicationWillEnterForeground() override;
};

#endif  // CLASH_OF_CLANS_LOCAL_CLASSES_MAIN_APPDELEGATE_H_
