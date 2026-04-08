#include "FluentTreeItemDelegate.h"

#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QStyle>
#include <QTreeView>

#include "common/CornerRadius.h"
#include "common/Spacing.h"
#include "common/Typography.h"
#include "view/FluentElement.h"
#include "view/collections/TreeView.h"
#include <QVariantAnimation>

namespace treeview_test {

FluentTreeItemDelegate::FluentTreeItemDelegate(FluentElement* themeHost, int rowHeight,
                                               QAbstractItemView* view,
                                               QObject* parent)
    : QStyledItemDelegate(parent), m_themeHost(themeHost), m_rowHeight(rowHeight), m_view(view) {
    if (view && view->selectionModel()) {
        connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, [this](const QItemSelection& selected, const QItemSelection&) {
                    for (const auto& index : selected.indexes())
                        animateAccent(index);
                });
    }
}

qreal FluentTreeItemDelegate::accentProgress(const QModelIndex& index) const {
    auto it = m_accentAnims.find(QPersistentModelIndex(index));
    return it == m_accentAnims.end() ? 1.0 : it.value()->currentValue().toReal();
}

void FluentTreeItemDelegate::animateAccent(const QModelIndex& index) {
    QPersistentModelIndex pi(index);
    if (m_accentAnims.contains(pi)) return;
    auto* a = new QVariantAnimation(this);
    a->setDuration(167);
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    m_accentAnims.insert(pi, a);
    connect(a, &QVariantAnimation::valueChanged, this, [this] {
        if (m_view && m_view->viewport()) m_view->viewport()->update();
    });
    connect(a, &QVariantAnimation::finished, this, [this, pi, a] {
        m_accentAnims.remove(pi);
        a->deleteLater();
    });
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void FluentTreeItemDelegate::setThemeHost(FluentElement* host) {
    m_themeHost = host;
}

void FluentTreeItemDelegate::setRowHeight(int height) {
    m_rowHeight = height;
}

void FluentTreeItemDelegate::setCheckBoxVisible(bool visible) {
    if (m_checkBoxVisible == visible) return;
    m_checkBoxVisible = visible;
    if (m_view && m_view->viewport()) m_view->viewport()->update();
}

// ── Checkbox painting ────────────────────────────────────────────────────────

void FluentTreeItemDelegate::paintCheckBox(QPainter* painter, const QRectF& rect,
                                           Qt::CheckState state,
                                           const FluentElement::Colors& colors) const {
    painter->save();
    const qreal boxSize = 18.0;
    const qreal x = rect.center().x() - boxSize / 2.0;
    const qreal y = rect.center().y() - boxSize / 2.0;
    QRectF boxRect(x, y, boxSize, boxSize);
    const qreal r = 3.0;

    QPainterPath path;
    path.addRoundedRect(boxRect, r, r);

    if (state == Qt::Checked || state == Qt::PartiallyChecked) {
        // Filled box with accent color
        painter->setPen(Qt::NoPen);
        painter->setBrush(colors.accentDefault);
        painter->drawPath(path);

        // Glyph (checkmark or hyphen)
        QFont iconFont(Typography::FontFamily::SegoeFluentIcons, -1);
        iconFont.setPixelSize(12);
        painter->setFont(iconFont);
        painter->setPen(Qt::white);
        const QString glyph = (state == Qt::Checked)
            ? Typography::Icons::CheckMark
            : Typography::Icons::Hyphen;
        painter->drawText(boxRect, Qt::AlignCenter, glyph);
    } else {
        // Empty box with border
        painter->setPen(QPen(colors.strokeDefault, 1.5));
        painter->setBrush(Qt::transparent);
        painter->drawPath(path);
    }
    painter->restore();
}

// ── Icon glyph painting ─────────────────────────────────────────────────────

void FluentTreeItemDelegate::paintIconGlyph(QPainter* painter, const QRectF& rect,
                                            const QString& glyph,
                                            const QColor& color) const {
    QFont iconFont(Typography::FontFamily::SegoeFluentIcons, -1);
    iconFont.setPixelSize(16);
    painter->setFont(iconFont);
    painter->setPen(color);
    painter->drawText(rect, Qt::AlignCenter, glyph);
}

// ── Layout constants ─────────────────────────────────────────────────────────
namespace {
constexpr qreal kChevronAreaW = 20.0;
constexpr qreal kCheckBoxAreaW = 22.0;
constexpr qreal kIconAreaW = 20.0;
constexpr qreal kGap = 4.0;
constexpr qreal kCursorStart = 12.0;
}

// ── Hit-test helpers ─────────────────────────────────────────────────────────

QRectF FluentTreeItemDelegate::bgRectForOption(const QStyleOptionViewItem& option) const {
    const QAbstractItemView* viewWidget = qobject_cast<const QAbstractItemView*>(option.widget);
    const int vpWidth = viewWidget && viewWidget->viewport()
                            ? viewWidget->viewport()->width()
                            : option.rect.right();
    return QRectF(2, option.rect.top() + 2, vpWidth - 4, option.rect.height() - 4);
}

QRectF FluentTreeItemDelegate::checkBoxRectForOption(const QStyleOptionViewItem& option) const {
    if (!m_checkBoxVisible) return {};
    QRectF bg = bgRectForOption(option);
    qreal x = qreal(option.rect.left()) + kCursorStart;
    return QRectF(x, bg.top(), kCheckBoxAreaW, bg.height());
}

QRectF FluentTreeItemDelegate::chevronRectForOption(const QStyleOptionViewItem& option) const {
    qreal x = qreal(option.rect.left()) + kCursorStart;
    if (m_checkBoxVisible)
        x += kCheckBoxAreaW + kGap;
    QRectF bg = bgRectForOption(option);
    return QRectF(x, bg.top(), kChevronAreaW, bg.height());
}

// ── Main paint ───────────────────────────────────────────────────────────────

void FluentTreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
    if (!index.isValid())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    FluentElement::Colors colors{};
    FluentElement::Radius radius{};
    if (m_themeHost) {
        colors = m_themeHost->themeColors();
        radius = m_themeHost->themeRadius();
    }

    const int cornerR = radius.control > 0 ? radius.control : 4;
    QRectF bgRect = bgRectForOption(option);

    const bool isSelected = option.state & QStyle::State_Selected;
    const bool isHovered  = option.state & QStyle::State_MouseOver;
    const bool isPressed  = (option.state & QStyle::State_Sunken) && isHovered;
    const bool isEnabled  = option.state & QStyle::State_Enabled;

    QColor bgColor  = Qt::transparent;
    QColor textColor = colors.textPrimary;

    if (!isEnabled) {
        textColor = colors.textDisabled;
    } else if (isSelected && isPressed) {
        bgColor = colors.subtleTertiary;
    } else if (isSelected && isHovered) {
        bgColor = colors.subtleSecondary;
    } else if (isSelected) {
        bgColor = colors.subtleSecondary;
    } else if (isPressed) {
        bgColor = colors.subtleTertiary;
    } else if (isHovered) {
        bgColor = colors.subtleSecondary;
    }

    // --- Background ---
    if (bgColor.alpha() > 0) {
        QPainterPath path;
        path.addRoundedRect(bgRect, cornerR, cornerR);
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColor);
        painter->drawPath(path);
    }

    // --- Accent bar for selected items ---
    if (isSelected && isEnabled && colors.accentDefault.isValid()) {
        const qreal accentT = qBound(0.0, accentProgress(index), 1.0);

        const qreal indicatorW = 3.0;
        const qreal fullH = 16.0;
        const qreal indicatorH = fullH * (0.35 + 0.65 * accentT);
        const qreal indicatorX = bgRect.left() + 4;
        const qreal indicatorY = bgRect.center().y() - indicatorH / 2.0;
        QRectF indicatorRect(indicatorX, indicatorY, indicatorW, indicatorH);

        QPainterPath indicatorPath;
        indicatorPath.addRoundedRect(indicatorRect, indicatorW / 2.0, indicatorW / 2.0);
        QColor ac = colors.accentDefault;
        ac.setAlphaF(ac.alphaF() * accentT);
        painter->setPen(Qt::NoPen);
        painter->setBrush(ac);
        painter->drawPath(indicatorPath);
    }

    // --- Layout X cursor (从 QTreeView 的缩进位置开始) ---
    qreal cursorX = qreal(option.rect.left()) + kCursorStart;

    // --- CheckBox (WinUI: checkbox at row start, before chevron) ---
    if (m_checkBoxVisible) {
        QRectF cbRect(cursorX, bgRect.top(), kCheckBoxAreaW, bgRect.height());
        QVariant checkData = index.data(Qt::CheckStateRole);
        Qt::CheckState checkState = checkData.isValid()
            ? static_cast<Qt::CheckState>(checkData.toInt())
            : Qt::Unchecked;
        paintCheckBox(painter, cbRect, checkState, colors);
        cursorX += kCheckBoxAreaW + kGap;
    }

    // --- Expand/collapse chevron ---
    const QAbstractItemModel* m = index.model();
    const bool hasChildren = m && m->hasChildren(index);

    const qreal chevronLeft = cursorX;

    if (hasChildren) {
        auto* tv = qobject_cast<view::collections::TreeView*>(
            const_cast<QWidget*>(option.widget));
        // Get smooth rotation progress from TreeView
        const qreal rotation = tv ? tv->chevronRotation(index) : (tv && tv->isExpanded(index) ? 1.0 : 0.0);
        // Rotation: 0.0 = ChevronRight (0°), 1.0 = ChevronDown (90°)
        const qreal angle = rotation * 90.0;

        QFont iconFont(Typography::FontFamily::SegoeFluentIcons, -1);
        iconFont.setPixelSize(12);
        painter->setFont(iconFont);
        painter->setPen(textColor);

        QRectF chevronRect(chevronLeft, bgRect.top(), kChevronAreaW, bgRect.height());
        // Always draw ChevronRight and rotate it
        painter->save();
        painter->translate(chevronRect.center());
        painter->rotate(angle);
        painter->translate(-chevronRect.center());
        painter->drawText(chevronRect, Qt::AlignCenter, Typography::Icons::ChevronRightMed);
        painter->restore();
    }
    cursorX = chevronLeft + kChevronAreaW + kGap;

    // --- Icon glyph (if present) ---
    const QString iconGlyph = index.data(IconGlyphRole).toString();
    if (!iconGlyph.isEmpty()) {
        QRectF iconRect(cursorX, bgRect.top(), kIconAreaW, bgRect.height());
        paintIconGlyph(painter, iconRect, iconGlyph, textColor);
        cursorX += kIconAreaW + kGap;
    }

    // --- Text ---
    QRectF textRect(cursorX, bgRect.top(), bgRect.right() - cursorX - 8.0, bgRect.height());

    painter->setPen(textColor);
    painter->setFont(option.font);
    const QString text = index.data(Qt::DisplayRole).toString();
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                      painter->fontMetrics().elidedText(text, Qt::ElideRight, int(textRect.width())));

    painter->restore();
}

// ── editorEvent: handle clicks on checkbox and chevron ───────────────────────

bool FluentTreeItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                         const QStyleOptionViewItem& option,
                                         const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        auto* me = static_cast<QMouseEvent*>(event);
        const QPointF pos = me->position();

        // Checkbox click — cascade to children + update ancestors tri-state
        if (m_checkBoxVisible) {
            QRectF cbRect = checkBoxRectForOption(option);
            if (cbRect.contains(pos)) {
                QVariant checkData = index.data(Qt::CheckStateRole);
                Qt::CheckState curState = checkData.isValid()
                    ? static_cast<Qt::CheckState>(checkData.toInt())
                    : Qt::Unchecked;
                Qt::CheckState newState = (curState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;

                // 1) Set self
                model->setData(index, newState, Qt::CheckStateRole);

                // 2) Cascade to all descendants
                std::function<void(const QModelIndex&)> cascadeChildren =
                    [&](const QModelIndex& parent) {
                        int rows = model->rowCount(parent);
                        for (int r = 0; r < rows; ++r) {
                            QModelIndex child = model->index(r, 0, parent);
                            model->setData(child, newState, Qt::CheckStateRole);
                            cascadeChildren(child);
                        }
                    };
                cascadeChildren(index);

                // 3) Walk up and update ancestor tri-state
                std::function<void(const QModelIndex&)> updateAncestors =
                    [&](const QModelIndex& child) {
                        QModelIndex parent = child.parent();
                        if (!parent.isValid()) return;
                        int rows = model->rowCount(parent);
                        int checkedCount = 0, uncheckedCount = 0;
                        for (int r = 0; r < rows; ++r) {
                            QVariant v = model->index(r, 0, parent).data(Qt::CheckStateRole);
                            auto st = v.isValid() ? static_cast<Qt::CheckState>(v.toInt()) : Qt::Unchecked;
                            if (st == Qt::Checked) ++checkedCount;
                            else if (st == Qt::Unchecked) ++uncheckedCount;
                        }
                        Qt::CheckState parentState;
                        if (checkedCount == rows)
                            parentState = Qt::Checked;
                        else if (uncheckedCount == rows)
                            parentState = Qt::Unchecked;
                        else
                            parentState = Qt::PartiallyChecked;
                        model->setData(parent, parentState, Qt::CheckStateRole);
                        updateAncestors(parent);
                    };
                updateAncestors(index);

                return true;
            }
        }

        // Chevron click → toggle expand
        if (index.model() && index.model()->hasChildren(index)) {
            QRectF chevRect = chevronRectForOption(option);
            if (chevRect.contains(pos)) {
                auto* tv = qobject_cast<view::collections::TreeView*>(
                    const_cast<QWidget*>(option.widget));
                if (tv) tv->toggleExpanded(index);
                return true;
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize FluentTreeItemDelegate::sizeHint(const QStyleOptionViewItem& /*option*/,
                                       const QModelIndex& /*index*/) const {
    return QSize(0, m_rowHeight);
}

} // namespace treeview_test
