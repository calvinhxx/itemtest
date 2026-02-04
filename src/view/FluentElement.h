#ifndef FLUENTELEMENT_H
#define FLUENTELEMENT_H

#include <QColor>
#include <QFont>
#include <QString>
#include <QEasingCurve>
#include "common/Elevation.h"
#include "common/Animation.h"

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

        // Text Colors
        QColor textPrimary, textSecondary, textTertiary, textDisabled, textOnAccent;

        // Backgrounds & Neutrals
        QColor bgCanvas, bgLayer, bgSolid;
        QColor grey10, grey20, grey30, grey40, grey50, grey60, grey90, grey130, grey160, grey190;

        // Charts
        QList<QColor> charts;
    };

    struct FontStyle {
        QString family;
        int size;
        int weight;
        QFont toQFont() const {
            QFont font(family, size, weight);
            font.setPixelSize(size);
            return font;
        }
    };

    struct Radius {
        int none, overlay, control, small, medium, inPage, topLevel, large;
    };

    struct Spacing {
        struct { int controlH, controlV, card, dialog; } padding;
        struct { int tight, normal, loose, section; } gap;
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
    QString themeMaterial(const QString& type = "Acrylic") const;
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
