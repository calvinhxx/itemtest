# Project Guidelines — Fluent Design Qt Component Library

## Overview

基于 Qt Widgets 实现的 Microsoft Fluent Design System (WinUI 3) 组件库。所有 UI 组件通过自绘 (`paintEvent`) 实现 WinUI 3 视觉效果，不使用 QSS/QPalette。

## Architecture

### 三重继承 Mixin 模式

每个 UI 组件遵循三重继承结构：

```cpp
class MyWidget : public QSomeWidget,      // Qt 基础控件
                 public FluentElement,     // Design Token 访问 (主题色、字体、圆角、间距等)
                 public view::QMLPlus {    // AnchorLayout 布局 + PropertyBinder 绑定 + State 状态机
};
```

### 核心层

| 组件 | 职责 |
|------|------|
| `FluentElement` (+ `FluentThemeManager` 单例) | 全局主题管理 (Light/Dark)，分发 Design Token（Colors/Font/Radius/Spacing/Animation/Material/Elevation） |
| `QMLPlus` | QML 风格的 `AnchorLayout`、`PropertyBinder` 数据绑定、`States` 状态切换 |
| `ViewModel` | 通用视图模型，暴露 text/enabled/title/visible 属性，配合 PropertyBinder 使用 |
| `DebugOverlay` | 调试工具，红框高亮目标控件边界 |

### Design Token (src/common/)

- `ThemeColors.h` — 60+ 语义化颜色 (Fill/Stroke/Text/Background/System)
- `Typography.h` — Segoe UI Variable 字体族 + 8 种排版样式 (Caption→Display)
- `CornerRadius.h` — None(0) / Control(4) / Overlay(8)
- `Spacing.h` — 4px 网格间距系统 + 控件高度标准
- `Animation.h` — 动画时长 + 缓动曲线
- `Material.h` — Acrylic/Mica/Smoke 材质
- `Elevation.h` — 5 级阴影 (None→VeryHigh)
- `Breakpoints.h` — 响应式断点

### 组件目录 (src/view/)

```
basicinput/       — Button, CheckBox, RadioButton, Slider, ComboBox, ColorPicker,
                    ToggleSwitch, ToggleButton, SplitButton, ToggleSplitButton,
                    DropDownButton, HyperlinkButton, RepeatButton, RatingControl
textfields/       — LineEdit, TextEdit, TextBlock
dialogs_flyouts/  — Dialog
menus_toolbars/   — Menu, MenuBar
status_info/      — ToolTip
scrolling/        — ScrollBar
collections/      — ListView, GridView, TreeView, FlipView
                    （仅视图；model/delegate 由业务或 tests/views/collections 示例组装）
                    ListView  — 列表视图，分组/Header/Footer/拖拽排序
                    GridView  — 网格视图，自动换行/可配尺寸/拖拽排序
                    TreeView  — 树视图，展开折叠动画/Checkbox级联/文件管理器风格拖拽
                    FlipView  — 翻页轮播，导航按钮/指示器
navigation/       — (规划中)
date_time/        — (规划中)
media/            — (规划中)
shell/            — (规划中)
shellprimitives/  — (规划中)
splashscreen/     — (规划中)
```

## Build and Test

```bash
# 配置 (Qt 6, macOS/MSVC)
cmake -B build -DBUILD_TESTING=ON

# 构建
cmake --build build

# 运行全部单元测试 (VisualCheck 自动跳过，ctest 会注入 SKIP_VISUAL_TEST=1)
cd build && ctest --verbose

# 运行单个测试 (直接运行二进制，VisualCheck 会正常弹窗需手动关闭)
./build/tests/views/basicinput/test_button
./build/tests/views/test_fluent_element

# 只跑可视化测试
./build/tests/views/collections/test_list_view --gtest_filter="*VisualCheck*"
```

- **构建目标**: `itemstest_lib` (静态库)
- **依赖**: Qt 6 Widgets + Qt Test, GoogleTest (third_party/)
- **C++ 标准**: C++17
- **MSVC**: UTF-8 编译 + 静态运行时 (MT/MTd)

### 测试执行策略

- **增量验证（默认）**: 只改动单个 `.cpp` 文件几行代码时，**禁止全量 `ctest`**，只构建并运行受影响的测试目标：
  ```bash
  cmake --build build --target <test_target> && ./build/tests/views/<category>/<test_target>
  ```
  示例：改 `ListView.cpp` → `cmake --build build --target test_list_view && ./build/tests/views/collections/test_list_view`
  示例：改 `Button.cpp` → `cmake --build build --target test_button && ./build/tests/views/basicinput/test_button`
- **全量测试（仅以下情况）**:
  - 用户明确要求跑全量
  - 改动涉及公共头文件（`FluentElement.h`、`ThemeColors.h`、`Animation.h` 等 `src/common/` 下的文件）
  - 改动影响静态库 `itemstest_lib` 的公共 API
  - 提交 / PR 前的最终验证
- **测试二进制路径**: `./build/tests/views/<category>/<test_name>`
  - 顶层框架测试: `./build/tests/views/test_fluent_element`
  - 分类测试: `./build/tests/views/basicinput/test_button`、`./build/tests/views/collections/test_tree_view` 等

## Conventions

### 命名规范

- 类名 / 枚举: `PascalCase` — `ButtonStyle`, `InteractionState`
- 函数 / 属性: `camelCase` — `setFluentStyle()`, `onThemeUpdated()`
- 成员变量: `m_` 前缀 — `m_style`, `m_iconGlyph`
- Design Token 常量: 命名空间嵌套 — `Spacing::Gap::Normal`, `CornerRadius::Control`

### 新组件 Checklist

1. 继承对应 Qt 基类 + `FluentElement` + `view::QMLPlus`
2. 实现 `onThemeUpdated()` — 响应主题切换，重新设置字体/颜色
3. 实现 `paintEvent()` — 使用 `themeColors()` 等 Token 自绘，启用 `QPainter::Antialiasing`
4. 通过 `Q_PROPERTY` 暴露 Fluent 风格属性 (如 `fluentStyle`, `fluentSize`)
5. **禁止硬编码 Design Token 字符串** — `themeFont("Body")` 等访问 Token 的字符串参数必须抽为可配置的 `Q_PROPERTY` 成员变量（如 `m_fontRole`），在 `paintEvent()` / `onThemeUpdated()` 中通过成员变量间接访问，禁止直接写死字面量
6. **Qt5/Qt6 兼容** — 重写 `enterEvent()` 时 MUST 使用 `FluentEnterEvent`（来自 `common/QtCompat.h`），切勿直接使用 `QEnterEvent`（Qt5 不存在该类型）
7. 状态驱动: Rest → Hover → Pressed → Disabled，通过 event filter 或 `enterEvent`/`leaveEvent` 驱动
8. 创建对应测试文件 `tests/views/<category>/Test<Name>.cpp`
9. 在对应 `CMakeLists.txt` 中使用 `add_qt_test_module(test_<name> Test<Name>.cpp [额外源文件...])` 注册

### 测试模式

- 每个组件一个 `Test<Name>.cpp`，使用 GTest 框架
- Mock 组件继承 `QWidget` + `FluentElement` 用于隔离测试
- **VisualCheck 测试**:
  - 必须使用 `qApp->exec()` 阻塞（不使用 `QTest::qWait()`）
  - 开头添加环境变量守卫，`ctest` 自动注入 `SKIP_VISUAL_TEST=1` 跳过，Qt Creator 中运行单个测试正常弹窗：
    ```cpp
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }
    ```
  - 命令行运行：`./build/tests/views/<category>/test_xxx --gtest_filter="*VisualCheck*"`
  - Qt Creator 中：在 Test Results 面板右键指定测试 → Run
  - **内容溢出处理**: 当 VisualCheck 中组件过多导致窗口高度超出屏幕时，使用 `QScrollArea` 包裹内容区。参考模式：创建 `QScrollArea`（隐藏原生滚动条）+ Fluent `ScrollBar` 覆盖层 + `FluentTestWindow` 作为 content widget，所有子控件挂载到 content 而非 window。主题切换时同步 `scrollArea->setStyleSheet(content->styleSheet())`。详见 `TestListView.cpp` / `TestGridView.cpp` 的 VisualCheck 实现

### 资源

- 字体文件打包在 `res/` 目录，通过 `resources.qrc` 编译进二进制
- Icon 使用 Segoe Fluent Icons 字体 (iconfont)，常量定义在 `Typography::Icons`

## Pitfalls

- **macOS 字体 Metrics**: 首次布局时需 `ensurePolished()` + `onThemeUpdated()` 以确保 Segoe UI Variable 字体 metrics 正确初始化
- **Segoe 字体版权**: 该字体属于微软，生产使用需注意授权问题
- **已知缺陷**: Dialog 弹出闪缩、TextBox 多行实现不完善、ComboBox 实现有缺陷
- **TreeView 拖拽**: 文件管理器风格拖拽需注意 cycle prevention（祖先不能拖入后代）、跨父级 re-parenting 需 `takeRow` + `insertRow`/`appendRow`、`visualRect()` 返回含缩进偏移的 rect
- **HiDPI 渲染**: 拖拽快照等自定义绘制需使用 `devicePixelRatioF()` 适配 Retina 屏幕
