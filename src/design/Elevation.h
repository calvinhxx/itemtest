#ifndef ELEVATION_H
#define ELEVATION_H

#include <QtGui/QColor>

/** Elevation - Fluent Design 阴影层级系统（offsetY / blurRadius / opacity）。 */
namespace Elevation {

    // -------------------------------------------------------------------------
    // 层级枚举
    // -------------------------------------------------------------------------
    enum Level : int {
        None    = 0,
        Low     = 1,
        Medium  = 2,
        High    = 3,
        VeryHigh = 4
    };

    // -------------------------------------------------------------------------
    // 阴影参数
    // -------------------------------------------------------------------------
    struct ShadowParams {
        int    offsetX;
        int    offsetY;
        int    blurRadius;
        int    spreadRadius;
        QColor color;
        double opacity;
    };

    // -------------------------------------------------------------------------
    // Light 主题阴影
    // -------------------------------------------------------------------------
    namespace Light {
        const ShadowParams Low      = { 0,  1,  2, 0, QColor(0,0,0), 0.10 };
        const ShadowParams Medium   = { 0,  2,  4, 0, QColor(0,0,0), 0.15 };
        const ShadowParams High     = { 0,  4,  8, 0, QColor(0,0,0), 0.20 };
        const ShadowParams VeryHigh = { 0,  8, 16, 0, QColor(0,0,0), 0.25 };
    }

    // -------------------------------------------------------------------------
    // Dark 主题阴影（opacity 整体更高以补偿深色背景对比度）
    // -------------------------------------------------------------------------
    namespace Dark {
        const ShadowParams Low      = { 0,  1,  2, 0, QColor(0,0,0), 0.30 };
        const ShadowParams Medium   = { 0,  2,  4, 0, QColor(0,0,0), 0.40 };
        const ShadowParams High     = { 0,  4,  8, 0, QColor(0,0,0), 0.50 };
        const ShadowParams VeryHigh = { 0,  8, 16, 0, QColor(0,0,0), 0.60 };
    }

    // -------------------------------------------------------------------------
    // 辅助函数
    // -------------------------------------------------------------------------
    inline const ShadowParams& getShadow(Level level, bool isDark = false) {
        if (isDark) {
            switch (level) {
                case Low:     return Dark::Low;
                case Medium:  return Dark::Medium;
                case High:    return Dark::High;
                case VeryHigh:return Dark::VeryHigh;
                default:      return Dark::Low;
            }
        } else {
            switch (level) {
                case Low:     return Light::Low;
                case Medium:  return Light::Medium;
                case High:    return Light::High;
                case VeryHigh:return Light::VeryHigh;
                default:      return Light::Low;
            }
        }
    }
}

#endif // ELEVATION_H
