#ifndef MATERIAL_H
#define MATERIAL_H

#include <QColor>

/**
 * Material - Fluent Design 材质系统参数。
 *
 * 每种材质以结构体形式暴露纯数据，不包含任何渲染逻辑。
 * 调用方根据自身渲染方式（QPainter / QSS / QGraphicsEffect 等）自行决定如何使用这些参数。
 *
 *   Acrylic — 毛玻璃透明材质：tintColor + tintOpacity 组成覆盖层，luminosityOpacity 提供亮度层
 *   Mica    — 云母非透明材质：较高 opacity 的单色填充，呈半透明实心感
 *   Smoke   — 模态遮罩背景：黑色半透明，用于遮罩下层内容
 */
namespace Material {

    // -------------------------------------------------------------------------
    // 数据结构
    // -------------------------------------------------------------------------

    struct AcrylicToken {
        QColor tintColor;           // Tint 叠加色
        double tintOpacity;         // Tint 层透明度 [0, 1]
        double luminosityOpacity;   // Luminosity 层透明度 [0, 1]
        int    blurRadius;          // 背景模糊半径（需 QGraphicsBlurEffect 或平台 API 实现）
    };

    struct MicaToken {
        QColor baseColor;           // 材质基础色
        double opacity;             // 整体透明度 [0, 1]
    };

    struct SmokeToken {
        QColor baseColor;           // 遮罩色（通常为黑色）
        double opacity;             // 整体透明度 [0, 1]
    };

    // -------------------------------------------------------------------------
    // Acrylic 参数
    // -------------------------------------------------------------------------
    namespace Acrylic {
        inline AcrylicToken light() {
            return { QColor(252, 252, 252), 0.60, 0.22, 30 };
        }
        inline AcrylicToken dark() {
            return { QColor(44, 44, 44), 0.65, 0.16, 30 };
        }
        inline AcrylicToken get(bool isDark) {
            return isDark ? dark() : light();
        }
    }

    // -------------------------------------------------------------------------
    // Mica 参数
    // -------------------------------------------------------------------------
    namespace Mica {
        inline MicaToken light() {
            return { QColor(243, 242, 241), 0.90 };
        }
        inline MicaToken dark() {
            return { QColor(32, 32, 32), 0.90 };
        }
        inline MicaToken get(bool isDark) {
            return isDark ? dark() : light();
        }
    }

    // -------------------------------------------------------------------------
    // Smoke 参数
    // -------------------------------------------------------------------------
    namespace Smoke {
        inline SmokeToken light() {
            return { QColor(0, 0, 0), 0.40 };
        }
        inline SmokeToken dark() {
            return { QColor(0, 0, 0), 0.60 };
        }
        inline SmokeToken get(bool isDark) {
            return isDark ? dark() : light();
        }
    }

    // -------------------------------------------------------------------------
    // 材质类型枚举（用于接口统一访问）
    // -------------------------------------------------------------------------
    enum class Type { Acrylic, Mica, Smoke };
}

#endif // MATERIAL_H
