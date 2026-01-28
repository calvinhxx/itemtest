#ifndef ANIMATION_H
#define ANIMATION_H

#include <QEasingCurve>

/**
 * @brief Animation - Windows UI Kit 动画规范
 * 基于 Fluent Design System 的 Motion 指导
 */
namespace Animation {

    /**
     * @brief 持续时间 (ms)
     */
    namespace Duration {
        const int Fast = 150;      // 快速反馈 (如按钮点击、小组件切换)
        const int Normal = 250;    // 标准时长 (如展开/收起、简单的进入/退出)
        const int Slow = 400;      // 显著变化 (如复杂的页面转场、弹窗弹出)
        const int VerySlow = 700;  // 极慢 (如大型容器的初始加载)
    }

    /**
     * @brief 缓动曲线类型
     */
    enum class EasingType {
        Standard,   // 标准：平滑开始和平滑结束 (InOut)
        Accelerate, // 加速：开始慢，结束快 (In)
        Decelerate, // 减速：开始快，结束慢 (Out)
        Entrance,   // 进入：具有轻微的回弹效果 (OutBack)
        Exit        // 退出：迅速消失 (InQuint)
    };

    /**
     * @brief 获取对应的 QEasingCurve
     */
    inline QEasingCurve getEasing(EasingType type) {
        switch (type) {
            case EasingType::Standard:   return QEasingCurve::InOutSine;
            case EasingType::Accelerate: return QEasingCurve::InCubic;
            case EasingType::Decelerate: return QEasingCurve::OutCubic;
            case EasingType::Entrance: {
                QEasingCurve curve(QEasingCurve::OutBack);
                curve.setAmplitude(0.5); // 适度的回弹感
                return curve;
            }
            case EasingType::Exit:       return QEasingCurve::InQuint;
            default:                     return QEasingCurve::Linear;
        }
    }
}

#endif // ANIMATION_H
