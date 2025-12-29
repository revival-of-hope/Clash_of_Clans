#include "Main/AppDelegate.h"

#include "Classes/Integration/GameServices.h"

bool AppDelegate::applicationDidFinishLaunching() {
    auto* director = cocos2d::Director::getInstance();
    auto* glview = director->getOpenGLView();
    if (!glview) {
        // TODO(DevC): Configure GLView in platform bootstrap if not initialized.
        return false;
    }

    director->setAnimationInterval(1.0f / 60.0f);

    Integration::SceneFlowService* scene_flow = Integration::ResolveSceneFlowService();
    if (!scene_flow) {
        return false;
    }

    cocos2d::Scene* boot_scene = scene_flow->CreateBootScene();
    if (!boot_scene) {
        return false;
    }

    director->runWithScene(boot_scene);
    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    cocos2d::Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
    cocos2d::Director::getInstance()->startAnimation();
}
