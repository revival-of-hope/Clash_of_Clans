---
title: cocos模块学习
tags:
cover:
---
## cocos命令
cocos new <项目名>  -l cpp 当前目录下生成
cmake -S . -B build -G "Visual Studio 17 2022" -A win32 -T v143  在新项目里强制改cmake方式.
新增加文件时,在cmake里面加入对应的文件路径
再运行` cmake .. `就行了

## 概念部分

精灵:可以理解为人物和物品,是可操作的对象
label: 可以理解为字幕,大到开始界面标题,小到物品资源名字,都能用

## 初始化部分

## 窗口部分

```cpp
auto visibleSize = Director::getInstance()->getVisibleSize();
    //窗口对角线向量,这个向量可以调用visibleSize.width
    //和visibleSize.height两个子属性
    auto origin = Director::getInstance()->getVisibleOrigin();
    //左下角坐标(0,0)
```

在AppDelegate.cpp下有一行
`glview = GLViewImpl::createWithRect("test", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));`
修改designResolutionSize为其他大小(如largeResolutionSize)即可修改窗口默认大小


## 创建部分

### 创建元素
**所有创建的精灵和场景,动作都会自动销毁**
*创建精灵*
```cpp
auto child = Sprite::create("Images/test.png");
auto mySprite = Sprite::create("mysprite.png", Rect(0,0,200,200));
//可以指定裁剪区域,(0,0)为左上角坐标,大小为200X200

// 创建多边形精灵.
auto pinfo = AutoPolygon::generatePolygon("filename.png");

// Create a sprite with polygon info.
auto sprite = Sprite::create(pinfo);
```
[还可以使用图集来创建](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/spritesheets.html)


*创建label*
```cpp
auto label = Label::createWithTTF("Hello World", "fonts/arial.ttf", 16.0f);
auto label = Label::create("Hello World", "fonts/arial.ttf", 16.0f);
//两种方法均可以,下面一种在字体不能正常加载时使用系统字体,那我肯定用下面一种啊
```

*创建menu*
```cpp
//最复杂的部分

//先创建一个image对象
auto closeItem = MenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        CC_CALLBACK_1(HelloWorld::menuCloseCallback,this));
//可以用回调函数,但用lamda表达式更像人类一点,官方例子如下

//使用 lambda 表达式作为菜单项的回调函数：
auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
[&](Ref* sender){
    // your code here
});

//还是看不懂的话有以下例子
auto showPlayerDlgItem = MenuItemImage::create("Images/Pea.png",
                                               "Images/Pea.png",
                                               [this](Ref* sender){
    this->_playerDlg->setVisible(!this->_playerDlg->isVisible());
});
//简单说来就是点击这个菜单图片会发生什么

//现在用图像来创建菜单
auto menu = Menu::create(closeItem, nullptr);
auto menu = Menu::create(closeItem,closeItem2,closeItem3, nullptr);
//可以一直叠下去

```


### 指定元素位置(菜单,UI组件通用)
```cpp
child->setPosition(Vec2(origin.x,origin.y));
child->setPosition(vec2(visibleSize/2));
//两种方法都可以
```

**改变精灵的中心点**
```cpp
// DEFAULT anchor point for all Sprites
mySprite->setAnchorPoint(0.5, 0.5);

// bottom left
mySprite->setAnchorPoint(0, 0);

// top left
mySprite->setAnchorPoint(0, 1);

// bottom right
mySprite->setAnchorPoint(1, 0);

// top right
mySprite->setAnchorPoint(1, 1);
```

### 将元素加入指定图层(通用)
`this->addChild(child, 1,people);`

>原定义部分
void Node::addChild(Node *child, int localZOrder, int tag)

即第一个参数是元素名称,第二个参数是指定图层坐标,第三个参数是标签,
后两个均为可选


## 动作部分

### 旋转
通过 setRotation() 方法，设置一个角度值可以控制精灵的旋转，正值精灵顺时针旋转，负值精灵逆时针旋转，默认位置的角度值是 0.0。
`mySprite->setRotation(20.0f);`

### 缩放

```cpp
// increases X and Y size by 2.0 uniformly
mySprite->setScale(2.0);

// increases just X scale by 2.0
mySprite->setScaleX(2.0);

// increases just Y scale by 2.0
mySprite->setScaleY(2.0);
```
**动作中的缩放方法**
```cpp
// Scale uniformly by 3x over 2 seconds
auto scaleBy = ScaleBy::create(2.0f, 3.0f);
mySprite->runAction(scaleBy);

// Scale X by 2 and Y by 3x over 2 seconds
auto scaleBy = ScaleBy::create(2.0f, 3.0f, 3.0f);
mySprite->runAction(scaleBy);

// Scale to uniformly to 3x over 2 seconds
auto scaleTo = ScaleTo::create(2.0f, 3.0f);
mySprite->runAction(scaleTo);

// Scale X to 2 and Y to 3x over 2 seconds
auto scaleTo = ScaleTo::create(2.0f, 3.0f, 3.0f);
mySprite->runAction(scaleTo);
```

### 移动
```cpp
// Move sprite to position 50,10 in 2 seconds.
auto moveTo = MoveTo::create(2, Vec2(50, 10));
mySprite1->runAction(moveTo);

// Move sprite 20 points to right in 2 seconds
auto moveBy = MoveBy::create(2, Vec2(20,0));
mySprite2->runAction(moveBy);
```
注意到每个动作都要先创建为对象再用runAction执行

**可以创建一个移动序列**
```cpp
auto moveBy = MoveBy::create(2, Vec2(500, mySprite->getPositionY()));

// MoveTo - lets move the new sprite to 300 x 256 over 2 seconds
// MoveTo is absolute - The sprite gets moved to 300 x 256 regardless of
// where it is located now.
auto moveTo = MoveTo::create(2, Vec2(300, mySprite->getPositionY()));

// Delay - create a small delay
auto delay = DelayTime::create(1);

auto seq = Sequence::create(moveBy, delay, moveTo, nullptr);

mySprite->runAction(seq);
```
注意到cocos里面用到数组对象时总要指定最后一位为空指针

查看源码发现runAction指接收一个参数,也就是说延迟动作只能显示写一个delay对象实现

### 色彩混合
```cpp
auto mySprite = Sprite::create("mysprite.png");

// Tints a node to the specified RGB values
auto tintTo = TintTo::create(2.0f, 120.0f, 232.0f, 254.0f);
mySprite->runAction(tintTo);

// Tints a node BY the delta of the specified RGB values.
auto tintBy = TintBy::create(2.0f, 120.0f, 232.0f, 254.0f);
mySprite->runAction(tintBy);
```
可用于实现药水对兵种的作用效果