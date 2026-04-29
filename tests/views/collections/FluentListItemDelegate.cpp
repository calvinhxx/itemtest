#include "FluentListItemDelegate.h"

#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QStyle>

#include "design/Spacing.h"
#include "view/FluentElement.h"
#include <QVariantAnimation>

namespace listview_test {

FluentListItemDelegate::FluentListItemDelegate(FluentElement* themeHost, int rowHeight,
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

qreal FluentListItemDelegate::accentProgress(const QModelIndex& index) const {
    auto it = m_accentAnims.find(QPersistentModelIndex(index));
    return it == m_accentAnims.end() ? 1.0 : it.value()->currentValue().toReal();
}

void FluentListItemDelegate::animateAccent(const QModelIndex& index) {
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

void FluentListItemDelegate::setThemeHost(FluentElement* host) {
    m_themeHost = host;
}

void FluentListItemDelegate::setRowHeight(int height) {
    m_rowHeight = height;
}

void FluentListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
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

    const int hPad = Spacing::Padding::ListItemHorizontal;
    const int cornerR = radius.control > 0 ? radius.control : 4;

    QRectF bgRect = QRectF(option.rect).adjusted(2, 1, -2, -1);

    const bool isSelected = option.state & QStyle::State_Selected;
    const bool isHovered = option.state & QStyle::State_MouseOver;
    const bool isPressed = (option.state & QStyle::State_Sunken) && isHovered;
    const bool isEnabled = option.state & QStyle::State_Enabled;

    QColor bgColor = Qt::transparent;
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

    if (bgColor.alpha() > 0) {
        QPainterPath path;
        path.addRoundedRect(bgRect, cornerR, cornerR);
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColor);
        painter->drawPath(path);
    }

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

    QRectF textRect = bgRect.adjusted(hPad + 8, 0, -hPad, 0);
    painter->setPen(textColor);
    painter->setFont(option.font);
    const QString text = index.data(Qt::DisplayRole).toString();
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                      painter->fontMetrics().elidedText(text, Qt::ElideRight, int(textRect.width())));

    painter->restore();
}

QSize FluentListItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
    const int h = m_rowHeight > 0 ? m_rowHeight
                                  : (Spacing::ControlHeight::Standard + Spacing::Gap::Tight);
    const int hPad = Spacing::Padding::ListItemHorizontal;
    const int hGap  = Spacing::Gap::Tight;  // 水平 item 之间的间距
    const QString text = index.data(Qt::DisplayRole).toString();
    const QFontMetrics fm(option.font);
    const int w = fm.horizontalAdvance(text) + hPad * 2 + 12 + hGap;
    return QSize(w, h);
}

} // namespace listview_test
