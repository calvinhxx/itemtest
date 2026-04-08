---
description: "创建新的 Fluent Design Qt 组件：生成 .h/.cpp 骨架 + 测试文件 + CMake 注册"
agent: "agent"
argument-hint: "组件名称和 Qt 基类，例如：RadioButton 基于 QRadioButton，放在 basicinput 分类下"
---

根据用户的描述创建一个新的 Fluent Design 组件。严格遵循项目的三重继承 Mixin 模式和 [copilot-instructions.md](.github/copilot-instructions.md) 中的新组件 Checklist。

## 需要收集的信息

从用户输入中确定：
1. **组件名称** (PascalCase) — 例如 `RadioButton`
2. **Qt 基类** — 例如 `QRadioButton`
3. **分类目录** — `basicinput` / `textfields` / `dialogs_flyouts` / `collections` / `status_info` / `scrolling` / `menus_toolbars` / `navigation` / `date_time` / `media` / `shell` / `shellprimitives` / `splashscreen` 等
4. **Fluent 样式枚举** — 是否需要 `fluentStyle`（如 Standard/Accent）、`fluentSize`（如 Small/Standard/Large）
5. **交互状态** — Rest / Hover / Pressed / Disabled（默认都需要）

如果用户未指定全部信息，推断最合理的默认值。

## 生成步骤

### 1. 头文件 `src/view/<category>/<Name>.h`

```cpp
#ifndef <NAME>_H
#define <NAME>_H

#include <<QtBase>>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::<category> {

class <Name> : public <QtBase>, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    // Q_PROPERTY 声明 (fluentStyle, interactionState 等)

public:
    // 交互状态枚举
    enum InteractionState { Rest, Hover, Pressed, Disabled };
    Q_ENUM(InteractionState)

    explicit <Name>(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    InteractionState m_interactionState = Rest;
};

} // namespace view::<category>

#endif
```

### 2. 实现文件 `src/view/<category>/<Name>.cpp`

- 构造函数：`setAttribute(Qt::WA_Hover)`，设置字体 `themeFont("Body")`
- `onThemeUpdated()`：调用 `update()` 触发重绘
- `paintEvent()`：使用 `QPainter` + `setRenderHint(QPainter::Antialiasing)` + `themeColors()` / `themeRadius()` 等 Design Token 自绘
- 交互事件：切换 `m_interactionState` 并 `update()`

### 3. 测试文件 `tests/views/<category>/Test<Name>.cpp`

```cpp
#include <gtest/gtest.h>
#include <QApplication>
#include <QFontDatabase>
#include "view/<category>/<Name>.h"
#include "view/FluentElement.h"

using namespace view::<category>;

class <Name>TestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class <Name>Test : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");
        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new <Name>TestWindow();
        window->setFixedSize(600, 400);
        window->onThemeUpdated();
    }

    void TearDown() override { delete window; }

    <Name>TestWindow* window = nullptr;
};

// 单元测试
TEST_F(<Name>Test, DefaultState) {
    auto* widget = new <Name>(window);
    // 验证默认属性
}

// 可视化测试
TEST_F(<Name>Test, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }
    // 创建 UI，展示各种状态
    window->show();
    qApp->exec();
}
```

### 4. CMake 注册

在 `tests/views/<category>/CMakeLists.txt` 追加：
```cmake
add_qt_test_module(test_<snake_name> Test<Name>.cpp)
```

## 最终检查

- [ ] 三重继承: Qt 基类 + FluentElement + QMLPlus
- [ ] `onThemeUpdated()` 已实现
- [ ] `paintEvent()` 使用 Design Token 自绘 + Antialiasing
- [ ] Q_PROPERTY 暴露 Fluent 样式属性
- [ ] 状态驱动 (Rest → Hover → Pressed → Disabled)
- [ ] 测试文件包含 VisualCheck 环境变量守卫
- [ ] CMakeLists.txt 已注册 test module
