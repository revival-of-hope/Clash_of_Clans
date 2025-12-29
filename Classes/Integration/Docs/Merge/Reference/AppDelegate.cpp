#include "Main/AppDelegate.h"

#include "Classes/Integration/GameServices.h"

#ifndef USE_COCOS_ENGINE
#define USE_COCOS_ENGINE 0
#endif

#if USE_COCOS_ENGINE
#include "base/CCFileUtils.h"
#include "platform/CCGLViewImpl.h"
#endif

namespace {
constexpr bool kDemoModeEnabled = false;
}  // namespace

bool AppDelegate::applicationDidFinishLaunching() {
    auto* director = cocos2d::Director::getInstance();
    auto* glview = director->getOpenGLView();
#if USE_COCOS_ENGINE
    if (!glview) {
        glview = cocos2d::GLViewImpl::create("Clash_of_Clans");
        if (!glview) {
            return false;
        }
        director->setOpenGLView(glview);
    }
    cocos2d::FileUtils::getInstance()->addSearchPath("Resources");
#else
    if (!glview) {
        return false;
    }
#endif

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
#if USE_COCOS_ENGINE
    if (kDemoModeEnabled) {
        cocos2d::Scene* menu_scene = scene_flow->ShowMenuScene();
        if (menu_scene) {
            director->replaceScene(menu_scene);
        }
    }
#endif
    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    cocos2d::Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
    cocos2d::Director::getInstance()->startAnimation();
}
