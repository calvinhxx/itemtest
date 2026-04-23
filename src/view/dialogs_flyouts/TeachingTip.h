#ifndef TEACHINGTIP_H
#define TEACHINGTIP_H

#include <QMargins>
#include <QPointer>
#include <QSize>

#include "view/dialogs_flyouts/Popup.h"

class QKeyEvent;
class QPaintEvent;
class QResizeEvent;
class QWidget;

namespace view::dialogs_flyouts {

/**
 * @brief TeachingTip — Fluent Design 教学提示浮层（轻量基础层）
 *
 * 职责：
 *  - 带气泡箭头（tail）的卡片外壳 + 阴影自绘
 *  - 12 方向 placement + Auto 回退 + 窗口边界夹紧
 *  - 追踪 target 控件的 Move / Resize / Hide / Destroy 生命周期
 *  - CloseReason 语义管线（closing 信号）
 *  - 默认 NoAutoClose（非 light-dismiss）
 *
 * 内容由调用方自行组装：通过 contentHost() 获取卡片内容区域容器，
 * 在其上建立 AnchorLayout 并添加子控件，不需要通过本类属性管理。
 *
 * 尺寸控制：
 *  - setCardSize(QSize) 设置卡片内容区域尺寸，默认 360×200。
 *  - TeachingTip widget 尺寸 = cardSize + 2×shadowMargin + tailInsets，自动计算。
 */
class TeachingTip : public Popup {
    Q_OBJECT
    Q_PROPERTY(QWidget* target READ target WRITE setTarget)
    Q_PROPERTY(PreferredPlacement preferredPlacement READ preferredPlacement WRITE setPreferredPlacement)
    Q_PROPERTY(int placementMargin READ placementMargin WRITE setPlacementMargin)
    Q_PROPERTY(bool lightDismissEnabled READ isLightDismissEnabled WRITE setLightDismissEnabled)
    Q_PROPERTY(bool tailVisible READ isTailVisible WRITE setTailVisible)

public:
    enum PreferredPlacement {
        Auto,
        Top,
        TopLeft,
        TopRight,
        Bottom,
        BottomLeft,
        BottomRight,
        Left,
        LeftTop,
        LeftBottom,
        Right,
        RightTop,
        RightBottom,
    };
    Q_ENUM(PreferredPlacement)

    enum CloseReason {
        Programmatic,
        ActionButton,
        CloseButton,
        LightDismiss,
        TargetDestroyed,
    };
    Q_ENUM(CloseReason)

    explicit TeachingTip(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    QWidget* target() const { return m_target.data(); }
    void setTarget(QWidget* target);

    PreferredPlacement preferredPlacement() const { return m_preferredPlacement; }
    void setPreferredPlacement(PreferredPlacement placement);

    int placementMargin() const { return m_placementMargin; }
    void setPlacementMargin(int margin);

    bool isLightDismissEnabled() const { return m_lightDismissEnabled; }
    void setLightDismissEnabled(bool enabled);

    bool isTailVisible() const { return m_tailVisible; }
    void setTailVisible(bool visible);

    /// 卡片内容区域容器。向此 widget 添加子控件（AnchorLayout 等）。
    /// 几何随 placement / cardSize 自动更新，无需手动管理。
    QWidget* contentHost() const { return m_contentHost; }

    /// 设置卡片内容区域尺寸（不含阴影和气泡箭头）。默认 360×200。
    void setCardSize(const QSize& size);
    QSize cardSize() const { return m_cardSizeHint; }

    void showAt(QWidget* target);
    void closeWithReason(CloseReason reason);

signals:
    void closing(CloseReason reason);

protected:
    QPoint computePosition() const override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void syncContentHostGeometry();
    void updateWidgetSize();

    QRect cardRect() const;
    QRect targetRectInTopLevel() const;
    PreferredPlacement resolvedPlacement() const;
    PreferredPlacement resolveAutoPlacement(const QSize& cardSize) const;
    QPoint cardTopLeftForPlacement(PreferredPlacement placement, const QRect& targetRect,
                                   const QSize& cardSize) const;
    QPoint clampCardTopLeft(const QPoint& cardTopLeft, const QSize& cardSize) const;
    QPoint widgetTopLeftForCardTopLeft(const QPoint& cardTopLeft,
                                       PreferredPlacement placement) const;
    QMargins tailInsets(PreferredPlacement placement) const;

    void markPendingCloseReason(CloseReason reason);
    void emitClosingReason();

    QWidget* m_contentHost = nullptr;
    QPointer<QWidget> m_target;
    QPointer<QWidget> m_targetTopLevel;
    PreferredPlacement m_preferredPlacement = Auto;
    int m_placementMargin = 4;
    bool m_lightDismissEnabled = false;
    bool m_tailVisible = true;
    QSize m_cardSizeHint = QSize(360, 200);

    CloseReason m_pendingCloseReason = Programmatic;
    bool m_closeReasonExplicit = false;
};

} // namespace view::dialogs_flyouts

#endif // TEACHINGTIP_H