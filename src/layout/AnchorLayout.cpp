#include "AnchorLayout.h"
#include <QWidget>
#include <QStyle>
#include <QWidgetItem>
#include <algorithm>

AnchorLayout::AnchorLayout(QWidget *parent)
    : QLayout(parent)
{
}

AnchorLayout::~AnchorLayout()
{
    while (!m_items.isEmpty()) {
        QLayoutItem *it = takeAt(0);
        delete it;
    }
}

void AnchorLayout::addAnchoredWidget(QWidget *w, const AnchorSpec &spec)
{
    if (!w) return;
    QWidgetItem *wi = new QWidgetItem(w);
    Item it;
    it.layoutItem = wi;
    it.widget = w;
    it.spec = spec;
    m_items.append(it);
}

void AnchorLayout::addItem(QLayoutItem *item)
{
    Item it;
    it.layoutItem = item;
    it.widget = item->widget();
    m_items.append(it);
}

int AnchorLayout::count() const { return m_items.size(); }

QLayoutItem *AnchorLayout::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size()) return nullptr;
    return m_items.at(index).layoutItem;
}

QLayoutItem *AnchorLayout::takeAt(int index)
{
    if (index < 0 || index >= m_items.size()) return nullptr;
    Item it = m_items.takeAt(index);
    QLayoutItem *li = it.layoutItem;
    return li;
}

QSize AnchorLayout::sizeHint() const
{
    QSize s;
    for (const Item &it : m_items) {
        s = s.expandedTo(itemSizeHint(it));
    }
    return s;
}

QSize AnchorLayout::minimumSize() const
{
    QSize s;
    for (const Item &it : m_items) {
        if (it.layoutItem) s = s.expandedTo(it.layoutItem->minimumSize());
    }
    return s;
}

QRect AnchorLayout::parentContentsRect() const
{
    QWidget *p = parentWidget();
    if (!p) return QRect();
    return p->contentsRect();
}

int AnchorLayout::widgetEdgePos(QWidget *w, AnchorSpec::Edge edge) const
{
    if (!w) return 0;
    const QRect g = w->geometry();
    switch (edge) {
    case AnchorSpec::LeftEdge: return g.left();
    case AnchorSpec::RightEdge: return g.right();
    case AnchorSpec::TopEdge: return g.top();
    case AnchorSpec::BottomEdge: return g.bottom();
    }
    return g.left();
}

QSize AnchorLayout::itemSizeHint(const Item &it) const
{
    if (!it.spec.preferredSize.isEmpty()) return it.spec.preferredSize;
    if (it.layoutItem) return it.layoutItem->sizeHint();
    return QSize();
}

void AnchorLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    QRect parentRect = parentContentsRect();

    struct Calc { Item it; QSize size; QRect geom; };
    QVector<Calc> calcs;
    calcs.reserve(m_items.size());
    for (const Item &it : m_items) {
        Calc c; c.it = it; c.size = itemSizeHint(it); c.geom = QRect(0,0,c.size.width(), c.size.height());
        calcs.append(c);
    }

    for (int i = 0; i < calcs.size(); ++i) {
        AnchorSpec &s = calcs[i].it.spec;
        QRect g = calcs[i].geom;

        if (s.fillParent) {
            QRect fr = parentRect.marginsRemoved(s.fillMargins);
            calcs[i].geom = fr;
            if (calcs[i].it.widget) calcs[i].it.widget->setGeometry(calcs[i].geom);
            continue;
        }

        // Horizontal
        bool hasLeft = false, hasRight = false;
        int left = 0, right = 0;
        if (s.leftToParent) {
            hasLeft = true;
            left = (s.leftToParentEdge == AnchorSpec::LeftEdge) ? parentRect.left() + s.leftOffset
                                                                 : parentRect.right() + s.leftOffset;
        } else if (!s.leftToWidget.isNull()) {
            hasLeft = true;
            left = widgetEdgePos(s.leftToWidget.data(), s.leftToWidgetEdge) + s.leftOffset;
        }

        if (s.rightToParent) {
            hasRight = true;
            right = (s.rightToParentEdge == AnchorSpec::RightEdge) ? parentRect.right() - s.rightOffset
                                                                   : parentRect.left() - s.rightOffset;
        } else if (!s.rightToWidget.isNull()) {
            hasRight = true;
            right = widgetEdgePos(s.rightToWidget.data(), s.rightToWidgetEdge) - s.rightOffset;
        }

        int w = g.width();
        int x = parentRect.left();
        if (hasLeft && hasRight) {
            int newW = right - left + 1;
            if (newW < 0) newW = w;
            x = left;
            w = newW;
        } else if (hasLeft) {
            x = left;
        } else if (hasRight) {
            x = right - w + 1;
        } else if (s.horizontalCenter) {
            int cx;
            if (!s.hCenterToWidget.isNull()) {
                QRect tg = s.hCenterToWidget->geometry();
                cx = tg.left() + tg.width()/2 + s.hCenterOffset;
            } else {
                cx = parentRect.left() + parentRect.width()/2 + s.hCenterOffset;
            }
            x = cx - w/2;
        } else {
            x = parentRect.left();
        }

        // Vertical
        bool hasTop = false, hasBottom = false;
        int top = 0, bottom = 0;
        if (s.topToParent) {
            hasTop = true;
            top = (s.topToParentEdge == AnchorSpec::TopEdge) ? parentRect.top() + s.topOffset
                                                             : parentRect.bottom() + s.topOffset;
        } else if (!s.topToWidget.isNull()) {
            hasTop = true;
            top = widgetEdgePos(s.topToWidget.data(), s.topToWidgetEdge) + s.topOffset;
        }

        if (s.bottomToParent) {
            hasBottom = true;
            bottom = (s.bottomToParentEdge == AnchorSpec::BottomEdge) ? parentRect.bottom() - s.bottomOffset
                                                                       : parentRect.top() - s.bottomOffset;
        } else if (!s.bottomToWidget.isNull()) {
            hasBottom = true;
            bottom = widgetEdgePos(s.bottomToWidget.data(), s.bottomToWidgetEdge) - s.bottomOffset;
        }

        int h = g.height();
        int y = parentRect.top();
        if (hasTop && hasBottom) {
            int newH = bottom - top + 1;
            if (newH < 0) newH = h;
            y = top;
            h = newH;
        } else if (hasTop) {
            y = top;
        } else if (hasBottom) {
            y = bottom - h + 1;
        } else if (s.verticalCenter) {
            int cy;
            if (!s.vCenterToWidget.isNull()) {
                QRect tg = s.vCenterToWidget->geometry();
                cy = tg.top() + tg.height()/2 + s.vCenterOffset;
            } else {
                cy = parentRect.top() + parentRect.height()/2 + s.vCenterOffset;
            }
            y = cy - h/2;
        } else {
            y = parentRect.top();
        }

        calcs[i].geom = QRect(x, y, w, h);
        if (calcs[i].it.widget) calcs[i].it.widget->setGeometry(calcs[i].geom);
    }
}
