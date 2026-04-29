#ifndef CORNERRADIUS_H
#define CORNERRADIUS_H

/**
 * CornerRadius - Fluent Design 圆角规范
 *
 * WinUI3 仅定义两个标准档位：
 *   ControlCornerRadius = 4px  用于页面内控件（Button、TextBox 等）
 *   OverlayCornerRadius = 8px  用于浮层容器（Flyout、Card、Dialog）
 */
namespace CornerRadius {

    const int None    = 0;  // 直角，用于与直边相接的元素
    const int Control = 4;  // 页面内控件（Button、TextBox 等）
    const int Overlay = 8;  // 浮层容器（Flyout、Card、Dialog、Tooltip）
}

#endif // CORNERRADIUS_H
