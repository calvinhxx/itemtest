#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

/**
 * @brief Breakpoints - Windows UI Kit 响应式布局断点
 * 基于 Fluent Design "Adaptive Layout" 规范
 */
namespace Breakpoints {
    // --- 标准断点 ---
    const int Small = 640;      // 0 - 640px: Compact (手机/窄窗口)
    const int Medium = 1007;    // 641 - 1007px: Medium (平板/半屏)
    const int Large = 1920;     // 1008 - 1920px: Expanded (桌面全屏)
    
    // --- 窗口限制 ---
    const int MinWindowWidth = 320;
    const int MinWindowHeight = 500;

    // --- 侧边栏/导航栏标准尺寸 ---
    const int NavigationPaneCompactWidth = 48;
    const int NavigationPaneExpandedWidth = 320;
}

#endif // BREAKPOINTS_H
