#ifndef ELEVATION_H
#define ELEVATION_H

#include <QtCore/QString>
#include <QtGui/QColor>

/**
 * @brief Elevation - Windows UI Kit 阴影和层级系统
 * 类似 Qt 中的 z-order + 阴影，越顶层的窗口阴影效果越明显
 */
namespace Elevation {
    /**
     * @brief 阴影级别
     */
    enum Level : int {
        None = 0,      // 无阴影
        Low = 1,       // 低层级
        Medium = 2,    // 中层级
        High = 3,      // 高层级
        VeryHigh = 4   // 极高层级
    };

    /**
     * @brief 阴影参数结构
     */
    struct ShadowParams {
        int offsetX;      // X 偏移
        int offsetY;      // Y 偏移
        int blurRadius;   // 模糊半径
        int spreadRadius; // 扩散半径
        QColor color;     // 阴影颜色
        double opacity;   // 透明度
    };

    /**
     * @brief Light 主题阴影
     */
    namespace Light {
        const ShadowParams Low = {
            0, 1, 2, 0,
            QColor(0, 0, 0),
            0.1
        };

        const ShadowParams Medium = {
            0, 2, 4, 0,
            QColor(0, 0, 0),
            0.15
        };

        const ShadowParams High = {
            0, 4, 8, 0,
            QColor(0, 0, 0),
            0.2
        };

        const ShadowParams VeryHigh = {
            0, 8, 16, 0,
            QColor(0, 0, 0),
            0.25
        };
    }

    /**
     * @brief Dark 主题阴影
     */
    namespace Dark {
        const ShadowParams Low = {
            0, 1, 2, 0,
            QColor(0, 0, 0),
            0.3
        };

        const ShadowParams Medium = {
            0, 2, 4, 0,
            QColor(0, 0, 0),
            0.4
        };

        const ShadowParams High = {
            0, 4, 8, 0,
            QColor(0, 0, 0),
            0.5
        };

        const ShadowParams VeryHigh = {
            0, 8, 16, 0,
            QColor(0, 0, 0),
            0.6
        };
    }

    /**
     * @brief 根据主题和级别获取阴影参数
     */
    inline const ShadowParams& getShadow(Level level, bool isDark = false) {
        if (isDark) {
            switch (level) {
                case Low: return Dark::Low;
                case Medium: return Dark::Medium;
                case High: return Dark::High;
                case VeryHigh: return Dark::VeryHigh;
                default: return Dark::Low;
            }
        } else {
            switch (level) {
                case Low: return Light::Low;
                case Medium: return Light::Medium;
                case High: return Light::High;
                case VeryHigh: return Light::VeryHigh;
                default: return Light::Low;
            }
        }
    }
}

#endif // ELEVATION_H
