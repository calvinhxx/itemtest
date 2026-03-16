#ifndef SPACING_H
#define SPACING_H

/**
 * Spacing - Fluent Design 间距系统
 *
 * 基于 4px 网格，对齐 Windows UI Kit (Community) Basic Input 规范。
 */
namespace Spacing {

    // -------------------------------------------------------------------------
    // 基础间距阶梯（4px 网格）
    // -------------------------------------------------------------------------
    const int BaseUnit = 4;

    const int XSmall   =  4;   // 1×
    const int Small    =  8;   // 2×
    const int Medium   = 12;   // 3×
    const int Standard = 16;   // 4×
    const int Large    = 24;   // 6×
    const int XLarge   = 32;   // 8×
    const int XXLarge  = 48;   // 12×

    // -------------------------------------------------------------------------
    // 组件内边距（Padding）
    // -------------------------------------------------------------------------
    namespace Padding {
        const int ControlHorizontal  = 12;  // 通用控件左右（Button、ComboBox）
        const int ControlVertical    =  8;  // 通用控件上下

        const int TextFieldHorizontal = 8;  // 输入框内容区左右
        const int TextFieldVertical   = 4;  // 输入框内容区上下

        const int Card   = 16;  // 卡片
        const int Dialog = 24;  // 对话框

        const int ListItemHorizontal = 12;  // 列表项左右
        const int ListItemVertical   =  8;  // 列表项上下
    }

    // -------------------------------------------------------------------------
    // 描边宽度
    // -------------------------------------------------------------------------
    namespace Border {
        const int Normal  = 1;  // 默认状态
        const int Focused = 2;  // 聚焦高亮条
    }

    // -------------------------------------------------------------------------
    // 组件间距（Gap）
    // -------------------------------------------------------------------------
    namespace Gap {
        const int Tight   =  4;  // 图标与文字之间
        const int Normal  =  8;  // 同组控件之间
        const int Loose   = 16;  // 不同组控件之间
        const int Section = 24;  // 区块之间
    }

    // -------------------------------------------------------------------------
    // 标准控件高度
    // -------------------------------------------------------------------------
    namespace ControlHeight {
        const int Small    = 24;  // 紧凑布局
        const int Standard = 32;  // 默认（TextBox、Button）
        const int Large    = 40;  // 宽松布局
    }
}

#endif // SPACING_H
