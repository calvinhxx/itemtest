#ifndef SPACING_H
#define SPACING_H

/**
 * @brief Spacing - Windows UI Kit 间距系统
 * 提供标准化的间距值，确保 UI 元素之间的一致性
 */
namespace Spacing {
    /**
     * @brief 基础间距单位（4px 网格系统）
     */
    const int BaseUnit = 4;

    /**
     * @brief 极小间距
     */
    const int XSmall = 4;   // 1x base unit

    /**
     * @brief 小间距
     */
    const int Small = 8;    // 2x base unit

    /**
     * @brief 中等间距
     */
    const int Medium = 12;  // 3x base unit

    /**
     * @brief 标准间距
     */
    const int Standard = 16; // 4x base unit

    /**
     * @brief 大间距
     */
    const int Large = 24;    // 6x base unit

    /**
     * @brief 超大间距
     */
    const int XLarge = 32;   // 8x base unit

    /**
     * @brief 极大间距
     */
    const int XXLarge = 48;  // 12x base unit

    /**
     * @brief 组件内边距规范
     */
    namespace Padding {
        const int ControlHorizontal = 12; // 控件通用水平内边距
        const int ControlVertical = 6;    // 控件通用垂直内边距
        
        const int Card = 16;              // 卡片内边距
        const int Dialog = 24;            // 对话框内边距
    }

    /**
     * @brief 组件间距规范
     */
    namespace Gap {
        const int Tight = 4;      // 紧密间距
        const int Normal = 8;     // 正常间距
        const int Loose = 16;      // 宽松间距
        const int Section = 24;    // 区块间距
    }
}

#endif // SPACING_H
