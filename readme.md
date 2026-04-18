# 设计稿 📖 [Win UI Kit](https://aka.ms/WinUI/3.0-figma-toolkit)
# 源项目 📚 [WinUI-Gallery](https://github.com/microsoft/WinUI-Gallery)

# Qt 版本支持

本工程同时兼容 **Qt 5.15+** 与 **Qt 6.2+**（CMake 自动选择可用版本）。

```bash
# Qt6 构建（默认优先）
cmake -B build && cmake --build build

# Qt5 构建（指定 Qt5 路径）
cmake -B build-qt5 -DCMAKE_PREFIX_PATH=/path/to/qt5 && cmake --build build-qt5
```

CMake 会强制版本下限：Qt 5 < 5.15 或 Qt 6 < 6.2 时配置失败。
新组件重写 `enterEvent` 时 MUST 使用 `FluentEnterEvent`（来自 `common/QtCompat.h`），切勿直接使用 `QEnterEvent`。

# todo
1️⃣ lisence问题
2️⃣ Segoe 版权问题
3️⃣ 迁移标准FluentUI项目
4️⃣ 迁移标准项目后使用gitemoji风格commit设计changelog
5️⃣ readme
6️⃣ 样式, 配置等可以通过配置文件热更新修改
所有组件实现完后系统性走查一下所有组件代码，简化一下代码

##
collections内的容器测试
找一个windows带touchpad的笔记本修复滚轮问题