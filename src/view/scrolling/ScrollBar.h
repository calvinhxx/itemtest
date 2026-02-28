#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QShowEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QMargins>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::scrolling {

/**
 * @brief ScrollBar - 基于 Fluent Design 的自绘滚动条
 *
 * 使用 FluentElement / QMLPlus 提供的主题与 Anchors 能力，
 * 通过 thickness 属性控制厚度，并在 Hover / Press 等交互状态下切换视觉。
 */
class ScrollBar : public QScrollBar, public FluentElement, public view::QMLPlus {
    Q_OBJECT

    /** @brief 滚动条的厚度（垂直时为宽度，水平时为高度，单位：像素） */
    Q_PROPERTY(int thickness READ thickness WRITE setThickness NOTIFY thicknessChanged)
    /** @brief 当前不透明度（0.0~1.0），用于淡入/淡出动画的 overlay 效果 */
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);
    explicit ScrollBar(QWidget *parent = nullptr);
    ~ScrollBar() override;

    int thickness() const { return m_thickness; }
    void setThickness(int thickness);

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal value);

signals:
    void thicknessChanged();

protected:
    // 尺寸提示：厚度由 thickness 控制，长度沿用 QScrollBar 默认逻辑
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void paintEvent(QPaintEvent *event) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;

    void onThemeUpdated() override;

private:
    void init();

    // --- 尺寸与视觉参数（避免在 cpp 中硬编码 magic number） ---
    int      m_thickness      = 7;                     ///< 控件实际厚度（像素），始终占位此宽度/高度
    int      m_minThumbLength = 16;                    ///< Thumb 的最小长度（像素）
    QMargins m_thumbPadding   = QMargins(1, 1, 1, 1);  ///< Thumb 与背景轨道的左右/上下间距

    bool m_isHovered = false;
    bool m_isPressed = false;

    // Overlay 淡入/淡出
    qreal               m_opacity       = 0.0;
    QPropertyAnimation* m_opacityAnim   = nullptr;
    QTimer*             m_autoHideTimer = nullptr;

    void ensureAnimation();
    void showWithAutoHide();
};

} // namespace view::scrolling

#endif // SCROLLBAR_H
