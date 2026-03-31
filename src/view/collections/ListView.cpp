#include "ListView.h"

#include <algorithm>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShowEvent>
#include <QTimer>
#include <QVariantAnimation>
#include <QWheelEvent>

#include "common/Animation.h"
#include "common/CornerRadius.h"
#include "common/Spacing.h"
#include "common/Typography.h"
#include "view/scrolling/ScrollBar.h"

namespace view::collections {

ListView::ListView(QWidget* parent)
    : QListView(parent) {

    m_fontRole = Typography::FontRole::Body;

    setFrameStyle(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);

    QListView::setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, &QAbstractItemView::clicked, this, [this](const QModelIndex& idx) {
        emit itemClicked(idx.row());
    });

    // --- Header label ---
    m_headerLabel = new QLabel(this);
    m_headerLabel->hide();
    m_headerLabel->setIndent(::Spacing::Padding::ListItemHorizontal);

    // --- Fluent scroll bar ---
    m_vScrollBar = new ::view::scrolling::ScrollBar(Qt::Vertical, this);
    m_vScrollBar->setObjectName(QStringLiteral("fluentListViewScrollBar"));
    m_vScrollBar->hide();

    auto* nativeVBar = verticalScrollBar();
    // Bidirectional value sync (QAbstractSlider::setValue is no-op when value unchanged)
    connect(nativeVBar,  &QScrollBar::valueChanged, m_vScrollBar, &QScrollBar::setValue);
    connect(m_vScrollBar, &QScrollBar::valueChanged, nativeVBar,  &QScrollBar::setValue);
    // Range/visibility/position sync
    connect(nativeVBar, &QScrollBar::rangeChanged, this, &ListView::syncFluentScrollBar);

    // --- Overscroll bounce ---
    m_bounceAnim = new QVariantAnimation(this);
    m_bounceAnim->setDuration(300);
    m_bounceAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_bounceAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        m_overscrollY = v.toReal();
        viewport()->update();
    });

    m_bounceTimer = new QTimer(this);
    m_bounceTimer->setSingleShot(true);
    m_bounceTimer->setInterval(150);
    connect(m_bounceTimer, &QTimer::timeout, this, &ListView::startBounceBack);

    syncFluentScrollBar();
    onThemeUpdated();
}

// ── Selection mode ────────────────────────────────────────────────────────────

void ListView::setSelectionMode(ListSelectionMode mode) {
    if (m_selectionMode == mode) return;
    m_selectionMode = mode;

    switch (mode) {
    case ListSelectionMode::None:
        QListView::setSelectionMode(QAbstractItemView::NoSelection);
        break;
    case ListSelectionMode::Single:
        QListView::setSelectionMode(QAbstractItemView::SingleSelection);
        break;
    case ListSelectionMode::Multiple:
        QListView::setSelectionMode(QAbstractItemView::MultiSelection);
        break;
    case ListSelectionMode::Extended:
        QListView::setSelectionMode(QAbstractItemView::ExtendedSelection);
        break;
    }
    emit selectionModeChanged();
}

// ── Appearance properties ────────────────────────────────────────────────────

void ListView::setFontRole(const QString& role) {
    if (m_fontRole == role) return;
    m_fontRole = role;
    applyThemeStyle();
    emit fontRoleChanged();
}

void ListView::setBorderVisible(bool visible) {
    if (m_borderVisible == visible) return;
    m_borderVisible = visible;
    update();
    emit borderVisibleChanged();
}

void ListView::setBackgroundVisible(bool visible) {
    if (m_backgroundVisible == visible) return;
    m_backgroundVisible = visible;
    if (viewport()) viewport()->update();
    emit backgroundVisibleChanged();
}

void ListView::setHeaderText(const QString& text) {
    if (m_headerText == text) return;
    m_headerText = text;
    if (m_headerLabel) {
        m_headerLabel->setText(text);
        m_headerLabel->setVisible(!text.isEmpty());
    }
    updateViewportMargins();
    layoutHeader();
    emit headerTextChanged();
}

void ListView::setPlaceholderText(const QString& text) {
    if (m_placeholderText == text) return;
    m_placeholderText = text;
    if (viewport()) viewport()->update();
    emit placeholderTextChanged();
}

// ── Selection API ─────────────────────────────────────────────────────────────

int ListView::selectedIndex() const {
    auto idxList = selectionModel()->selectedIndexes();
    return idxList.isEmpty() ? -1 : idxList.first().row();
}

QList<int> ListView::selectedRows() const {
    QSet<int> seen;
    for (const auto& idx : selectionModel()->selectedIndexes())
        seen.insert(idx.row());
    QList<int> rows(seen.begin(), seen.end());
    std::sort(rows.begin(), rows.end());
    return rows;
}

void ListView::setSelectedIndex(int index) {
    const QAbstractItemModel* m = model();
    if (!m || index < 0 || index >= m->rowCount()) {
        clearSelection();
        return;
    }
    setCurrentIndex(m->index(index, 0));
}

::view::scrolling::ScrollBar* ListView::verticalFluentScrollBar() const {
    return m_vScrollBar;
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void ListView::paintEvent(QPaintEvent* event) {
    const auto& c = themeColors();
    const int r = CornerRadius::Control;

    // --- 1. 绘制容器背景 ---
    if (m_backgroundVisible) {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(viewport()->rect(), c.bgLayer);
        p.end();
    }

    // --- 2. 空列表占位符 ---
    const bool isEmpty = !model() || model()->rowCount() == 0;
    if (isEmpty && !m_placeholderText.isEmpty()) {
        QPainter ph(viewport());
        ph.setRenderHint(QPainter::Antialiasing);
        ph.setPen(c.textTertiary);
        ph.setFont(themeFont(m_fontRole).toQFont());
        ph.drawText(viewport()->rect(), Qt::AlignCenter, m_placeholderText);
        ph.end();
    }

    // --- 3. 绘制列表项（QListView 默认绘制） ---
    QListView::paintEvent(event);

    // --- 4. 在 widget 层绘制容器边框（在 viewport 之上） ---
    if (m_borderVisible) {
        QPainter bp(this);
        bp.setRenderHint(QPainter::Antialiasing);

        // 边框区域：整个 widget 减去 header 区域
        QRect borderRect = rect();
        if (m_headerLabel && m_headerLabel->isVisible()) {
            borderRect.setTop(m_headerLabel->geometry().bottom());
        }

        QPainterPath borderPath;
        borderPath.addRoundedRect(QRectF(borderRect).adjusted(0.5, 0.5, -0.5, -0.5), r, r);
        bp.setPen(QPen(c.strokeDefault, 1.0));
        bp.setBrush(Qt::NoBrush);
        bp.drawPath(borderPath);
        bp.end();
    }
}

// ── Layout ────────────────────────────────────────────────────────────────────

void ListView::resizeEvent(QResizeEvent* event) {
    QListView::resizeEvent(event);
    syncFluentScrollBar();
    layoutHeader();
}

void ListView::showEvent(QShowEvent* event) {
    QListView::showEvent(event);
    syncFluentScrollBar();
    layoutHeader();
    QTimer::singleShot(0, this, &ListView::syncFluentScrollBar);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ListView::enterEvent(QEnterEvent* event) {
    setViewportHovered(true);
    QListView::enterEvent(event);
}
#else
void ListView::enterEvent(QEvent* event) {
    setViewportHovered(true);
    QListView::enterEvent(event);
}
#endif

void ListView::leaveEvent(QEvent* event) {
    setViewportHovered(false);
    QListView::leaveEvent(event);
}

// ── Overscroll bounce ─────────────────────────────────────────────────────────

void ListView::wheelEvent(QWheelEvent* event) {
    const int delta = event->angleDelta().y();
    if (delta == 0) {
        QListView::wheelEvent(event);
        return;
    }

    const qreal scrollPx = !event->pixelDelta().isNull()
                               ? static_cast<qreal>(event->pixelDelta().y())
                               : delta / 120.0 * 20.0;
    const auto phase = event->phase();

    // ── 1. Already overscrolled ──────────────────────────────────────────
    if (!qFuzzyIsNull(m_overscrollY)) {
        if (m_bounceAnim->state() == QAbstractAnimation::Running)
            m_bounceAnim->stop();
        m_bounceTimer->stop();

        // Trackpad momentum / finger-lift → bounce back immediately
        if (phase == Qt::ScrollMomentum || phase == Qt::ScrollEnd) {
            startBounceBack();
            event->accept();
            return;
        }

        // Rubber-band: quadratic resistance  damping = (1 - |y|/max)²
        constexpr qreal kMax = 100.0;
        const qreal ratio = qMin(qAbs(m_overscrollY) / kMax, 1.0);
        const qreal damping = (1.0 - ratio) * (1.0 - ratio);

        const qreal prev = m_overscrollY;
        m_overscrollY += scrollPx * qMax(damping, 0.05) * 0.5;
        m_overscrollY = qBound(-kMax, m_overscrollY, kMax);

        // Scrolled back past zero → snap and let normal scroll resume
        if ((prev > 0.0 && m_overscrollY <= 0.0) ||
            (prev < 0.0 && m_overscrollY >= 0.0)) {
            m_overscrollY = 0.0;
        }

        viewport()->update();

        // Mouse wheel (no phase) → timer triggers bounce when idle
        if (!qFuzzyIsNull(m_overscrollY) && phase == Qt::NoScrollPhase)
            m_bounceTimer->start();

        event->accept();
        return;
    }

    // ── 2. At boundary → enter overscroll ────────────────────────────────
    QScrollBar* vsb = verticalScrollBar();
    const bool atTop    = vsb->value() <= vsb->minimum();
    const bool atBottom = vsb->value() >= vsb->maximum();

    if ((atTop && scrollPx > 0) || (atBottom && scrollPx < 0)) {
        // Don't start overscroll from trackpad inertia
        if (phase == Qt::ScrollMomentum) {
            event->accept();
            return;
        }

        m_overscrollY = scrollPx * 0.5;
        viewport()->update();

        if (phase == Qt::NoScrollPhase)
            m_bounceTimer->start();

        event->accept();
        return;
    }

    // ── 3. Normal scroll ─────────────────────────────────────────────────
    QListView::wheelEvent(event);
}

int ListView::verticalOffset() const {
    return QListView::verticalOffset() - qRound(m_overscrollY);
}

void ListView::startBounceBack() {
    if (qFuzzyIsNull(m_overscrollY))
        return;
    m_bounceAnim->stop();
    m_bounceAnim->setStartValue(m_overscrollY);
    m_bounceAnim->setEndValue(0.0);
    m_bounceAnim->start();
}

void ListView::setViewportHovered(bool hovered) {
    if (m_viewportHovered == hovered)
        return;
    m_viewportHovered = hovered;
    emit viewportHoveredChanged();
}

// ── Theme ─────────────────────────────────────────────────────────────────────

void ListView::onThemeUpdated() {
    applyThemeStyle();
}

void ListView::applyThemeStyle() {
    const auto& c = themeColors();

    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Window, Qt::transparent);
    pal.setColor(QPalette::Text, c.textPrimary);
    pal.setColor(QPalette::Highlight, Qt::transparent);
    pal.setColor(QPalette::HighlightedText, c.textPrimary);
    setPalette(pal);

    setFont(themeFont(m_fontRole).toQFont());

    if (viewport()) {
        viewport()->setAutoFillBackground(false);
        QPalette vpal = viewport()->palette();
        vpal.setColor(QPalette::Base, Qt::transparent);
        vpal.setColor(QPalette::Window, Qt::transparent);
        viewport()->setPalette(vpal);
    }

    // Header label theme
    if (m_headerLabel) {
        m_headerLabel->setFont(themeFont(Typography::FontRole::Subtitle).toQFont());
        QPalette hpal = m_headerLabel->palette();
        hpal.setColor(QPalette::WindowText, c.textPrimary);
        m_headerLabel->setPalette(hpal);
    }

    update();
}

// ── Internal layout helpers ───────────────────────────────────────────────────

void ListView::layoutHeader() {
    if (!m_headerLabel) return;
    if (m_headerText.isEmpty()) {
        m_headerLabel->hide();
        return;
    }
    m_headerLabel->setText(m_headerText);
    m_headerLabel->setVisible(true);

    const int headerH = m_headerLabel->sizeHint().height() + ::Spacing::Gap::Normal;
    m_headerLabel->setGeometry(0, 0, width(), headerH);
    m_headerLabel->raise();
}

void ListView::updateViewportMargins() {
    if (m_headerLabel && !m_headerText.isEmpty()) {
        const int headerH = m_headerLabel->sizeHint().height() + ::Spacing::Gap::Normal;
        setViewportMargins(0, headerH, 0, 0);
    } else {
        setViewportMargins(0, 0, 0, 0);
    }
}

/**
 * 统一管理 Fluent 纵向滚动条：压制原生条、同步 range/pageStep、定位、显隐。
 * 平台/样式可能在 show/resize 后重新显示原生条，因此需要反复调用。
 */
void ListView::syncFluentScrollBar() {
    // 压制原生滚动条
    for (auto* sb : {verticalScrollBar(), horizontalScrollBar()}) {
        if (sb) {
            sb->setAttribute(Qt::WA_DontShowOnScreen, true);
            sb->hide();
        }
    }

    if (!m_vScrollBar) return;

    auto* native = verticalScrollBar();
    m_vScrollBar->setRange(native->minimum(), native->maximum());
    m_vScrollBar->setPageStep(native->pageStep());

    const bool needScroll = native->maximum() > native->minimum();
    m_vScrollBar->setVisible(needScroll);
    if (!needScroll) return;

    // 定位：右侧边缘，尊重 header 偏移
    const QRect r = rect();
    const int top = (m_headerLabel && m_headerLabel->isVisible())
                        ? m_headerLabel->geometry().bottom() + 2
                        : r.top() + 2;
    const int x = r.right() - m_vScrollBar->thickness() + 1;
    const int h = r.bottom() - top - 2;
    m_vScrollBar->setGeometry(x, top, m_vScrollBar->thickness(), h);
    m_vScrollBar->raise();
}

void ListView::refreshFluentScrollChrome() {
    syncFluentScrollBar();
}

} // namespace view::collections
