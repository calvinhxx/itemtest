#ifndef CORNERRADIUS_H
#define CORNERRADIUS_H

/**
 * @brief CornerRadius - Windows UI Kit 圆角规范
 * 根据 Guidance & Charts 页面定义的标准圆角值
 */
namespace CornerRadius {
    /**
     * @brief 顶层容器圆角
     * 用于 Top level container（如对话框、弹窗等）
     */
    const int TopLevel = 8;

    /**
     * @brief 页面内容器圆角
     * 用于 In page container（如卡片、面板等）
     */
    const int InPage = 4;

    /**
     * @brief 无圆角
     * 用于与其他直边相交的元素、最大化窗口等
     */
    const int None = 0;

    /**
     * @brief 小圆角（可选）
     * 用于小型元素，如按钮、输入框等
     */
    const int Small = 2;

    /**
     * @brief 中等圆角（可选）
     * 介于 Small 和 InPage 之间
     */
    const int Medium = 4;

    /**
     * @brief 大圆角（可选）
     * 用于特殊设计需求
     */
    const int Large = 12;
}

#endif // CORNERRADIUS_H
