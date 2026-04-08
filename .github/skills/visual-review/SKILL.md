---
name: visual-review
description: "运行 Fluent Design 组件的 VisualCheck 测试进行可视化审查。Use when: 需要查看组件渲染效果、验证 UI 样式、调试视觉问题。"
argument-hint: "组件名称，例如 button / checkbox / list_view"
---

# 可视化审查 (Visual Review)

运行指定组件的 VisualCheck 测试，打开可交互窗口进行实时视觉审查。

## 使用场景

- 修改组件 `paintEvent()` 后验证渲染效果
- 主题切换 (Light/Dark) 后检查颜色是否正确
- 新组件开发时实时预览

## 步骤

### 1. 确定测试二进制

根据用户提供的组件名，映射到对应的测试二进制：

| 组件 | 测试二进制 |
|------|-----------|
| button | `./build/tests/views/basicinput/test_button` |
| checkbox | `./build/tests/views/basicinput/test_checkbox` |
| radio_button | `./build/tests/views/basicinput/test_radio_button` |
| slider | `./build/tests/views/basicinput/test_slider` |
| combo_box | `./build/tests/views/basicinput/test_combo_box` |
| color_picker | `./build/tests/views/basicinput/test_color_picker` |
| toggle_switch | `./build/tests/views/basicinput/test_toggle_switch` |
| toggle_button | `./build/tests/views/basicinput/test_toggle_button` |
| split_button | `./build/tests/views/basicinput/test_split_button` |
| toggle_split_button | `./build/tests/views/basicinput/test_toggle_split_button` |
| drop_down_button | `./build/tests/views/basicinput/test_drop_down_button` |
| hyperlink_button | `./build/tests/views/basicinput/test_hyperlink_button` |
| repeat_button | `./build/tests/views/basicinput/test_repeat_button` |
| rating_control | `./build/tests/views/basicinput/test_rating_control` |
| list_view | `./build/tests/views/collections/test_list_view` |
| grid_view | `./build/tests/views/collections/test_grid_view` |
| tree_view | `./build/tests/views/collections/test_tree_view` |
| flip_view | `./build/tests/views/collections/test_flip_view` |
| dialog | `./build/tests/views/dialogs_flyouts/test_dialog` |
| tooltip | `./build/tests/views/status_info/test_tooltip` |
| scrollbar | `./build/tests/views/scrolling/test_scroll_bar` |
| menu_bar | `./build/tests/views/menus_toolbars/test_menu_bar` |
| text_block | `./build/tests/views/textfields/test_text_block` |
| line_edit | `./build/tests/views/textfields/test_line_edit` |
| text_edit | `./build/tests/views/textfields/test_text_edit` |

通用规则：`./build/tests/views/<category>/test_<snake_case_name>`

### 2. 构建项目

确保最新代码已编译：

```bash
cmake --build build
```

### 3. 运行 VisualCheck 测试

只运行 VisualCheck 相关的测试用例（窗口弹出后手动关闭结束）：

```bash
./build/tests/views/<category>/test_<name> --gtest_filter="*VisualCheck*"
```

**注意**：VisualCheck 测试通过 `SKIP_VISUAL_TEST` 环境变量守卫控制跳过，`ctest` 会自动注入 `SKIP_VISUAL_TEST=1` 跳过所有 VisualCheck。直接运行二进制时不会跳过。

### 4. 检查要点

审查窗口弹出后，关注以下方面：
- **颜色**: 是否与 WinUI 3 设计规范一致（查看 `ThemeColors.h` 中的语义色）
- **圆角**: Control 级别组件应为 4px，Overlay 级别应为 8px
- **间距**: 是否遵循 4px 网格系统
- **字体**: 是否使用 Segoe UI Variable，大小是否匹配 Typography 定义
- **状态**: Rest → Hover → Pressed → Disabled 各状态视觉反馈是否正确
- **主题**: 如果测试包含主题切换按钮，验证 Light/Dark 切换效果

### 5. 运行全部测试验证

视觉审查通过后，运行全部自动化测试确保没有回归：

```bash
cd build && ctest --verbose
```
