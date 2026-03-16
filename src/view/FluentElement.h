#ifndef FLUENTELEMENT_H
#define FLUENTELEMENT_H

#include <QColor>
#include <QFont>
#include <QString>
#include <QEasingCurve>
#include "common/Elevation.h"
#include "common/Animation.h"
#include "common/Material.h"

class FluentElementPrivate;

/**
 * @brief FluentElement - Fluent Design System 的抽象访问入口
 * Mixin 模式：UI 组件通过继承此类获得访问设计元素的能力。
 * 使用 PImpl 模式隐藏内部管理逻辑。
 */
class FluentElement {
public:
    enum Theme { Light, Dark };

    // --- 设计元素数据结构 ---
    
    struct Colors {
        // Fill Colors
        QColor accentDefault, accentSecondary, accentTertiary, accentDisabled;
        QColor controlDefault, controlSecondary, controlTertiary, controlDisabled;
        QColor controlAltSecondary, controlAltTertiary;
        QColor subtleTransparent, subtleSecondary, subtleTertiary;

        // Stroke Colors
        QColor strokeDefault, strokeSecondary, strokeStrong;
        QColor strokeCard, strokeDivider, strokeSurface;
        QColor strokeFocusOuter, strokeFocusInner;  // 焦点环双层描边

        // Text Colors
        QColor textPrimary, textSecondary, textTertiary, textDisabled;
        QColor textOnAccent;        // 强调色背景上的文字（白/黑）
        QColor textAccentPrimary;   // 普通背景上的强调色文字（深蓝/亮蓝）

        // Backgrounds & Neutrals
        QColor bgCanvas, bgLayer, bgLayerAlt, bgSolid;
        QColor grey10, grey20, grey30, grey40, grey50, grey60, grey90, grey130, grey160, grey190;

        // System / Semantic Colors
        QColor systemCritical,    systemCriticalBg;
        QColor systemCaution,     systemCautionBg;
        QColor systemInfo,        systemInfoBg;
        QColor systemSuccess,     systemSuccessBg;

        // Charts
        QList<QColor> charts;
    };

    struct FontStyle {
        QString family;
        QString styleName;   // Segoe UI Variable 光学尺寸变体，如 "Text Regular"
        int size;
        int weight;
        int lineHeight;      // 绝对行高（px），来自 Figma MCP 实测值
        QFont toQFont() const {
            QFont font(family, -1, weight);
            font.setPixelSize(size);
            if (!styleName.isEmpty())
                font.setStyleName(styleName);
            return font;
        }
    };

    struct Radius {
        int none;     // 0  直角
        int control;  // 4  页面内控件
        int overlay;  // 8  浮层容器（Dialog、Tooltip、Flyout）
    };

    struct Spacing {
        struct {
            int controlH, controlV;
            int card, dialog;
            int textFieldH, textFieldV;   // 输入框内边距
            int listItemH, listItemV;     // 列表项内边距
        } padding;
        struct { int tight, normal, loose, section; } gap;
        struct { int small, standard, large; } controlHeight;  // 标准控件高度
        int xSmall, small, medium, standard, large, xLarge, xxLarge;
    };

    struct Animation {
        // Durations (ms)
        int fast, normal, slow, verySlow;
        // Easings
        QEasingCurve standard, accelerate, decelerate, entrance, exit;
    };

    // --- 静态全局管理 ---
    static void setTheme(Theme theme);
    static Theme currentTheme();

    // --- 组件访问接口 ---
    Colors themeColors() const;
    FontStyle themeFont(const QString& styleName = "Body") const;
    Radius themeRadius() const;
    Spacing themeSpacing() const;
    Animation themeAnimation() const;
    
    // 材质与阴影
    Material::AcrylicToken themeAcrylic() const;
    Material::MicaToken    themeMica()    const;
    Material::SmokeToken   themeSmoke()   const;
    Elevation::ShadowParams themeShadow(Elevation::Level level) const;
    int themeBreakpoint(const QString& size = "Medium") const;

    /**
     * @brief 主题更新回调
     * 当全局主题变化时，所有继承自此类的活跃实例都会被调用。
     */
    virtual void onThemeUpdated() {}

protected:
    FluentElement();
    virtual ~FluentElement();

    FluentElementPrivate* d_ptr; // PImpl 指针

private:
    Q_DISABLE_COPY(FluentElement)
};

#endif // FLUENTELEMENT_H
