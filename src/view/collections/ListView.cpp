#include "ListView.h"

#include <algorithm>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QDateTime>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShowEvent>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVariantAnimation>
#include <QWheelEvent>

#include "common/Animation.h"
#include "common/CornerRadius.h"
#include "common/Spacing.h"
#include "common/Typography.h"
#include "view/scrolling/ScrollBar.h"

namespace view::collections {

namespace {
// Cross-platform wheel input thresholds (see openspec listview-cross-platform-input/design.md).
// kClusterGapMs:   gap (ms) that closes a NoPhaseDiscrete cluster — covers Mac RDP forwarding
//                  jitter (typically 60-100ms between events for a single physical gesture).
// kClusterMinPx:   min accumulated |scrollPx| within one cluster required to enter overscroll.
//                  Suppresses spurious overscroll triggered by tiny RDP/touchpad jitter.
constexpr int   kClusterGapMs = 120;
constexpr qreal kClusterMinPx = 8.0;
} // namespace

// ── Section proxy delegate ────────────────────────────────────────────────────
// Wraps the user's delegate and adds extra space + section header painting
// for the first row of each section group.

class SectionProxyDelegate : public QStyledItemDelegate {
public:
    SectionProxyDelegate(ListView* listView, QObject* parent = nullptr)
        : QStyledItemDelegate(parent), m_listView(listView) {}

    void setInnerDelegate(QAbstractItemDelegate* d) { m_inner = d; }
    QAbstractItemDelegate* innerDelegate() const { return m_inner; }

    int sectionHeaderHeight() const {
        const QFont titleFont = m_listView->themeFont(Typography::FontRole::Title).toQFont();
        const QFontMetrics fm(titleFont);
        return fm.height() + 4; // text + separator(1px) + padding(3px)
    }

    bool isSectionStart(int row) const {
        if (!m_listView->sectionEnabled() || !m_listView->m_sectionKeyFunc) return false;
        if (row == 0) return true;
        auto* model = m_listView->model();
        if (!model || row >= model->rowCount()) return false;
        return m_listView->m_sectionKeyFunc(row) != m_listView->m_sectionKeyFunc(row - 1);
    }

    QString sectionKey(int row) const {
        if (!m_listView->m_sectionKeyFunc) return {};
        return m_listView->m_sectionKeyFunc(row);
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QSize s = m_inner ? m_inner->sizeHint(option, index)
                          : QStyledItemDelegate::sizeHint(option, index);
        if (isSectionStart(index.row())) {
            s.rheight() += sectionHeaderHeight();
        }
        return s;
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        const int row = index.row();
        const bool isStart = isSectionStart(row);
        const int headerH = isStart ? sectionHeaderHeight() : 0;

        if (isStart) {
            const auto& c = m_listView->themeColors();
            const QFont titleFont = m_listView->themeFont(Typography::FontRole::Title).toQFont();
            const QFontMetrics titleFm(titleFont);
            const int hPad = ::Spacing::Padding::ListItemHorizontal;
            const int textH = titleFm.height();

            // Fill section header area with background to clear any hover artifacts
            QRect headerArea(option.rect.left(), option.rect.top(),
                             option.rect.width(), headerH);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->fillRect(headerArea, c.bgLayer);

            // Section title text
            QRect textRect(option.rect.left() + hPad, option.rect.top(),
                           option.rect.width() - 2 * hPad, textH);
            painter->setFont(titleFont);
            painter->setPen(c.textPrimary);
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, sectionKey(row));

            // Separator line below text
            const int lineY = option.rect.top() + textH + 2;
            painter->setPen(QPen(c.strokeDefault, 1.0));
            painter->drawLine(option.rect.left() + hPad, lineY,
                              option.rect.right() - hPad, lineY);

            painter->restore();
        }

        // Adjust rect for inner delegate (item content area only)
        QStyleOptionViewItem adjusted = option;
        adjusted.rect.setTop(option.rect.top() + headerH);

        // If mouse is actually in the section header zone, strip hover/press states
        // so the item below doesn't show false hover highlight
        if (isStart && (option.state & QStyle::State_MouseOver)) {
            QPoint mousePos = m_listView->viewport()->mapFromGlobal(QCursor::pos());
            if (mousePos.y() < option.rect.top() + headerH) {
                adjusted.state &= ~QStyle::State_MouseOver;
                adjusted.state &= ~QStyle::State_Sunken;
            }
        }

        if (m_inner) {
            m_inner->paint(painter, adjusted, index);
        } else {
            QStyledItemDelegate::paint(painter, adjusted, index);
        }
    }

private:
    ListView* m_listView = nullptr;
    QAbstractItemDelegate* m_inner = nullptr;
};

ListView::ListView(QWidget* parent)
    : QListView(parent) {

    m_fontRole = Typography::FontRole::Body;

    setFrameStyle(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    setSpacing(2);  // item 四周留 2px 内边距，兼顾首尾 item 与边框的间隙

    QListView::setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, &QAbstractItemView::clicked, this, [this](const QModelIndex& idx) {
        emit itemClicked(idx.row());
    });

    // Header/footer 初始为空 —— 由 setHeader() / setHeaderText() 按需创建

    // --- Fluent scroll bar (vertical) ---
    m_vScrollBar = new ::view::scrolling::ScrollBar(Qt::Vertical, this);
    m_vScrollBar->setObjectName(QStringLiteral("fluentListViewScrollBar"));
    m_vScrollBar->hide();

    auto* nativeVBar = verticalScrollBar();
    connect(nativeVBar,  &QScrollBar::valueChanged, m_vScrollBar, &QScrollBar::setValue);
    connect(m_vScrollBar, &QScrollBar::valueChanged, nativeVBar,  &QScrollBar::setValue);
    connect(nativeVBar, &QScrollBar::rangeChanged, this, &ListView::syncFluentScrollBar);

    // --- Fluent scroll bar (horizontal) ---
    m_hScrollBar = new ::view::scrolling::ScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setObjectName(QStringLiteral("fluentListViewHScrollBar"));
    m_hScrollBar->hide();

    auto* nativeHBar = horizontalScrollBar();
    connect(nativeHBar,  &QScrollBar::valueChanged, m_hScrollBar, &QScrollBar::setValue);
    connect(m_hScrollBar, &QScrollBar::valueChanged, nativeHBar,  &QScrollBar::setValue);
    connect(nativeHBar, &QScrollBar::rangeChanged, this, &ListView::syncFluentHScrollBar);

    // --- Overscroll bounce ---
    m_bounceAnim = new QVariantAnimation(this);
    m_bounceAnim->setDuration(300);
    m_bounceAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_bounceAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        if (flow() == LeftToRight)
            m_overscrollX = v.toReal();
        else
            m_overscrollY = v.toReal();
        viewport()->update();
    });

    m_bounceTimer = new QTimer(this);
    m_bounceTimer->setSingleShot(true);
    m_bounceTimer->setInterval(150);
    connect(m_bounceTimer, &QTimer::timeout, this, &ListView::startBounceBack);

    syncFluentScrollBar();
    syncFluentHScrollBar();
    onThemeUpdated();
}

ListView::~ListView() {
    // Stop bounce animation/timer before destruction so no pending tick can fire on
    // a half-destroyed object (defensive — also helps Qt5 path stability).
    if (m_bounceAnim) m_bounceAnim->stop();
    if (m_bounceTimer) m_bounceTimer->stop();
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

// ── Flow ──────────────────────────────────────────────────────────────────────

ListView::Flow ListView::flow() const {
    return QListView::flow();
}

void ListView::setFlow(Flow f) {
    if (QListView::flow() == f) return;
    QListView::setFlow(f);
    // Horizontal flow: scrollbar values must be in pixels so wheelEvent pixelDelta maps correctly.
    // Default ScrollPerItem makes scrollbar unit = item index, causing huge jumps.
    if (f == LeftToRight) {
        setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    }
    syncFluentScrollBar();
    syncFluentHScrollBar();
    emit flowChanged();
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

void ListView::setHeader(QWidget* widget) {
    if (m_header == widget) return;

    // 清理旧 header
    if (m_header) {
        m_header->hide();
        if (m_ownsHeader) {
            m_header->deleteLater();
        }
    }
    m_header = widget;
    m_ownsHeader = false;

    if (m_header) {
        m_header->setParent(this);
        m_header->show();
    }
    updateViewportMargins();
    layoutHeader();
    layoutFooter();
    syncFluentScrollBar();
    emit headerChanged();
}

void ListView::setFooter(QWidget* widget) {
    if (m_footer == widget) return;

    // 清理旧 footer
    if (m_footer) {
        m_footer->hide();
        if (m_ownsFooter) {
            m_footer->deleteLater();
        }
    }
    m_footer = widget;
    m_ownsFooter = false;

    if (m_footer) {
        m_footer->setParent(this);
        m_footer->show();
    }
    updateViewportMargins();
    layoutHeader();
    layoutFooter();
    syncFluentScrollBar();
    emit footerChanged();
}

void ListView::setHeaderText(const QString& text) {
    if (m_headerText == text) return;
    m_headerText = text;

    if (text.isEmpty()) {
        // 清空便捷文本 → 移除内部 label
        if (m_ownsHeader) {
            setHeader(nullptr);
        }
    } else {
        // 复用已有 label 或新建
        auto* lbl = m_ownsHeader ? qobject_cast<QLabel*>(m_header) : nullptr;
        if (!lbl) {
            lbl = new QLabel(this);
            lbl->setObjectName(QStringLiteral("fluentListViewHeader"));
            lbl->setIndent(::Spacing::Padding::ListItemHorizontal);
            setHeader(lbl);
            m_ownsHeader = true;
        }
        lbl->setText(text);
        applyThemeStyle();    // 确保字体已设置，sizeHint 准确
    }
    emit headerTextChanged();
}

void ListView::setFooterText(const QString& text) {
    if (m_footerText == text) return;
    m_footerText = text;

    if (text.isEmpty()) {
        if (m_ownsFooter) {
            setFooter(nullptr);
        }
    } else {
        auto* lbl = m_ownsFooter ? qobject_cast<QLabel*>(m_footer) : nullptr;
        if (!lbl) {
            lbl = new QLabel(this);
            lbl->setObjectName(QStringLiteral("fluentListViewFooter"));
            lbl->setIndent(::Spacing::Padding::ListItemHorizontal);
            setFooter(lbl);
            m_ownsFooter = true;
        }
        lbl->setText(text);
        applyThemeStyle();
    }
    emit footerTextChanged();
}

void ListView::setPlaceholderText(const QString& text) {
    if (m_placeholderText == text) return;
    m_placeholderText = text;
    if (viewport()) viewport()->update();
    emit placeholderTextChanged();
}

// ── Drag reorder ──────────────────────────────────────────────────────────────

void ListView::setCanReorderItems(bool enabled) {
    if (m_canReorderItems == enabled) return;
    m_canReorderItems = enabled;
    emit canReorderItemsChanged();
}

// ── Section ───────────────────────────────────────────────────────────────────

void ListView::setSectionEnabled(bool enabled) {
    if (m_sectionEnabled == enabled) return;
    m_sectionEnabled = enabled;
    installSectionProxy();
    if (viewport()) viewport()->update();
    emit sectionEnabledChanged();
}

void ListView::setSectionKeyFunction(SectionKeyFunc func) {
    m_sectionKeyFunc = std::move(func);
    installSectionProxy();
    if (m_sectionEnabled && viewport()) viewport()->update();
}

void ListView::installSectionProxy() {
    const bool need = m_sectionEnabled && m_sectionKeyFunc;

    if (need) {
        if (!m_sectionProxy) {
            // Capture the user's current delegate before wrapping
            m_userDelegate = itemDelegate();
            auto* proxy = new SectionProxyDelegate(this, this);
            proxy->setInnerDelegate(m_userDelegate);
            m_sectionProxy = proxy;
            QAbstractItemView::setItemDelegate(proxy);
        }
    } else if (m_sectionProxy) {
        // Restore the user's original delegate
        if (m_userDelegate) {
            QAbstractItemView::setItemDelegate(m_userDelegate);
        }
        m_sectionProxy->deleteLater();
        m_sectionProxy = nullptr;
        m_userDelegate = nullptr;
    }
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
    const QModelIndex idx = m->index(index, 0);
    if (isVisible()) {
        setCurrentIndex(idx);
    } else {
        // 未显示前不触发 scrollTo，避免在 viewport 未正确 layout 时产生错误滚动
        selectionModel()->setCurrentIndex(idx,
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
    }
}

::view::scrolling::ScrollBar* ListView::verticalFluentScrollBar() const {
    return m_vScrollBar;
}

::view::scrolling::ScrollBar* ListView::horizontalFluentScrollBar() const {
    return m_hScrollBar;
}

// ── Drag reorder events ───────────────────────────────────────────────────────

QPixmap ListView::renderItemPixmap(int row) const {
    if (!model() || row < 0 || row >= model()->rowCount())
        return {};

    QModelIndex idx = model()->index(row, 0);
    QRect rect = QListView::visualRect(idx);
    if (rect.isEmpty()) return {};

    // Use full viewport width for the snapshot so it looks like the real row.
    // If a SectionProxyDelegate is active, use the inner delegate directly
    // so the snapshot doesn't include section header area.
    QAbstractItemDelegate* del = itemDelegate();
    int h = rect.height();
    if (auto* proxy = dynamic_cast<SectionProxyDelegate*>(del)) {
        del = proxy->innerDelegate() ? proxy->innerDelegate() : del;
        // Subtract section header height for section-start rows
        if (proxy->isSectionStart(row))
            h -= proxy->sectionHeaderHeight();
    }
    const int w = viewport()->width();

    const qreal dpr = devicePixelRatioF();
    QPixmap pix(QSize(w, h) * dpr);
    pix.setDevicePixelRatio(dpr);
    pix.fill(themeColors().bgLayer);  // container background

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QStyleOptionViewItem opt;
    FLUENT_INIT_VIEW_ITEM_OPTION(&opt);
    opt.rect = QRect(0, 0, w, h);
    opt.state |= QStyle::State_Selected | QStyle::State_Enabled;
    opt.state &= ~QStyle::State_MouseOver;
    del->paint(&p, opt, idx);
    p.end();

    return pix;
}

int ListView::dropIndicatorRow(const QPoint& pos) const {
    if (!model()) return 0;
    const int count = model()->rowCount();
    if (count == 0) return 0;

    // During drag, use displaced visual positions for hit testing
    // so the indicator follows the actual visual layout.
    if (m_isDragging) {
        for (int i = 0; i < count; ++i) {
            if (i == m_dragSourceRow) continue;
            QRect rect = QListView::visualRect(model()->index(i, 0));
            rect.translate(0, qRound(m_dragOffsets.value(i, 0.0)));
            if (pos.y() < rect.center().y())
                return i;
        }
        return count;
    }

    // Non-drag: standard hit test
    QModelIndex idx = indexAt(pos);
    if (!idx.isValid()) return count;
    QRect rect = visualRect(idx);
    if (pos.y() > rect.center().y()) return idx.row() + 1;
    return idx.row();
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
    // 拖拽时应用位移偏移：通过 visualRect override 临时返回偏移后的矩形
    m_paintingWithOffsets = !m_dragOffsets.isEmpty();
    QListView::paintEvent(event);
    m_paintingWithOffsets = false;

    // --- 3.5 Section headers are now handled by SectionProxyDelegate ---

    // --- 3.6 绘制拖拽指示线 ---
    if (m_isDragging && m_dropTargetRow >= 0 && model()) {
        m_paintingWithOffsets = !m_dragOffsets.isEmpty();
        QPainter dp(viewport());
        dp.setRenderHint(QPainter::Antialiasing);

        int y;
        if (m_dropTargetRow < model()->rowCount()) {
            y = visualRect(model()->index(m_dropTargetRow, 0)).top();
        } else {
            QRect lastRect = visualRect(model()->index(model()->rowCount() - 1, 0));
            y = lastRect.bottom() + 1;
        }

        const auto& clr = themeColors();
        dp.setPen(QPen(clr.accentDefault, 2.0));
        dp.drawLine(::Spacing::Padding::ListItemHorizontal, y,
                    viewport()->width() - ::Spacing::Padding::ListItemHorizontal, y);
        const int circleR = 3;
        dp.setBrush(clr.accentDefault);
        dp.setPen(Qt::NoPen);
        dp.drawEllipse(QPoint(::Spacing::Padding::ListItemHorizontal, y), circleR, circleR);
        dp.drawEllipse(QPoint(viewport()->width() - ::Spacing::Padding::ListItemHorizontal, y), circleR, circleR);
        m_paintingWithOffsets = false;
        dp.end();
    }

    // --- 3.7 拖拽浮动图层 ---
    if (m_isDragging && !m_dragPixmap.isNull()) {
        QPainter fp(viewport());
        fp.setRenderHint(QPainter::Antialiasing);
        fp.setOpacity(0.85);
        const int pixH = qRound(m_dragPixmap.height() / m_dragPixmap.devicePixelRatio());
        QPoint pixPos(0, m_dragCurrentPos.y() - pixH / 2);
        fp.drawPixmap(pixPos, m_dragPixmap);
        fp.end();
    }

    // --- 4. 圆角遮罩：用父背景色覆盖四角区域（抗锯齿，替代 setMask） ---
    if (m_borderVisible || m_backgroundVisible) {
        QPainter cp(viewport());
        cp.setRenderHint(QPainter::Antialiasing);

        QPainterPath fullRect;
        fullRect.addRect(QRectF(viewport()->rect()));
        QPainterPath roundedArea;
        roundedArea.addRoundedRect(QRectF(viewport()->rect()), r, r);
        QPainterPath corners = fullRect - roundedArea;

        // 从父控件取背景色；回退到 bgCanvas
        QColor parentBg = c.bgCanvas;
        if (parentWidget()) {
            const QPalette& pp = parentWidget()->palette();
            if (pp.color(QPalette::Window).alpha() > 0)
                parentBg = pp.color(QPalette::Window);
        }
        cp.fillPath(corners, parentBg);
        cp.end();
    }

    // --- 5. 容器边框 ---
    if (m_borderVisible) {
        QPainter bp(viewport());
        bp.setRenderHint(QPainter::Antialiasing);

        QPainterPath borderPath;
        borderPath.addRoundedRect(QRectF(viewport()->rect()).adjusted(0.5, 0.5, -0.5, -0.5), r, r);
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
    syncFluentHScrollBar();
    layoutHeader();
    layoutFooter();
}

void ListView::showEvent(QShowEvent* event) {
    QListView::showEvent(event);
    updateViewportMargins();
    syncFluentScrollBar();
    syncFluentHScrollBar();
    layoutHeader();
    layoutFooter();
    // widget 初次显示时用正确的 viewport 尺寸重新定位到当前选中项
    if (currentIndex().isValid()) {
        scrollTo(currentIndex(), QAbstractItemView::EnsureVisible);
    }
    QTimer::singleShot(0, this, [this]() {
        syncFluentScrollBar();
        syncFluentHScrollBar();
    });
}

bool ListView::isPointInSectionHeader(const QPoint& viewportPos) const {
    if (!m_sectionEnabled || !m_sectionProxy) return false;

    auto* proxy = static_cast<SectionProxyDelegate*>(m_sectionProxy);
    if (!proxy) return false;

    QModelIndex idx = indexAt(viewportPos);
    if (!idx.isValid()) return false;

    if (!proxy->isSectionStart(idx.row())) return false;

    // The section header occupies the top portion of the visual rect
    QRect vr = visualRect(idx);
    int headerH = proxy->sectionHeaderHeight();
    return viewportPos.y() < vr.top() + headerH;
}

void ListView::mousePressEvent(QMouseEvent* event) {
    if (isPointInSectionHeader(event->pos())) {
        event->accept();
        return;  // Swallow click on section header area
    }
    if (m_canReorderItems && event->button() == Qt::LeftButton) {
        QModelIndex idx = indexAt(event->pos());
        if (idx.isValid()) {
            m_dragStartPos = event->pos();
            m_dragSourceRow = idx.row();
        }
    }
    QListView::mousePressEvent(event);
}

void ListView::mouseMoveEvent(QMouseEvent* event) {
    if (m_canReorderItems && m_dragSourceRow >= 0 && (event->buttons() & Qt::LeftButton)) {
        if (!m_isDragging) {
            if ((event->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance()) {
                // Grab snapshot BEFORE setting m_isDragging to avoid capturing ghost overlay
                m_dragPixmap = renderItemPixmap(m_dragSourceRow);
                m_isDragging = true;
            }
        }

        if (m_isDragging) {
            m_dragCurrentPos = event->pos();
            int target = dropIndicatorRow(event->pos());
            if (target != m_dropTargetRow) {
                m_dropTargetRow = target;
                updateDragDisplacement();
            }
            viewport()->update();
            event->accept();
            return;
        }
    }
    QListView::mouseMoveEvent(event);
}

void ListView::mouseReleaseEvent(QMouseEvent* event) {
    if (m_isDragging && event->button() == Qt::LeftButton) {
        int src = m_dragSourceRow;
        int dst = m_dropTargetRow;

        if (dst >= 0 && src >= 0 && model()) {
            if (src < dst) dst--;
            if (src != dst && src < model()->rowCount()) {
                int destRow = (src < dst) ? dst + 1 : dst;
                bool moved = model()->moveRow(QModelIndex(), src, QModelIndex(), destRow);
                if (!moved) {
                    // QStandardItemModel doesn't implement moveRow — use takeRow/insertRow
                    if (auto* sim = qobject_cast<QStandardItemModel*>(model())) {
                        auto row = sim->takeRow(src);
                        sim->insertRow(dst, row);
                        moved = true;
                    }
                }
                if (moved) {
                    setCurrentIndex(model()->index(dst, 0));
                    emit itemReordered(src, dst);
                }
            }
        }

        m_isDragging = false;
        m_dragSourceRow = -1;
        m_dropTargetRow = -1;
        m_dragPixmap = QPixmap();
        clearDragAnimations();
        viewport()->update();
        event->accept();
        return;
    }

    if (m_canReorderItems) {
        m_dragSourceRow = -1;
    }
    QListView::mouseReleaseEvent(event);
}

void ListView::enterEvent(FluentEnterEvent* event) {
    setViewportHovered(true);
    QListView::enterEvent(event);
}

void ListView::leaveEvent(QEvent* event) {
    setViewportHovered(false);
    QListView::leaveEvent(event);
}

// ── Overscroll bounce ─────────────────────────────────────────────────────────
//
// Cross-platform wheel input handling — see openspec listview-cross-platform-input/design.md.
// Events are classified into three paths:
//
//   PhaseBased       phase != NoScrollPhase                   macOS native trackpad
//   NoPhasePixel     phase == NoScrollPhase, pixelDelta != 0  some Win precision touchpad drivers
//   NoPhaseDiscrete  phase == NoScrollPhase, pixelDelta == 0  mouse wheel / Mac RDP→Windows / Qt5
//
// PhaseBased / NoPhasePixel keep the original behavior (smooth pixel scrolling + boundary
// overscroll). NoPhaseDiscrete is throttled by a cluster window (kClusterGapMs / kClusterMinPx)
// to prevent RDP-forwarded high-frequency events from triggering reflexive bounce flapping.

void ListView::wheelEvent(QWheelEvent* event) {
    enum class WheelKind { PhaseBased, NoPhasePixel, NoPhaseDiscrete };
    const auto phase = event->phase();
    const bool hasPixelDelta = !event->pixelDelta().isNull();
    const WheelKind kind = (phase != Qt::NoScrollPhase) ? WheelKind::PhaseBased
                         : (hasPixelDelta             ? WheelKind::NoPhasePixel
                                                      : WheelKind::NoPhaseDiscrete);

    const bool horizontal = (flow() == LeftToRight);

    // For horizontal flow, pick the dominant axis (not sum) to avoid double-counting on diagonal swipes.
    // Trackpad users naturally swipe vertically, so Y is often the dominant axis even for horizontal lists.
    const int delta = horizontal
        ? (qAbs(event->angleDelta().y()) >= qAbs(event->angleDelta().x())
               ? event->angleDelta().y() : event->angleDelta().x())
        : event->angleDelta().y();

    qreal& overscroll = horizontal ? m_overscrollX : m_overscrollY;

    // Zero-delta event (e.g. ScrollEnd on Windows touchpad with no residual)
    if (delta == 0 && !hasPixelDelta) {
        if (!qFuzzyIsNull(overscroll) &&
            (phase == Qt::ScrollEnd || phase == Qt::ScrollMomentum)) {
            startBounceBack();
            event->accept();
            return;
        }
        QListView::wheelEvent(event);
        return;
    }

    // pixelDelta: same dominant-axis logic; apply directly like Qt does for vertical scroll — no damping
    const qreal scrollPx = hasPixelDelta
        ? static_cast<qreal>(horizontal
              ? (qAbs(event->pixelDelta().y()) >= qAbs(event->pixelDelta().x())
                     ? event->pixelDelta().y() : event->pixelDelta().x())
              : event->pixelDelta().y())
        : delta / 120.0 * 20.0;

    // ── NoPhaseDiscrete cluster accumulation (mouse wheel / Mac RDP / Qt5) ──
    // Tracked regardless of bounce state so that successive ticks within the same physical
    // gesture continue to coalesce.
    qreal clusterAccum = 0.0;
    if (kind == WheelKind::NoPhaseDiscrete) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - m_lastNoPhaseTs > kClusterGapMs) {
            m_clusterAccum = 0.0;
        }
        m_lastNoPhaseTs = now;
        m_clusterAccum += scrollPx;
        clusterAccum = m_clusterAccum;
    }

    // ── 1. Already overscrolled ──────────────────────────────────────────
    if (!qFuzzyIsNull(overscroll)) {
        if (m_bounceAnim && m_bounceAnim->state() == QAbstractAnimation::Running) {
            // Bounce in progress: NoPhasePixel and NoPhaseDiscrete events are stale residuals
            // (mouse wheel / RDP / Win touchpad fallback) — consume them so the bounce
            // animation completes smoothly. Only PhaseBased (native macOS trackpad) is allowed
            // to interrupt, matching the user's explicit finger-press intent.
            if (kind != WheelKind::PhaseBased) {
                event->accept();
                return;
            }
            m_bounceAnim->stop();
        }
        if (m_bounceTimer) m_bounceTimer->stop();

        // Trackpad momentum / finger-lift → bounce back immediately
        if (phase == Qt::ScrollMomentum || phase == Qt::ScrollEnd) {
            startBounceBack();
            event->accept();
            return;
        }

        // Rubber-band: quadratic resistance  damping = (1 - |v|/max)²
        constexpr qreal kMax = 100.0;
        const qreal ratio = qMin(qAbs(overscroll) / kMax, 1.0);
        const qreal damping = (1.0 - ratio) * (1.0 - ratio);

        const qreal prev = overscroll;
        overscroll += scrollPx * qMax(damping, 0.05) * 0.5;
        overscroll = qBound(-kMax, overscroll, kMax);

        // Scrolled back past zero → snap and let normal scroll resume
        if ((prev > 0.0 && overscroll <= 0.0) ||
            (prev < 0.0 && overscroll >= 0.0)) {
            overscroll = 0.0;
        }

        viewport()->update();

        // NoPhaseDiscrete (mouse wheel / RDP) → timer bounce
        if (!qFuzzyIsNull(overscroll) && kind == WheelKind::NoPhaseDiscrete && m_bounceTimer)
            m_bounceTimer->start();

        event->accept();
        return;
    }

    // ── 2. At boundary → enter overscroll ────────────────────────────────
    QScrollBar* sb = horizontal ? horizontalScrollBar() : verticalScrollBar();
    const bool atStart = sb->value() <= sb->minimum();
    const bool atEnd   = sb->value() >= sb->maximum();

    // Enter-overscroll trigger: PhaseBased / NoPhasePixel use single-event scrollPx; NoPhaseDiscrete
    // uses cluster-accumulated value gated by kClusterMinPx to suppress RDP/jitter false positives.
    const qreal triggerVal = (kind == WheelKind::NoPhaseDiscrete) ? clusterAccum : scrollPx;
    const qreal triggerThresh = (kind == WheelKind::NoPhaseDiscrete) ? kClusterMinPx : 0.0;

    const bool wantsEnter =
        (atStart && triggerVal >  triggerThresh) ||
        (atEnd   && triggerVal < -triggerThresh);

    if (wantsEnter) {
        // Don't enter overscroll from inertia or finger-lift
        if (phase == Qt::ScrollMomentum || phase == Qt::ScrollEnd) {
            event->accept();
            return;
        }

        overscroll = triggerVal * 0.5;
        viewport()->update();

        if (kind == WheelKind::NoPhaseDiscrete && m_bounceTimer)
            m_bounceTimer->start();

        event->accept();
        return;
    }

    // At-boundary NoPhaseDiscrete events that fail the cluster threshold: consume them
    // to avoid Qt forwarding to parent / triggering native scrollbar artifacts.
    if (kind == WheelKind::NoPhaseDiscrete && (atStart || atEnd) &&
        ((atStart && scrollPx > 0) || (atEnd && scrollPx < 0))) {
        event->accept();
        return;
    }

    // ── 3. Normal scroll ─────────────────────────────────────────────────
    if (horizontal) {
        QScrollBar* hsb = horizontalScrollBar();
        hsb->setValue(hsb->value() - qRound(scrollPx));
        event->accept();
    } else {
        QListView::wheelEvent(event);
    }
}

int ListView::verticalOffset() const {
    return QListView::verticalOffset() - qRound(m_overscrollY);
}

int ListView::horizontalOffset() const {
    return QListView::horizontalOffset() - qRound(m_overscrollX);
}

QRect ListView::visualRect(const QModelIndex& index) const {
    QRect r = QListView::visualRect(index);
    if (m_paintingWithOffsets && index.isValid()) {
        if (m_isDragging && index.row() == m_dragSourceRow) {
            // Hide source row: move off-screen so QListView won't paint it
            r.moveTop(-r.height() * 2);
            return r;
        }
        r.translate(0, qRound(m_dragOffsets.value(index.row(), 0.0)));
    }
    return r;
}

void ListView::startBounceBack() {
    if (!m_bounceAnim) return;
    const qreal val = (flow() == LeftToRight) ? m_overscrollX : m_overscrollY;
    if (qFuzzyIsNull(val))
        return;
    m_bounceAnim->stop();
    m_bounceAnim->setStartValue(val);
    m_bounceAnim->setEndValue(0.0);
    m_bounceAnim->start();
}

// paintSectionHeaders() removed — section headers are now painted by SectionProxyDelegate

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

    // Header label theme (only for internally-created labels)
    if (m_ownsHeader) {
        if (auto* lbl = qobject_cast<QLabel*>(m_header)) {
            lbl->setFont(themeFont(Typography::FontRole::Subtitle).toQFont());
            QPalette hpal = lbl->palette();
            hpal.setColor(QPalette::WindowText, c.textPrimary);
            lbl->setPalette(hpal);
        }
    }

    // Footer label theme (only for internally-created labels)
    if (m_ownsFooter) {
        if (auto* lbl = qobject_cast<QLabel*>(m_footer)) {
            lbl->setFont(themeFont(Typography::FontRole::Caption).toQFont());
            QPalette fpal = lbl->palette();
            fpal.setColor(QPalette::WindowText, c.textSecondary);
            lbl->setPalette(fpal);
        }
    }

    updateViewportMargins();
    layoutHeader();
    layoutFooter();
    update();
}

// ── Internal layout helpers ───────────────────────────────────────────────────

void ListView::layoutHeader() {
    if (!m_header || !m_header->isVisible()) return;

    const int headerH = m_header->sizeHint().height() + ::Spacing::Gap::Normal;
    m_header->setGeometry(0, 0, width(), headerH);
    m_header->raise();
}

void ListView::layoutFooter() {
    if (!m_footer || !m_footer->isVisible()) return;

    const int footerH = m_footer->sizeHint().height() + ::Spacing::Gap::Normal;
    m_footer->setGeometry(0, height() - footerH, width(), footerH);
    m_footer->raise();
}

void ListView::updateViewportMargins() {
    int top = 0, bottom = 0;
    if (m_header && m_header->isVisible()) {
        top = m_header->sizeHint().height() + ::Spacing::Gap::Normal;
    }
    if (m_footer && m_footer->isVisible()) {
        bottom = m_footer->sizeHint().height() + ::Spacing::Gap::Normal;
    }
    setViewportMargins(0, top, 0, bottom);
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
    const int top = (m_header && m_header->isVisible())
                        ? m_header->geometry().bottom() + 2
                        : r.top() + 2;
    const int x = r.right() - m_vScrollBar->thickness() + 1;
    const int h = r.bottom() - top - 2;
    m_vScrollBar->setGeometry(x, top, m_vScrollBar->thickness(), h);
    m_vScrollBar->raise();
}

void ListView::syncFluentHScrollBar() {
    // 压制原生滚动条
    for (auto* sb : {verticalScrollBar(), horizontalScrollBar()}) {
        if (sb) {
            sb->setAttribute(Qt::WA_DontShowOnScreen, true);
            sb->hide();
        }
    }

    if (!m_hScrollBar) return;

    auto* native = horizontalScrollBar();
    m_hScrollBar->setRange(native->minimum(), native->maximum());
    m_hScrollBar->setPageStep(native->pageStep());

    const bool needScroll = native->maximum() > native->minimum();
    m_hScrollBar->setVisible(needScroll);
    if (!needScroll) return;

    // 定位：底部边缘
    const QRect r = rect();
    const int left = r.left() + 2;
    const int w = r.width() - 4;
    const int y = r.bottom() - m_hScrollBar->thickness() + 1;
    m_hScrollBar->setGeometry(left, y, w, m_hScrollBar->thickness());
    m_hScrollBar->raise();
}

void ListView::refreshFluentScrollChrome() {
    syncFluentScrollBar();
    syncFluentHScrollBar();
}

// ── Drag displacement animation ───────────────────────────────────────────────

void ListView::updateDragDisplacement() {
    if (m_dragSourceRow < 0 || m_dropTargetRow < 0 || !model()) {
        clearDragAnimations();
        return;
    }

    const int itemCount = model()->rowCount();
    const int src = m_dragSourceRow;
    const int dst = m_dropTargetRow;

    // Use the actual source row height for displacement amount
    const int srcH = QListView::visualRect(model()->index(src, 0)).height();
    if (srcH <= 0) return;

    for (int i = 0; i < itemCount; ++i) {
        qreal target = 0.0;
        if (i == src) {
            // Source item: no displacement (follows cursor via drag pixmap)
            target = 0.0;
        } else if (src < dst && i > src && i < dst) {
            // Source moves down: items between (src, dst) shift up
            target = -srcH;
        } else if (src > dst && i >= dst && i < src) {
            // Source moves up: items between [dst, src) shift down
            target = srcH;
        }

        const qreal current = m_dragOffsets.value(i, 0.0);
        if (qFuzzyCompare(current, target) && !m_dragAnims.contains(i)) {
            continue;
        }

        if (auto* oldAnim = m_dragAnims.value(i)) {
            oldAnim->stop();
            oldAnim->deleteLater();
            m_dragAnims.remove(i);
        }

        if (qFuzzyCompare(current, target)) {
            continue;
        }

        auto* anim = new QVariantAnimation(this);
        anim->setStartValue(current);
        anim->setEndValue(target);
        anim->setDuration(::Animation::Duration::Fast);
        anim->setEasingCurve(::Animation::getEasing(::Animation::EasingType::Decelerate));
        connect(anim, &QVariantAnimation::valueChanged, this, [this, i](const QVariant& v) {
            m_dragOffsets[i] = v.toReal();
            viewport()->update();
        });
        connect(anim, &QVariantAnimation::finished, this, [this, i, target]() {
            m_dragOffsets[i] = target;
            if (auto* a = m_dragAnims.value(i)) {
                a->deleteLater();
                m_dragAnims.remove(i);
            }
        });
        m_dragAnims[i] = anim;
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void ListView::clearDragAnimations() {
    for (auto it = m_dragAnims.begin(); it != m_dragAnims.end(); ++it) {
        if (it.value()) {
            it.value()->stop();
            it.value()->deleteLater();
        }
    }
    m_dragAnims.clear();
    m_dragOffsets.clear();
    viewport()->update();
}

} // namespace view::collections
