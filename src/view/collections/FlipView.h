#ifndef FLIPVIEW_H
#define FLIPVIEW_H

#include <QWidget>
#include <QElapsedTimer>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QPropertyAnimation;

namespace view::collections {

/**
 * @brief FlipView - WinUI 3 风格翻页视图控件
 *
 * 一次显示一个页面（QWidget），支持水平/垂直方向翻页、
 * 导航按钮（悬停时显示）、页面指示器圆点、滑动动画。
 *
 * 使用方式：调用 addPage() 添加 QWidget 页面。
 */
class FlipView : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(bool showNavigationButtons READ showNavigationButtons WRITE setShowNavigationButtons NOTIFY showNavigationButtonsChanged)
    Q_PROPERTY(bool showPageIndicator READ showPageIndicator WRITE setShowPageIndicator NOTIFY showPageIndicatorChanged)
    Q_PROPERTY(qreal slideOffset READ slideOffset WRITE setSlideOffset)

    friend class FlipViewOverlay;

public:
    explicit FlipView(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    // ── 页面管理 ──
    void addPage(QWidget* page);
    void insertPage(int index, QWidget* page);
    void removePage(int index);
    QWidget* pageAt(int index) const;
    int pageCount() const;

    // ── 属性 ──
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);

    Qt::Orientation orientation() const { return m_orientation; }
    void setOrientation(Qt::Orientation orientation);

    bool showNavigationButtons() const { return m_showNavButtons; }
    void setShowNavigationButtons(bool show);

    bool showPageIndicator() const { return m_showPageIndicator; }
    void setShowPageIndicator(bool show);

    qreal slideOffset() const { return m_slideOffset; }
    void setSlideOffset(qreal offset);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    // ── 导航 ──
    void goNext();
    void goPrevious();

signals:
    void currentIndexChanged(int index);
    void orientationChanged();
    void showNavigationButtonsChanged();
    void showPageIndicatorChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    // ── 几何 ──
    QRect contentRect() const;
    QRect prevButtonRect() const;
    QRect nextButtonRect() const;
    QRect pageIndicatorRect() const;

    // ── 内部 ──
    void layoutPages();
    void animateSlide(int fromIndex, int toIndex);
    void drawNavButton(QPainter& p, const QRect& rect, bool isNext, bool hovered, bool pressed);
    void drawPageIndicator(QPainter& p);
    void updateMask();
    void raiseOverlay();

    QWidget* m_overlay = nullptr;
    QList<QWidget*> m_pages;
    int m_currentIndex = -1;
    Qt::Orientation m_orientation = Qt::Horizontal;
    bool m_showNavButtons = true;
    bool m_showPageIndicator = true;

    // 动画
    qreal m_slideOffset = 0.0;
    QPropertyAnimation* m_slideAnimation = nullptr;
    int m_animatingFromIndex = -1;

    // 悬停状态
    bool m_isHovered = false;
    bool m_prevBtnHovered = false;
    bool m_nextBtnHovered = false;
    bool m_prevBtnPressed = false;
    bool m_nextBtnPressed = false;

    // 滚轮/触控板
    QElapsedTimer m_wheelCooldown;
    int m_gestureAccum = 0;
    bool m_gestureConsumed = false;
};

} // namespace view::collections

#endif // FLIPVIEW_H
