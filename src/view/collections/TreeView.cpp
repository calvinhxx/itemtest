#include "TreeView.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
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

TreeView::TreeView(QWidget* parent)
    : QTreeView(parent) {

    m_fontRole = Typography::FontRole::Body;

    setFrameStyle(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    setIndentation(16);                 // WinUI 3: 16px per level
    setItemsExpandable(true);
    setRootIsDecorated(false);          // 由 delegate 绘制 chevron，禁用原生展开箭头
    setExpandsOnDoubleClick(false);     // 单击展开，禁用双击
    setHeaderHidden(true);              // 隐藏列标题

    QTreeView::setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // WinUI: itemClicked signal (展开/收缩由 delegate editorEvent 处理)
    connect(this, &QAbstractItemView::clicked, this, [this](const QModelIndex& idx) {
        emit itemClicked(idx);
    });

    // Header label
    m_headerLabel = new QLabel(this);
    m_headerLabel->setObjectName(QStringLiteral("fluentTreeViewHeader"));
    m_headerLabel->setIndent(::Spacing::Padding::ListItemHorizontal);
    m_headerLabel->hide();

    // --- Fluent scroll bar (vertical) ---
    m_vScrollBar = new ::view::scrolling::ScrollBar(Qt::Vertical, this);
    m_vScrollBar->setObjectName(QStringLiteral("fluentTreeViewVScrollBar"));
    m_vScrollBar->hide();

    auto* nativeVBar = verticalScrollBar();
    connect(nativeVBar,  &QScrollBar::valueChanged, m_vScrollBar, &QScrollBar::setValue);
    connect(m_vScrollBar, &QScrollBar::valueChanged, nativeVBar,  &QScrollBar::setValue);
    connect(nativeVBar, &QScrollBar::rangeChanged, this, &TreeView::syncFluentScrollBar);

    // --- Fluent scroll bar (horizontal) ---
    m_hScrollBar = new ::view::scrolling::ScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setObjectName(QStringLiteral("fluentTreeViewHScrollBar"));
    m_hScrollBar->hide();

    auto* nativeHBar = horizontalScrollBar();
    connect(nativeHBar,  &QScrollBar::valueChanged, m_hScrollBar, &QScrollBar::setValue);
    connect(m_hScrollBar, &QScrollBar::valueChanged, nativeHBar,  &QScrollBar::setValue);
    connect(nativeHBar, &QScrollBar::rangeChanged, this, &TreeView::syncFluentHScrollBar);

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
    connect(m_bounceTimer, &QTimer::timeout, this, &TreeView::startBounceBack);

    // --- Expand reveal animation ---
    m_expandRevealAnim = new QVariantAnimation(this);
    m_expandRevealAnim->setDuration(::Animation::Duration::Normal);
    m_expandRevealAnim->setEasingCurve(::Animation::getEasing(::Animation::EasingType::Decelerate));
    connect(m_expandRevealAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant&) {
        viewport()->update();
    });
    connect(m_expandRevealAnim, &QVariantAnimation::finished, this, [this]() {
        m_animParent = QPersistentModelIndex();
        viewport()->update();
    });
    connect(this, &QTreeView::expanded, this, [this](const QModelIndex& idx) {
        // Chevron rotation: 0 → 1
        {
            QPersistentModelIndex pi(idx);
            if (m_chevronAnims.contains(pi)) {
                m_chevronAnims[pi]->stop();
                m_chevronAnims[pi]->deleteLater();
                m_chevronAnims.remove(pi);
            }
            auto* ca = new QVariantAnimation(this);
            ca->setDuration(::Animation::Duration::Normal);
            ca->setEasingCurve(::Animation::getEasing(::Animation::EasingType::Decelerate));
            ca->setStartValue(0.0);
            ca->setEndValue(1.0);
            m_chevronAnims.insert(pi, ca);
            connect(ca, &QVariantAnimation::valueChanged, this, [this](const QVariant&) {
                viewport()->update();
            });
            connect(ca, &QVariantAnimation::finished, this, [this, pi, ca]() {
                m_chevronAnims.remove(pi);
                ca->deleteLater();
            });
            ca->start(QAbstractAnimation::DeleteWhenStopped);
        }
        // Child reveal animation
        if (!m_animEnabled) return;
        m_animParent = QPersistentModelIndex(idx);
        m_animExpanding = true;
        m_expandRevealAnim->stop();
        m_expandRevealAnim->setStartValue(0.0);
        m_expandRevealAnim->setEndValue(1.0);
        m_expandRevealAnim->start();
    });
    connect(this, &QTreeView::collapsed, this, [this](const QModelIndex& idx) {
        // Chevron rotation: 1 → 0
        {
            QPersistentModelIndex pi(idx);
            if (m_chevronAnims.contains(pi)) {
                m_chevronAnims[pi]->stop();
                m_chevronAnims[pi]->deleteLater();
                m_chevronAnims.remove(pi);
            }
            auto* ca = new QVariantAnimation(this);
            ca->setDuration(::Animation::Duration::Normal);
            ca->setEasingCurve(::Animation::getEasing(::Animation::EasingType::Decelerate));
            ca->setStartValue(1.0);
            ca->setEndValue(0.0);
            m_chevronAnims.insert(pi, ca);
            connect(ca, &QVariantAnimation::valueChanged, this, [this](const QVariant&) {
                viewport()->update();
            });
            connect(ca, &QVariantAnimation::finished, this, [this, pi, ca]() {
                m_chevronAnims.remove(pi);
                ca->deleteLater();
            });
            ca->start(QAbstractAnimation::DeleteWhenStopped);
        }
        // Stop expand animation if active
        if (m_animParent.isValid() && m_animParent == idx) {
            m_expandRevealAnim->stop();
            m_animParent = QPersistentModelIndex();
        }
    });

    syncFluentScrollBar();
    syncFluentHScrollBar();
    onThemeUpdated();
}

// ── Selection mode ────────────────────────────────────────────────────────────

void TreeView::setSelectionMode(TreeSelectionMode mode) {
    if (m_selectionMode == mode) return;
    m_selectionMode = mode;

    switch (mode) {
    case TreeSelectionMode::None:
        QTreeView::setSelectionMode(QAbstractItemView::NoSelection);
        break;
    case TreeSelectionMode::Single:
        QTreeView::setSelectionMode(QAbstractItemView::SingleSelection);
        break;
    case TreeSelectionMode::Multiple:
        QTreeView::setSelectionMode(QAbstractItemView::MultiSelection);
        break;
    case TreeSelectionMode::Extended:
        QTreeView::setSelectionMode(QAbstractItemView::ExtendedSelection);
        break;
    }
    emit selectionModeChanged();
}

// ── Drag reorder ──────────────────────────────────────────────────────────────

void TreeView::setCanReorderItems(bool enabled) {
    if (m_canReorderItems == enabled) return;
    m_canReorderItems = enabled;
    emit canReorderItemsChanged();
}

// ── Appearance properties ────────────────────────────────────────────────────

void TreeView::setFontRole(const QString& role) {
    if (m_fontRole == role) return;
    m_fontRole = role;
    applyThemeStyle();
    emit fontRoleChanged();
}

void TreeView::setBorderVisible(bool visible) {
    if (m_borderVisible == visible) return;
    m_borderVisible = visible;
    update();
    emit borderVisibleChanged();
}

void TreeView::setBackgroundVisible(bool visible) {
    if (m_backgroundVisible == visible) return;
    m_backgroundVisible = visible;
    if (viewport()) viewport()->update();
    emit backgroundVisibleChanged();
}

void TreeView::setHeaderText(const QString& text) {
    if (m_headerText == text) return;
    m_headerText = text;

    m_headerLabel->setText(text);
    m_headerLabel->setVisible(!text.isEmpty());
    applyThemeStyle();
    emit headerTextChanged();
}

void TreeView::setPlaceholderText(const QString& text) {
    if (m_placeholderText == text) return;
    m_placeholderText = text;
    if (viewport()) viewport()->update();
    emit placeholderTextChanged();
}

// ── Tree API ──────────────────────────────────────────────────────────────────

void TreeView::expandAll() {
    m_animEnabled = false;
    // Stop all chevron animations and clear
    for (auto* a : m_chevronAnims)
        a->stop();
    qDeleteAll(m_chevronAnims);
    m_chevronAnims.clear();
    QTreeView::expandAll();
    m_animEnabled = true;
}

void TreeView::collapseAll() {
    m_animEnabled = false;
    for (auto* a : m_chevronAnims)
        a->stop();
    qDeleteAll(m_chevronAnims);
    m_chevronAnims.clear();
    QTreeView::collapseAll();
    m_animEnabled = true;
}

void TreeView::toggleExpanded(const QModelIndex& index) {
    if (!index.isValid()) return;
    setExpanded(index, !isExpanded(index));
}

// ── Selection API ─────────────────────────────────────────────────────────────

QModelIndex TreeView::selectedItem() const {
    auto idxList = selectionModel() ? selectionModel()->selectedIndexes() : QModelIndexList();
    return idxList.isEmpty() ? QModelIndex() : idxList.first();
}

QModelIndexList TreeView::selectedItems() const {
    return selectionModel() ? selectionModel()->selectedIndexes() : QModelIndexList();
}

void TreeView::setSelectedItem(const QModelIndex& index) {
    if (!index.isValid()) {
        clearSelection();
        return;
    }
    if (isVisible()) {
        setCurrentIndex(index);
    } else {
        selectionModel()->setCurrentIndex(index,
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
    }
}

::view::scrolling::ScrollBar* TreeView::verticalFluentScrollBar() const {
    return m_vScrollBar;
}

::view::scrolling::ScrollBar* TreeView::horizontalFluentScrollBar() const {
    return m_hScrollBar;
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void TreeView::paintEvent(QPaintEvent* event) {
    const auto& c = themeColors();
    const int r = CornerRadius::Control;

    // --- 1. Container background ---
    if (m_backgroundVisible) {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(viewport()->rect(), c.bgLayer);
        p.end();
    }

    // --- 2. Empty placeholder ---
    const bool isEmpty = !model() || model()->rowCount() == 0;
    if (isEmpty && !m_placeholderText.isEmpty()) {
        QPainter ph(viewport());
        ph.setRenderHint(QPainter::Antialiasing);
        ph.setPen(c.textTertiary);
        ph.setFont(themeFont(m_fontRole).toQFont());
        ph.drawText(viewport()->rect(), Qt::AlignCenter, m_placeholderText);
        ph.end();
    }

    // --- 3. Tree items (QTreeView default) ---
    QTreeView::paintEvent(event);

    // --- 3.5 Drop indicator: OnItem highlight ---
    if (m_isDragging && m_dropMode == DropMode::OnItem && m_dropOnIndex.isValid()) {
        QRect targetRect = visualRect(m_dropOnIndex);
        if (!targetRect.isEmpty()) {
            QPainter hp(viewport());
            hp.setRenderHint(QPainter::Antialiasing);
            QColor fill = c.accentDefault;
            fill.setAlphaF(0.12f);
            const int rr = CornerRadius::Control;
            hp.setPen(QPen(c.accentDefault, 2.0));
            hp.setBrush(fill);
            hp.drawRoundedRect(QRectF(targetRect).adjusted(1, 0, -1, 0), rr, rr);
            hp.end();
        }
    }

    // --- 3.6 Drop indicator: Between line ---
    if (m_isDragging && m_dropMode == DropMode::Between && m_dropTargetRow >= 0 && model()) {
        QPainter dp(viewport());
        dp.setRenderHint(QPainter::Antialiasing);

        int siblingCount = model()->rowCount(m_dropTargetParent);
        int y;
        if (m_dropTargetRow < siblingCount) {
            QModelIndex targetIdx = model()->index(m_dropTargetRow, 0, m_dropTargetParent);
            QRect targetRect = visualRect(targetIdx);
            y = targetRect.top();
        } else {
            // After last sibling
            QModelIndex lastIdx = model()->index(siblingCount - 1, 0, m_dropTargetParent);
            QRect lastRect = visualRect(lastIdx);
            y = lastRect.bottom() + 1;
        }

        dp.setPen(QPen(c.accentDefault, 2.0));
        dp.drawLine(::Spacing::Padding::ListItemHorizontal, y,
                     viewport()->width() - ::Spacing::Padding::ListItemHorizontal, y);
        const int circleR = 3;
        dp.setBrush(c.accentDefault);
        dp.setPen(Qt::NoPen);
        dp.drawEllipse(QPoint(::Spacing::Padding::ListItemHorizontal, y), circleR, circleR);
        dp.drawEllipse(QPoint(viewport()->width() - ::Spacing::Padding::ListItemHorizontal, y), circleR, circleR);
        dp.end();
    }

    // --- 3.7 Drag floating pixmap (offset from cursor, like file manager) ---
    if (m_isDragging && !m_dragPixmap.isNull()) {
        QPainter fp(viewport());
        fp.setRenderHint(QPainter::Antialiasing);
        fp.setOpacity(0.85);
        const int pixH = qRound(m_dragPixmap.height() / m_dragPixmap.devicePixelRatio());
        // Place pixmap slightly right and below the cursor
        constexpr int kOffsetX = 8;
        constexpr int kOffsetY = 4;
        QPoint pixPos(m_dragCurrentPos.x() + kOffsetX,
                      m_dragCurrentPos.y() - pixH / 2 + kOffsetY);
        fp.drawPixmap(pixPos, m_dragPixmap);
        fp.end();
    }

    // --- 4. Corner masking: fill corners with parent bg (anti-alias) ---
    if (m_borderVisible || m_backgroundVisible) {
        QPainter cp(viewport());
        cp.setRenderHint(QPainter::Antialiasing);

        QPainterPath fullRect;
        fullRect.addRect(QRectF(viewport()->rect()));
        QPainterPath roundedArea;
        roundedArea.addRoundedRect(QRectF(viewport()->rect()), r, r);
        QPainterPath corners = fullRect - roundedArea;

        QColor parentBg = c.bgCanvas;
        if (parentWidget()) {
            const QPalette& pp = parentWidget()->palette();
            if (pp.color(QPalette::Window).alpha() > 0)
                parentBg = pp.color(QPalette::Window);
        }
        cp.fillPath(corners, parentBg);
        cp.end();
    }

    // --- 5. Container border ---
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

void TreeView::resizeEvent(QResizeEvent* event) {
    QTreeView::resizeEvent(event);
    updateViewportMargins();
    syncFluentScrollBar();
    syncFluentHScrollBar();
    layoutHeader();
}

void TreeView::showEvent(QShowEvent* event) {
    QTreeView::showEvent(event);
    updateViewportMargins();
    layoutHeader();
    // 延迟滚动和同步，避免首帧闪缩
    QTimer::singleShot(0, this, [this]() {
        if (currentIndex().isValid()) {
            scrollTo(currentIndex(), QAbstractItemView::EnsureVisible);
        }
        syncFluentScrollBar();
        syncFluentHScrollBar();
    });
}

void TreeView::enterEvent(FluentEnterEvent* event) {
    setViewportHovered(true);
    QTreeView::enterEvent(event);
}

void TreeView::leaveEvent(QEvent* event) {
    setViewportHovered(false);
    QTreeView::leaveEvent(event);
}

// ── Overscroll bounce ─────────────────────────────────────────────────────────

void TreeView::wheelEvent(QWheelEvent* event) {
    const int delta = event->angleDelta().y();
    const auto phase = event->phase();

    // Zero-delta event (e.g. ScrollEnd on Windows touchpad with no residual)
    if (delta == 0 && event->pixelDelta().isNull()) {
        // If overscrolled and gesture ended → bounce back
        if (!qFuzzyIsNull(m_overscrollY) &&
            (phase == Qt::ScrollEnd || phase == Qt::ScrollMomentum)) {
            startBounceBack();
            event->accept();
            return;
        }
        QTreeView::wheelEvent(event);
        return;
    }

    const qreal scrollPx = !event->pixelDelta().isNull()
                               ? static_cast<qreal>(event->pixelDelta().y())
                               : delta / 120.0 * 20.0;

    // ── 1. Already overscrolled ──────────────────────────────────────────
    if (!qFuzzyIsNull(m_overscrollY)) {
        if (m_bounceAnim->state() == QAbstractAnimation::Running) {
            // Bounce in progress: consume stale NoScrollPhase events (RDP / mouse wheel)
            if (phase == Qt::NoScrollPhase) {
                event->accept();
                return;
            }
            m_bounceAnim->stop();
        }
        m_bounceTimer->stop();

        if (phase == Qt::ScrollMomentum || phase == Qt::ScrollEnd) {
            startBounceBack();
            event->accept();
            return;
        }

        constexpr qreal kMax = 100.0;
        const qreal ratio = qMin(qAbs(m_overscrollY) / kMax, 1.0);
        const qreal damping = (1.0 - ratio) * (1.0 - ratio);

        const qreal prev = m_overscrollY;
        m_overscrollY += scrollPx * qMax(damping, 0.05) * 0.5;
        m_overscrollY = qBound(-kMax, m_overscrollY, kMax);

        if ((prev > 0.0 && m_overscrollY <= 0.0) ||
            (prev < 0.0 && m_overscrollY >= 0.0)) {
            m_overscrollY = 0.0;
        }

        viewport()->update();

        // NoScrollPhase (mouse wheel / Windows touchpad fallback) → timer bounce
        if (!qFuzzyIsNull(m_overscrollY) && phase == Qt::NoScrollPhase)
            m_bounceTimer->start();

        event->accept();
        return;
    }

    // ── 2. At boundary → enter overscroll ────────────────────────────────
    QScrollBar* sb = verticalScrollBar();
    const bool atStart = sb->value() <= sb->minimum();
    const bool atEnd   = sb->value() >= sb->maximum();

    if ((atStart && scrollPx > 0) || (atEnd && scrollPx < 0)) {
        // Don't enter overscroll from inertia or finger-lift
        if (phase == Qt::ScrollMomentum || phase == Qt::ScrollEnd) {
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
    QTreeView::wheelEvent(event);
}

int TreeView::verticalOffset() const {
    return QTreeView::verticalOffset() - qRound(m_overscrollY);
}

void TreeView::drawBranches(QPainter* /*painter*/, const QRect& /*rect*/,
                            const QModelIndex& /*index*/) const {
    // 不绘制原生 branch 指示器 — delegate 自行绘制 Fluent 风格 chevron
}

void TreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& options,
                       const QModelIndex& index) const {
    if (m_expandRevealAnim->state() == QAbstractAnimation::Running
        && m_animParent.isValid() && m_animExpanding) {
        // Check if this row is a descendant of the animating parent
        bool isDescendant = false;
        int childRow = -1;
        QModelIndex p = index.parent();
        if (p.isValid() && p == m_animParent) {
            isDescendant = true;
            childRow = index.row();
        } else {
            // deeper descendants
            QModelIndex walk = p;
            while (walk.isValid()) {
                if (walk == m_animParent) { isDescendant = true; break; }
                walk = walk.parent();
            }
        }
        if (isDescendant) {
            const qreal rawProgress = m_expandRevealAnim->currentValue().toReal();
            // Stagger: each direct child row starts slightly later
            constexpr qreal kStaggerDelay = 0.08;
            const qreal rowDelay = (childRow >= 0) ? childRow * kStaggerDelay : 0.0;
            const qreal progress = qBound(0.0, (rawProgress - rowDelay) / qMax(1.0 - rowDelay, 0.01), 1.0);

            painter->save();
            painter->setOpacity(progress);
            painter->translate(0, -(1.0 - progress) * 8.0);
            QTreeView::drawRow(painter, options, index);
            painter->restore();
            return;
        }
    }
    // --- Drag: dim source row ---
    if (m_isDragging && index == m_dragSourceIndex) {
        painter->save();
        painter->setOpacity(0.3);
        QTreeView::drawRow(painter, options, index);
        painter->restore();
        return;
    }
    QTreeView::drawRow(painter, options, index);
}

qreal TreeView::chevronRotation(const QModelIndex& index) const {
    QPersistentModelIndex pi(index);
    auto it = m_chevronAnims.find(pi);
    if (it != m_chevronAnims.end())
        return it.value()->currentValue().toReal();
    // No animation running — return steady state
    return isExpanded(index) ? 1.0 : 0.0;
}

// ── Drag reorder: mouse events ───────────────────────────────────────────────

void TreeView::mousePressEvent(QMouseEvent* event) {
    if (m_canReorderItems && event->button() == Qt::LeftButton) {
        QModelIndex idx = indexAt(event->pos());
        if (idx.isValid()) {
            m_dragStartPos = event->pos();
            m_dragSourceIndex = QPersistentModelIndex(idx);
        }
    }
    QTreeView::mousePressEvent(event);
}

void TreeView::mouseMoveEvent(QMouseEvent* event) {
    if (m_canReorderItems && m_dragSourceIndex.isValid()
        && (event->buttons() & Qt::LeftButton)) {
        if (!m_isDragging) {
            if ((event->pos() - m_dragStartPos).manhattanLength()
                >= QApplication::startDragDistance()) {
                m_dragPixmap = renderRowPixmap(m_dragSourceIndex);
                m_dragCurrentPos = event->pos();
                m_isDragging = true;
            }
        }

        if (m_isDragging) {
            m_dragCurrentPos = event->pos();
            computeDropTarget(event->pos());
            viewport()->update();
            event->accept();
            return;
        }
    }
    QTreeView::mouseMoveEvent(event);
}

void TreeView::mouseReleaseEvent(QMouseEvent* event) {
    if (m_isDragging && event->button() == Qt::LeftButton) {
        if (m_dropMode != DropMode::None && model()) {
            auto* sim = qobject_cast<QStandardItemModel*>(model());
            if (sim && m_dragSourceIndex.isValid()) {
                QModelIndex srcParentIdx = m_dragSourceIndex.parent();
                int srcRow = m_dragSourceIndex.row();

                QStandardItem* srcParentItem = srcParentIdx.isValid()
                    ? sim->itemFromIndex(srcParentIdx) : sim->invisibleRootItem();

                if (m_dropMode == DropMode::OnItem && m_dropOnIndex.isValid()) {
                    QStandardItem* targetItem = sim->itemFromIndex(QModelIndex(m_dropOnIndex));
                    if (srcParentItem && targetItem) {
                        auto row = srcParentItem->takeRow(srcRow);
                        targetItem->appendRow(row);
                        QModelIndex dropParent(m_dropOnIndex);
                        expand(dropParent);
                        QModelIndex newIdx = sim->indexFromItem(row.first());
                        setCurrentIndex(newIdx);
                        emit itemReordered(srcParentIdx, srcRow,
                                           dropParent, targetItem->rowCount() - 1);
                    }
                } else if (m_dropMode == DropMode::Between) {
                    QStandardItem* dstParentItem = m_dropTargetParent.isValid()
                        ? sim->itemFromIndex(QModelIndex(m_dropTargetParent))
                        : sim->invisibleRootItem();
                    if (srcParentItem && dstParentItem) {
                        bool sameParent = (srcParentItem == dstParentItem);
                        int dstRow = m_dropTargetRow;

                        auto row = srcParentItem->takeRow(srcRow);
                        if (sameParent && srcRow < dstRow)
                            dstRow--;
                        dstParentItem->insertRow(dstRow, row);
                        QModelIndex newIdx = sim->indexFromItem(row.first());
                        setCurrentIndex(newIdx);
                        emit itemReordered(srcParentIdx, srcRow,
                                           QModelIndex(m_dropTargetParent), dstRow);
                    }
                }
            }
        }

        m_isDragging = false;
        m_dragSourceIndex = QPersistentModelIndex();
        m_dropMode = DropMode::None;
        m_dropTargetParent = QPersistentModelIndex();
        m_dropTargetRow = -1;
        m_dropOnIndex = QPersistentModelIndex();
        m_dragPixmap = QPixmap();
        viewport()->update();
        event->accept();
        return;
    }

    if (m_canReorderItems) {
        m_dragSourceIndex = QPersistentModelIndex();
    }
    QTreeView::mouseReleaseEvent(event);
}

// ── Drag reorder: helpers ────────────────────────────────────────────────────

QPixmap TreeView::renderRowPixmap(const QModelIndex& index) const {
    if (!index.isValid() || !model()) return {};

    QRect rect = QTreeView::visualRect(index);
    if (rect.isEmpty()) return {};

    // Render the row content area (respecting indentation)
    const int contentX = rect.x();              // indentation offset
    const int contentW = rect.width();           // width after indent
    const int h = rect.height();
    const qreal dpr = devicePixelRatioF();

    QPixmap pix(QSize(contentW, h) * dpr);
    pix.setDevicePixelRatio(dpr);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QStyleOptionViewItem opt;
    FLUENT_INIT_VIEW_ITEM_OPTION(&opt);
    opt.rect = QRect(0, 0, contentW, h);
    opt.state |= QStyle::State_Selected | QStyle::State_Enabled;
    opt.state &= ~QStyle::State_MouseOver;
    if (itemDelegate())
        itemDelegate()->paint(&p, opt, index);
    p.end();

    // Scale to 75% for a compact floating thumbnail
    constexpr qreal kScale = 0.75;
    const int sw = qRound(contentW * kScale);
    const int sh = qRound(h * kScale);
    QPixmap scaled = pix.scaled(QSize(sw, sh) * dpr,
                                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(dpr);
    return scaled;
}

void TreeView::computeDropTarget(const QPoint& pos) {
    m_dropMode = DropMode::None;
    m_dropTargetParent = QPersistentModelIndex();
    m_dropTargetRow = -1;
    m_dropOnIndex = QPersistentModelIndex();

    if (!model()) return;

    QModelIndex idx = indexAt(pos);

    if (!idx.isValid()) {
        // Below all items → insert at end of root
        m_dropMode = DropMode::Between;
        m_dropTargetRow = model()->rowCount();
        return;
    }

    // Can't drop on self or on a descendant of the source (cycle)
    if (idx == m_dragSourceIndex) return;
    if (isDescendantOf(idx, m_dragSourceIndex)) return;

    QRect rect = visualRect(idx);
    int rowH = rect.height();
    if (rowH <= 0) return;
    int relY = pos.y() - rect.top();

    bool hasChildren = model()->rowCount(idx) > 0;

    if (hasChildren) {
        // Three zones: top 25% / middle 50% / bottom 25%
        if (relY < rowH / 4) {
            m_dropMode = DropMode::Between;
            m_dropTargetParent = QPersistentModelIndex(idx.parent());
            m_dropTargetRow = idx.row();
        } else if (relY > rowH * 3 / 4) {
            m_dropMode = DropMode::Between;
            m_dropTargetParent = QPersistentModelIndex(idx.parent());
            m_dropTargetRow = idx.row() + 1;
        } else {
            m_dropMode = DropMode::OnItem;
            m_dropOnIndex = QPersistentModelIndex(idx);
        }
    } else {
        // Three zones for leaf items too: top 25% / middle 50% / bottom 25%
        // Middle zone = drop onto item (create new nesting)
        if (relY < rowH / 4) {
            m_dropMode = DropMode::Between;
            m_dropTargetParent = QPersistentModelIndex(idx.parent());
            m_dropTargetRow = idx.row();
        } else if (relY > rowH * 3 / 4) {
            m_dropMode = DropMode::Between;
            m_dropTargetParent = QPersistentModelIndex(idx.parent());
            m_dropTargetRow = idx.row() + 1;
        } else {
            m_dropMode = DropMode::OnItem;
            m_dropOnIndex = QPersistentModelIndex(idx);
        }
    }
}

bool TreeView::isDescendantOf(const QModelIndex& candidate, const QModelIndex& ancestor) const {
    QModelIndex walk = candidate.parent();
    while (walk.isValid()) {
        if (walk == ancestor) return true;
        walk = walk.parent();
    }
    return false;
}

void TreeView::startBounceBack() {
    if (qFuzzyIsNull(m_overscrollY))
        return;
    m_bounceAnim->stop();
    m_bounceAnim->setStartValue(m_overscrollY);
    m_bounceAnim->setEndValue(0.0);
    m_bounceAnim->start();
}

void TreeView::setViewportHovered(bool hovered) {
    if (m_viewportHovered == hovered)
        return;
    m_viewportHovered = hovered;
    emit viewportHoveredChanged();
}

// ── Theme ─────────────────────────────────────────────────────────────────────

void TreeView::onThemeUpdated() {
    applyThemeStyle();
}

void TreeView::applyThemeStyle() {
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

    updateViewportMargins();
    layoutHeader();
    update();
}

// ── Internal layout helpers ───────────────────────────────────────────────────

void TreeView::layoutHeader() {
    if (!m_headerLabel || !m_headerLabel->isVisible()) return;

    const int headerH = m_headerLabel->sizeHint().height() + ::Spacing::Gap::Normal;
    m_headerLabel->setGeometry(0, 0, width(), headerH);
    m_headerLabel->raise();
}

void TreeView::updateViewportMargins() {
    int top = 2;  // 首行边距，配合 delegate bgRect inset 实现 ListView 同等间距
    if (m_headerLabel && m_headerLabel->isVisible()) {
        top = m_headerLabel->sizeHint().height() + ::Spacing::Gap::Normal;
    }
    setViewportMargins(0, top, 0, 0);
}

void TreeView::syncFluentScrollBar() {
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

    const QRect r = rect();
    const int top = (m_headerLabel && m_headerLabel->isVisible())
                        ? m_headerLabel->geometry().bottom() + 2
                        : r.top() + 2;
    const int x = r.right() - m_vScrollBar->thickness() + 1;
    const int h = r.bottom() - top - 2;
    m_vScrollBar->setGeometry(x, top, m_vScrollBar->thickness(), h);
    m_vScrollBar->raise();
}

void TreeView::syncFluentHScrollBar() {
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

    const QRect r = rect();
    const int left = r.left() + 2;
    const int w = r.width() - 4;
    const int y = r.bottom() - m_hScrollBar->thickness() + 1;
    m_hScrollBar->setGeometry(left, y, w, m_hScrollBar->thickness());
    m_hScrollBar->raise();
}

void TreeView::refreshFluentScrollChrome() {
    syncFluentScrollBar();
    syncFluentHScrollBar();
}

} // namespace view::collections
