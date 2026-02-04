#ifndef CORNERRADIUS_H
#define CORNERRADIUS_H

/**
 * @brief CornerRadius - Windows UI Kit 圆角规范
 * 参考 WinUI 3 Gallery 的 Geometry 页面定义
 */
namespace CornerRadius {
    /**
     * @brief Overlay 圆角 (8px)
     * 用于 Top-level containers (app windows, flyouts, cards, dialogs)
     */
    const int Overlay = 8;

    /**
     * @brief Control 圆角 (4px)
     * 用于 In-page elements (controls, list backplates)
     */
    const int Control = 4;

    /**
     * @brief 无圆角 (0px)
     * 用于与其他直边相交的边缘
     */
    const int None = 0;

    // --- 语义别名 (与之前版本保持兼容) ---
    const int TopLevel = Overlay;
    const int InPage = Control;
    const int Medium = Control;
    const int Small = 2; // WinUI 3 极少数场景仍使用 2px
    const int Large = 12;
}

#endif // CORNERRADIUS_H
