#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

/** Breakpoints - Fluent Design Adaptive Layout 响应式断点与窗口尺寸约束。 */
namespace Breakpoints {

    // -------------------------------------------------------------------------
    // 布局断点（px）
    // -------------------------------------------------------------------------
    const int Small  =  640;  // 0–640:   Compact（手机 / 窄窗口）
    const int Medium = 1007;  // 641–1007: Medium（平板 / 半屏）
    const int Large  = 1920;  // 1008–1920: Expanded（桌面全屏）

    // -------------------------------------------------------------------------
    // 窗口最小尺寸约束（px）
    // -------------------------------------------------------------------------
    const int MinWindowWidth  = 320;
    const int MinWindowHeight = 500;

    // -------------------------------------------------------------------------
    // 导航栏标准宽度（px）
    // -------------------------------------------------------------------------
    const int NavigationPaneCompactWidth  =  48;
    const int NavigationPaneExpandedWidth = 320;
}

#endif // BREAKPOINTS_H
