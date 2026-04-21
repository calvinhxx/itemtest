#ifndef POPUP_H
#define POPUP_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPointer>
#include <QFlags>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QGraphicsOpacityEffect;

namespace view::dialogs_flyouts {

/**
 * @brief Popup — Fluent Design 浮层基类
 *
 * 对标 QML Popup：通过相对某个 widget 的局部坐标定位，open() 时 reparent 到 topLevelWidget overlay。
 * 不提供 placementTarget / Placement 枚举 — 那是子类 Flyout 的职责。
 *
 * 核心能力：
 *  - 相对于某个 widget 的局部坐标定位（未设置则居中）
 *  - modal + dim（可选 Scrim 蒙层）
 *  - closePolicy（CloseOnPressOutside / CloseOnEscape）
 *  - 进/出场动画：opacity 0→1
 */
class Popup : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT

public:
    enum CloseFlag {
        NoAutoClose         = 0,
        CloseOnPressOutside = 1 << 0,
        CloseOnEscape       = 1 << 1,
    };
    Q_DECLARE_FLAGS(ClosePolicy, CloseFlag)
    Q_FLAG(ClosePolicy)

    Q_PROPERTY(bool isOpen READ isOpen WRITE setIsOpen NOTIFY isOpenChanged)
    Q_PROPERTY(ClosePolicy closePolicy READ closePolicy WRITE setClosePolicy)
    Q_PROPERTY(bool modal READ isModal WRITE setModal)
    Q_PROPERTY(bool dim READ isDim WRITE setDim)
    Q_PROPERTY(bool animationEnabled READ isAnimationEnabled WRITE setAnimationEnabled)
    Q_PROPERTY(double popupProgress READ popupProgress WRITE setPopupProgress NOTIFY popupProgressChanged)

public:
    explicit Popup(QWidget* parent = nullptr);
    ~Popup() override;

    void onThemeUpdated() override;

    bool isOpen() const { return m_isOpen; }

    ClosePolicy closePolicy() const { return m_closePolicy; }
    void setClosePolicy(ClosePolicy p) { m_closePolicy = p; }

    bool isModal() const { return m_modal; }
    void setModal(bool m) { m_modal = m; }

    bool isDim() const { return m_dim; }
    void setDim(bool d) { m_dim = d; }

    bool isAnimationEnabled() const { return m_animationEnabled; }
    void setAnimationEnabled(bool e) { m_animationEnabled = e; }

    double popupProgress() const { return m_popupProgress; }
    void   setPopupProgress(double p);

    /// 相对于指定 widget 的局部坐标定位 popup 左上角。
    /// 若 relativeTo 为 topLevelWidget，则等价于相对于顶层窗口定位。
    /// 坐标以可见卡片左上角为准，open() 内部自动补偿阴影 margin。
    /// 未调用则 open() 时自动居中。
    void setPosition(QWidget* relativeTo, const QPoint& localPos);

public slots:
    void open();
    void close();
    void setIsOpen(bool open);

signals:
    void isOpenChanged(bool open);
    void opened();
    void closed();
    void aboutToShow();
    void aboutToHide();
    void popupProgressChanged(double progress);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    /// 子类可重写以自定义 open() 时的定位策略（Flyout 会重写）
    virtual QPoint computePosition() const;

private:
    void  startEnterAnimation();
    void  startExitAnimation();
    void  finalizeOpened();
    void  finalizeClosed();

    void  ensureScrim();
    void  destroyScrim();

    QWidget* originalParentTopLevel() const;

    // ── State ───────────────────────────────────────────────────────────────
    bool m_isOpen = false;
    bool m_isClosing = false;

    QPointer<QWidget> m_originalParent;

    ClosePolicy m_closePolicy = ClosePolicy(CloseOnPressOutside | CloseOnEscape);
    bool m_modal = false;
    bool m_dim   = false;
    bool m_animationEnabled = true;
    bool m_positionSet = false;
    QPoint m_targetPos;

    double m_popupProgress = 0.0;
    QPropertyAnimation* m_anim = nullptr;
    QGraphicsOpacityEffect* m_opacityEffect = nullptr;

    QPointer<QWidget> m_scrim;
};

} // namespace view::dialogs_flyouts

Q_DECLARE_OPERATORS_FOR_FLAGS(view::dialogs_flyouts::Popup::ClosePolicy)

#endif // POPUP_H
