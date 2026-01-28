#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>
#include <QColor>

/**
 * @brief Material - Windows UI Kit 材质系统
 * 包括 Acrylic（透明）、Mica（非透明）、Smoke（模态背景）等材质效果
 */
namespace Material {
    /**
     * @brief 材质类型
     */
    enum Type {
        Acrylic,    // 透明材质（毛玻璃效果）
        Mica,       // 非透明材质（云母效果）
        Smoke       // 模态背景（烟雾效果）
    };

    /**
     * @brief Acrylic 材质参数
     * 透明材质，具有毛玻璃模糊效果
     */
    namespace AcrylicParams {
        // Light 主题
        namespace Light {
            const double TintOpacity = 0.8;        // 色调不透明度
            const double LuminosityOpacity = 0.6;  // 亮度不透明度
            const int BlurRadius = 30;             // 模糊半径
            const QColor TintColor = QColor(243, 242, 241); // 浅色色调
        }

        // Dark 主题
        namespace Dark {
            const double TintOpacity = 0.8;
            const double LuminosityOpacity = 0.6;
            const int BlurRadius = 30;
            const QColor TintColor = QColor(32, 32, 32);    // 深色色调
        }

        /**
         * @brief 获取 Acrylic 样式表
         * 注意：Qt 的 QSS 对背景模糊支持有限，可能需要使用 QGraphicsBlurEffect
         */
        inline QString getStyleSheet(bool isDark = false) {
            if (isDark) {
                return QString("background-color: rgba(%1, %2, %3, %4);")
                    .arg(Dark::TintColor.red())
                    .arg(Dark::TintColor.green())
                    .arg(Dark::TintColor.blue())
                    .arg(static_cast<int>(Dark::TintOpacity * 255));
            } else {
                return QString("background-color: rgba(%1, %2, %3, %4);")
                    .arg(Light::TintColor.red())
                    .arg(Light::TintColor.green())
                    .arg(Light::TintColor.blue())
                    .arg(static_cast<int>(Light::TintOpacity * 255));
            }
        }
    }

    /**
     * @brief Mica 材质参数
     * 非透明材质，具有云母纹理效果
     */
    namespace MicaParams {
        // Light 主题
        namespace Light {
            const QColor BaseColor = QColor(243, 242, 241);  // 基础颜色
            const double Opacity = 0.9;                      // 不透明度
        }

        // Dark 主题
        namespace Dark {
            const QColor BaseColor = QColor(32, 32, 32);
            const double Opacity = 0.9;
        }

        /**
         * @brief 获取 Mica 样式表
         */
        inline QString getStyleSheet(bool isDark = false) {
            if (isDark) {
                return QString("background-color: rgba(%1, %2, %3, %4);")
                    .arg(Dark::BaseColor.red())
                    .arg(Dark::BaseColor.green())
                    .arg(Dark::BaseColor.blue())
                    .arg(static_cast<int>(Dark::Opacity * 255));
            } else {
                return QString("background-color: rgba(%1, %2, %3, %4);")
                    .arg(Light::BaseColor.red())
                    .arg(Light::BaseColor.green())
                    .arg(Light::BaseColor.blue())
                    .arg(static_cast<int>(Light::Opacity * 255));
            }
        }
    }

    /**
     * @brief Smoke 材质参数
     * 模态背景，用于对话框、弹窗等场景
     */
    namespace SmokeParams {
        // Light 主题
        namespace Light {
            const QColor BaseColor = QColor(0, 0, 0);         // 黑色基础
            const double Opacity = 0.4;                        // 40% 不透明度
        }

        // Dark 主题
        namespace Dark {
            const QColor BaseColor = QColor(0, 0, 0);
            const double Opacity = 0.6;                        // 60% 不透明度（暗色主题需要更高）
        }

        /**
         * @brief 获取 Smoke 样式表
         */
        inline QString getStyleSheet(bool isDark = false) {
            if (isDark) {
                return QString("background-color: rgba(%1, %2, %3, %4);")
                    .arg(Dark::BaseColor.red())
                    .arg(Dark::BaseColor.green())
                    .arg(Dark::BaseColor.blue())
                    .arg(static_cast<int>(Dark::Opacity * 255));
            } else {
                return QString("background-color: rgba(%1, %2, %3, %4);")
                    .arg(Light::BaseColor.red())
                    .arg(Light::BaseColor.green())
                    .arg(Light::BaseColor.blue())
                    .arg(static_cast<int>(Light::Opacity * 255));
            }
        }
    }
}

#endif // MATERIAL_H
