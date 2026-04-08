---
description: "Fluent Design Qt 项目的 GTest 测试约定：FluentTestWindow 模式、VisualCheck 守卫、SetUpTestSuite 资源初始化"
applyTo: "tests/**"
---

# Fluent Design 测试规范

## 测试框架

使用 GoogleTest（GTest），通过 `add_qt_test_module(test_<name> Test<Name>.cpp)` 宏注册到 CMake。

## 必需的 SetUpTestSuite 初始化

每个测试类必须在 `SetUpTestSuite()` 中完成以下初始化：

```cpp
static void SetUpTestSuite() {
    int argc = 0;
    char** argv = nullptr;
    if (!qApp) new QApplication(argc, argv);
    QApplication::setStyle("Fusion");

    Q_INIT_RESOURCE(resources);
    QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
    QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
}
```

## FluentTestWindow 模式

每个测试文件定义一个继承 `QWidget` + `FluentElement` 的测试窗口类：

```cpp
class XxxTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};
```

在 `SetUp()` 中创建窗口并调用 `onThemeUpdated()` 确保主题初始化。

## VisualCheck 测试守卫

可视化测试开头添加环境变量守卫，`ctest` 自动注入 `SKIP_VISUAL_TEST=1` 跳过，Qt Creator 中运行单个测试正常弹窗：

```cpp
TEST_F(XxxTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }
    // 创建 UI 并展示
    window->show();
    qApp->exec();  // 阻塞直到手动关闭窗口
}
```

**运行方式**：
```bash
# 只跑可视化测试
./build/tests/views/<category>/test_<name> --gtest_filter="*VisualCheck*"
```

**关键**：VisualCheck 必须使用 `qApp->exec()` 阻塞，不要用 `QTest::qWait()`。

## CMake 注册

所有测试统一使用宏注册，支持传入额外源文件：
```cmake
# 单文件测试
add_qt_test_module(test_<name> Test<Name>.cpp)

# 需要额外源文件的测试（如 Delegate 实现）
add_qt_test_module(test_list_view TestListView.cpp FluentListItemDelegate.cpp)
```
