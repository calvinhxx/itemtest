#include "FlipView.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include "common/Typography.h"

namespace view::collections {

// ── WinUI 3 FlipView 尺寸常量 ───────────────────────────────────────────────
namespace {
    constexpr int kNavBtnW_H = 16;        // 水平模式导航按钮宽度
    constexpr int kNavBtnH_H = 28;        // 水平模式导航按钮高度
    constexpr int kNavBtnW_V = 28;        // 垂直模式导航按钮宽度
    constexpr int kNavBtnH_V = 16;        // 垂直模式导航按钮高度
    constexpr int kNavBtnMargin = 2;      // 导航按钮到边缘距离
    constexpr int kNavBtnRadius = 3;      // 按钮圆角
    constexpr int kIndicatorDotSize = 6;
    constexpr int kIndicatorSpacing = 8;
    constexpr int kIndicatorMargin = 12;
    constexpr int kArrowFontSize = 10;    // Chevron icon size
    constexpr int kGestureThreshold = 50;  // trackpad 累积像素阈值
    // NoScrollPhase 事件间隔 debounce：间隔小于此值的连续事件视为同一手势
    // macOS→RDP→Windows: 触控板事件全部变为 NoScrollPhase，间隔 ~20-30ms
    // 鼠标滚轮：离散 notch 间隔通常 >120ms
    constexpr int kWheelDebounceMs = 100;
}

// ── 覆盖层：在子页面之上绘制导航按钮和指示器 ────────────────────────────────

class FlipViewOverlay : public QWidget {
public:
    explicit FlipViewOverlay(FlipView* parent)
        : QWidget(parent), m_fv(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        auto radius = m_fv->themeRadius();
        const auto& c = m_fv->themeColors();
        qreal r = radius.control;

        // 圆角遮罩：用父控件背景色填充外角（抗锯齿）
        QPainterPath outer;
        outer.addRect(QRectF(rect()));
        QPainterPath inner;
        inner.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), r, r);
        QColor parentBg = c.bgCanvas;
        if (m_fv->parentWidget()) {
            const QPalette& pp = m_fv->parentWidget()->palette();
            if (pp.color(QPalette::Window).alpha() > 0)
                parentBg = pp.color(QPalette::Window);
        }
        p.setPen(Qt::NoPen);
        p.fillPath(outer - inner, parentBg);

        // 圆角边框
        p.setPen(QPen(c.strokeDefault, 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), r, r);

        // 导航按钮
        if (m_fv->m_showNavButtons && m_fv->m_isHovered && m_fv->m_pages.size() > 1) {
            if (m_fv->m_currentIndex > 0)
                m_fv->drawNavButton(p, m_fv->prevButtonRect(), false,
                                    m_fv->m_prevBtnHovered, m_fv->m_prevBtnPressed);
            if (m_fv->m_currentIndex < m_fv->m_pages.size() - 1)
                m_fv->drawNavButton(p, m_fv->nextButtonRect(), true,
                                    m_fv->m_nextBtnHovered, m_fv->m_nextBtnPressed);
        }

        // 页面指示器
        if (m_fv->m_showPageIndicator && m_fv->m_pages.size() > 1)
            m_fv->drawPageIndicator(p);
    }

private:
    FlipView* m_fv;
};

FlipView::FlipView(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_slideAnimation = new QPropertyAnimation(this, "slideOffset", this);
    m_slideAnimation->setDuration(themeAnimation().normal);
    m_slideAnimation->setEasingCurve(themeAnimation().decelerate);

    m_overlay = new FlipViewOverlay(this);
}

void FlipView::onThemeUpdated()
{
    if (m_slideAnimation) {
        m_slideAnimation->setDuration(themeAnimation().normal);
        m_slideAnimation->setEasingCurve(themeAnimation().decelerate);
    }
    update();
}

// ── 页面管理 ─────────────────────────────────────────────────────────────────

void FlipView::addPage(QWidget* page)
{
    insertPage(m_pages.size(), page);
}

void FlipView::insertPage(int index, QWidget* page)
{
    index = qBound(0, index, m_pages.size());
    page->setParent(this);
    m_pages.insert(index, page);

    if (m_currentIndex < 0) {
        m_currentIndex = 0;
    } else if (index <= m_currentIndex) {
        m_currentIndex++;
    }

    layoutPages();
    raiseOverlay();
    update();
}

void FlipView::removePage(int index)
{
    if (index < 0 || index >= m_pages.size()) return;

    QWidget* page = m_pages.takeAt(index);
    page->setParent(nullptr);

    if (m_pages.isEmpty()) {
        m_currentIndex = -1;
    } else if (index < m_currentIndex) {
        m_currentIndex--;
    } else if (index == m_currentIndex) {
        m_currentIndex = qMin(m_currentIndex, m_pages.size() - 1);
    }

    layoutPages();
    update();
    emit currentIndexChanged(m_currentIndex);
}

QWidget* FlipView::pageAt(int index) const
{
    if (index < 0 || index >= m_pages.size()) return nullptr;
    return m_pages.at(index);
}

int FlipView::pageCount() const
{
    return m_pages.size();
}

// ── 属性 ─────────────────────────────────────────────────────────────────────

void FlipView::setCurrentIndex(int index)
{
    if (m_pages.isEmpty()) return;
    index = qBound(0, index, m_pages.size() - 1);
    if (m_currentIndex == index) return;

    int oldIndex = m_currentIndex;
    m_currentIndex = index;
    animateSlide(oldIndex, index);
    emit currentIndexChanged(m_currentIndex);
}

void FlipView::setOrientation(Qt::Orientation orientation)
{
    if (m_orientation == orientation) return;
    m_orientation = orientation;
    layoutPages();
    update();
    emit orientationChanged();
}

void FlipView::setShowNavigationButtons(bool show)
{
    if (m_showNavButtons == show) return;
    m_showNavButtons = show;
    update();
    emit showNavigationButtonsChanged();
}

void FlipView::setShowPageIndicator(bool show)
{
    if (m_showPageIndicator == show) return;
    m_showPageIndicator = show;
    update();
    emit showPageIndicatorChanged();
}

void FlipView::setSlideOffset(qreal offset)
{
    if (qFuzzyCompare(m_slideOffset, offset)) return;
    m_slideOffset = offset;
    layoutPages();
    update();
}

void FlipView::goNext()
{
    if (m_currentIndex < m_pages.size() - 1) {
        setCurrentIndex(m_currentIndex + 1);
    }
}

void FlipView::goPrevious()
{
    if (m_currentIndex > 0) {
        setCurrentIndex(m_currentIndex - 1);
    }
}

QSize FlipView::sizeHint() const
{
    return QSize(400, 270);
}

QSize FlipView::minimumSizeHint() const
{
    return QSize(100, 60);
}

// ── 几何辅助 ─────────────────────────────────────────────────────────────────

QRect FlipView::contentRect() const
{
    return rect();
}

QRect FlipView::prevButtonRect() const
{
    QRect cr = contentRect();
    if (m_orientation == Qt::Horizontal) {
        int y = cr.center().y() - kNavBtnH_H / 2;
        return QRect(cr.left() + kNavBtnMargin, y, kNavBtnW_H, kNavBtnH_H);
    } else {
        int x = cr.center().x() - kNavBtnW_V / 2;
        return QRect(x, cr.top() + kNavBtnMargin, kNavBtnW_V, kNavBtnH_V);
    }
}

QRect FlipView::nextButtonRect() const
{
    QRect cr = contentRect();
    if (m_orientation == Qt::Horizontal) {
        int y = cr.center().y() - kNavBtnH_H / 2;
        return QRect(cr.right() - kNavBtnW_H - kNavBtnMargin, y, kNavBtnW_H, kNavBtnH_H);
    } else {
        int x = cr.center().x() - kNavBtnW_V / 2;
        return QRect(x, cr.bottom() - kNavBtnH_V - kNavBtnMargin, kNavBtnW_V, kNavBtnH_V);
    }
}

QRect FlipView::pageIndicatorRect() const
{
    QRect cr = contentRect();
    int dotCount = m_pages.size();
    if (dotCount <= 0) return QRect();

    if (m_orientation == Qt::Horizontal) {
        int totalW = dotCount * kIndicatorDotSize + (dotCount - 1) * kIndicatorSpacing;
        int x = cr.center().x() - totalW / 2;
        int y = cr.bottom() - kIndicatorMargin - kIndicatorDotSize;
        return QRect(x, y, totalW, kIndicatorDotSize);
    } else {
        int totalH = dotCount * kIndicatorDotSize + (dotCount - 1) * kIndicatorSpacing;
        int x = cr.right() - kIndicatorMargin - kIndicatorDotSize;
        int y = cr.center().y() - totalH / 2;
        return QRect(x, y, kIndicatorDotSize, totalH);
    }
}

// ── 布局 ─────────────────────────────────────────────────────────────────────

void FlipView::layoutPages()
{
    QRect cr = contentRect();
    for (int i = 0; i < m_pages.size(); ++i) {
        QWidget* page = m_pages[i];
        if (i == m_currentIndex) {
            if (m_orientation == Qt::Horizontal) {
                int offsetPx = static_cast<int>(m_slideOffset * cr.width());
                page->setGeometry(cr.x() + offsetPx, cr.y(), cr.width(), cr.height());
            } else {
                int offsetPx = static_cast<int>(m_slideOffset * cr.height());
                page->setGeometry(cr.x(), cr.y() + offsetPx, cr.width(), cr.height());
            }
            page->setVisible(true);
        } else if (i == m_animatingFromIndex && qAbs(m_slideOffset) > 0.001) {
            if (m_orientation == Qt::Horizontal) {
                int dir = (m_animatingFromIndex < m_currentIndex) ? -1 : 1;
                int offsetPx = static_cast<int>((m_slideOffset + dir) * cr.width());
                page->setGeometry(cr.x() + offsetPx, cr.y(), cr.width(), cr.height());
            } else {
                int dir = (m_animatingFromIndex < m_currentIndex) ? -1 : 1;
                int offsetPx = static_cast<int>((m_slideOffset + dir) * cr.height());
                page->setGeometry(cr.x(), cr.y() + offsetPx, cr.width(), cr.height());
            }
            page->setVisible(true);
        } else {
            page->setVisible(false);
        }
    }
}

// ── 动画 ─────────────────────────────────────────────────────────────────────

void FlipView::animateSlide(int fromIndex, int toIndex)
{
    m_animatingFromIndex = fromIndex;
    m_slideAnimation->stop();

    qreal startOffset = (toIndex > fromIndex) ? 1.0 : -1.0;
    m_slideAnimation->setStartValue(startOffset);
    m_slideAnimation->setEndValue(0.0);
    m_slideAnimation->start();
}

// ── 绘制 ─────────────────────────────────────────────────────────────────────

void FlipView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& c = themeColors();
    auto radius = themeRadius();

    // ── 背景 ──
    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(rect()), radius.control, radius.control);
    p.setClipPath(bgPath);
    p.fillRect(rect(), c.bgSolid);

    // 导航按钮和指示器由 FlipViewOverlay 在子页面之上绘制
}

void FlipView::drawNavButton(QPainter& p, const QRect& btnRect, bool isNext,
                              bool hovered, bool pressed)
{
    const auto& c = themeColors();

    // 背景 — AcrylicInApp 近似
    QColor bgColor;
    if (pressed) {
        bgColor = c.controlTertiary;
    } else if (hovered) {
        bgColor = c.controlSecondary;
    } else {
        bgColor = c.controlDefault;
    }
    bgColor.setAlpha(230);

    QPainterPath btnPath;
    btnPath.addRoundedRect(QRectF(btnRect), kNavBtnRadius, kNavBtnRadius);
    p.setPen(Qt::NoPen);
    p.setBrush(bgColor);
    p.drawPath(btnPath);

    // WinUI FlipViewButtonBorderThemeThickness=0，按钮无描边

    // 箭头
    QColor arrowColor = (pressed || hovered) ? c.textSecondary : c.strokeStrong;

    // FlipViewButtonScalePressed=0.875: 按压时箭头缩小
    int arrowSize = pressed ? static_cast<int>(kArrowFontSize * 0.875) : kArrowFontSize;
    QFont iconFont(Typography::FontFamily::SegoeFluentIcons);
    iconFont.setPixelSize(arrowSize);
    p.setFont(iconFont);
    p.setPen(arrowColor);

    // 使用 Chevron iconfont
    QString glyph;
    if (m_orientation == Qt::Horizontal) {
        glyph = isNext ? Typography::Icons::ChevronRight : Typography::Icons::ChevronLeft;
    } else {
        glyph = isNext ? Typography::Icons::ChevronDown : Typography::Icons::ChevronUp;
    }
    p.drawText(btnRect, Qt::AlignCenter, glyph);
}

void FlipView::drawPageIndicator(QPainter& p)
{
    const auto& c = themeColors();
    QRect indRect = pageIndicatorRect();
    if (indRect.isNull()) return;

    for (int i = 0; i < m_pages.size(); ++i) {
        QRect dotRect;
        if (m_orientation == Qt::Horizontal) {
            int x = indRect.x() + i * (kIndicatorDotSize + kIndicatorSpacing);
            dotRect = QRect(x, indRect.y(), kIndicatorDotSize, kIndicatorDotSize);
        } else {
            int y = indRect.y() + i * (kIndicatorDotSize + kIndicatorSpacing);
            dotRect = QRect(indRect.x(), y, kIndicatorDotSize, kIndicatorDotSize);
        }
        QColor dotColor = (i == m_currentIndex) ? c.textPrimary : c.textTertiary;
        p.setPen(Qt::NoPen);
        p.setBrush(dotColor);
        p.drawEllipse(dotRect);
    }
}

// ── 事件处理 ─────────────────────────────────────────────────────────────────

void FlipView::updateMask()
{
    // 不使用 setMask（QRegion 像素级裁剪无抗锯齿）
    // 圆角裁剪由 paintEvent clip + overlay 遮罩实现
    clearMask();
}

void FlipView::raiseOverlay()
{
    if (m_overlay) {
        m_overlay->raise();
    }
}

void FlipView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_overlay) {
        m_overlay->setGeometry(rect());
    }
    updateMask();
    layoutPages();
}

void FlipView::enterEvent(QEnterEvent* event)
{
    m_isHovered = true;
    update();
    QWidget::enterEvent(event);
}

void FlipView::leaveEvent(QEvent* event)
{
    m_isHovered = false;
    m_prevBtnHovered = false;
    m_nextBtnHovered = false;
    m_prevBtnPressed = false;
    m_nextBtnPressed = false;
    update();
    QWidget::leaveEvent(event);
}

void FlipView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        if (m_showNavButtons && m_isHovered && m_pages.size() > 1) {
            if (m_currentIndex > 0 && prevButtonRect().contains(pos)) {
                m_prevBtnPressed = true;
                update();
                return;
            }
            if (m_currentIndex < m_pages.size() - 1 && nextButtonRect().contains(pos)) {
                m_nextBtnPressed = true;
                update();
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void FlipView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        if (m_prevBtnPressed) {
            m_prevBtnPressed = false;
            if (prevButtonRect().contains(pos)) {
                goPrevious();
            }
            update();
            return;
        }
        if (m_nextBtnPressed) {
            m_nextBtnPressed = false;
            if (nextButtonRect().contains(pos)) {
                goNext();
            }
            update();
            return;
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void FlipView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    bool prevHovered = false;
    bool nextHovered = false;

    if (m_showNavButtons && m_isHovered && m_pages.size() > 1) {
        if (m_currentIndex > 0) {
            prevHovered = prevButtonRect().contains(pos);
        }
        if (m_currentIndex < m_pages.size() - 1) {
            nextHovered = nextButtonRect().contains(pos);
        }
    }

    if (prevHovered != m_prevBtnHovered || nextHovered != m_nextBtnHovered) {
        m_prevBtnHovered = prevHovered;
        m_nextBtnHovered = nextHovered;
        setCursor((prevHovered || nextHovered) ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void FlipView::wheelEvent(QWheelEvent* event)
{
    if (m_pages.size() <= 1) {
        QWidget::wheelEvent(event);
        return;
    }

    const auto phase = event->phase();

    // ── Phase-based: trackpad 手势 (Begin → Update → Momentum → End) ──
    // macOS 原生 / Windows 精密触控板 (WM_POINTER) 提供完整 phase 链
    if (phase == Qt::ScrollBegin) {
        if (m_slideAnimation->state() == QAbstractAnimation::Running) {
            event->accept(); return;
        }
        m_gestureAccum = 0;
        m_gestureConsumed = false;
        event->accept();
        return;
    }
    if (phase == Qt::ScrollMomentum || phase == Qt::ScrollEnd) {
        event->accept();
        return;
    }
    if (phase == Qt::ScrollUpdate) {
        if (m_slideAnimation->state() == QAbstractAnimation::Running) {
            event->accept(); return;
        }
        if (m_gestureConsumed) { event->accept(); return; }
        int delta = !event->pixelDelta().isNull()
            ? (m_orientation == Qt::Horizontal ? event->pixelDelta().x() : event->pixelDelta().y())
            : (m_orientation == Qt::Horizontal
                ? event->angleDelta().x() + event->angleDelta().y()
                : event->angleDelta().y());
        m_gestureAccum += delta;
        if (qAbs(m_gestureAccum) >= kGestureThreshold) {
            (m_gestureAccum > 0) ? goPrevious() : goNext();
            m_gestureConsumed = true;
        }
        event->accept();
        return;
    }

    // ── NoScrollPhase: 鼠标滚轮 / Mac→RDP→Windows 转发的触控板事件 ──
    // RDP 将 Mac 触控板手势（含惯性）转译为密集 WM_MOUSEWHEEL 流，全部 NoScrollPhase。
    // 通过事件间隔判断手势边界：间隔 < debounce 阈值视为同一手势，只翻一次。
    const qint64 sinceLast = m_wheelCooldown.isValid() ? m_wheelCooldown.elapsed() : 10000;
    m_wheelCooldown.start(); // 每次事件都更新（包括动画期间），保持 debounce 窗口滑动

    if (m_slideAnimation->state() == QAbstractAnimation::Running) {
        event->accept();
        return;
    }

    if (sinceLast < kWheelDebounceMs) {
        event->accept();
        return;
    }

    // 新手势（首次事件或间隔已超 debounce）→ 翻页
    int delta = (m_orientation == Qt::Horizontal)
                    ? event->angleDelta().x() + event->angleDelta().y()
                    : event->angleDelta().y();
    if (delta > 0) {
        goPrevious();
    } else if (delta < 0) {
        goNext();
    }
    event->accept();
}

void FlipView::keyPressEvent(QKeyEvent* event)
{
    if (m_orientation == Qt::Horizontal) {
        if (event->key() == Qt::Key_Left) { goPrevious(); return; }
        if (event->key() == Qt::Key_Right) { goNext(); return; }
    } else {
        if (event->key() == Qt::Key_Up) { goPrevious(); return; }
        if (event->key() == Qt::Key_Down) { goNext(); return; }
    }
    QWidget::keyPressEvent(event);
}

} // namespace view::collections
