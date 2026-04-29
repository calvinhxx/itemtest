#ifndef ANIMATION_H
#define ANIMATION_H

#include <QEasingCurve>

/** Animation - Fluent Design Motion 规范（持续时间 + 缓动曲线）。 */
namespace Animation {

    // -------------------------------------------------------------------------
    // 持续时间（ms）
    // -------------------------------------------------------------------------
    namespace Duration {
        const int Fast     = 150;  // 按钮点击、小组件切换等即时反馈
        const int Normal   = 250;  // 展开/收起、简单进入/退出
        const int Slow     = 400;  // 页面转场、弹窗弹出等显著变化
        const int VerySlow = 700;  // 大型容器初始加载
    }

    // -------------------------------------------------------------------------
    // 缓动曲线
    // -------------------------------------------------------------------------
    enum class EasingType {
        Standard,   // InOutSine  — 平滑开始与结束
        Accelerate, // InCubic    — 慢启快收
        Decelerate, // OutCubic   — 快启慢收
        Entrance,   // OutBack    — 带轻微回弹的进入
        Exit        // InQuint    — 快速消失
    };

    inline QEasingCurve getEasing(EasingType type) {
        switch (type) {
            case EasingType::Standard:   return QEasingCurve::InOutSine;
            case EasingType::Accelerate: return QEasingCurve::InCubic;
            case EasingType::Decelerate: return QEasingCurve::OutCubic;
            case EasingType::Entrance: {
                QEasingCurve curve(QEasingCurve::OutBack);
                curve.setAmplitude(0.5);
                return curve;
            }
            case EasingType::Exit: return QEasingCurve::InQuint;
            default:               return QEasingCurve::Linear;
        }
    }
}

#endif // ANIMATION_H
