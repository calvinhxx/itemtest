#include "AnchorLayout.h"
#include <QWidgetItem>

AnchorLayout::AnchorLayout(QWidget* parent) : QLayout(parent) {
    setContentsMargins(0, 0, 0, 0);
}

AnchorLayout::~AnchorLayout() {
    while (!m_items.isEmpty()) {
        delete takeAt(0);
    }
}

void AnchorLayout::addAnchoredWidget(QWidget* w, const Anchors& anchors) {
    if (!w) return;
    if (parentWidget() && w->parent() != parentWidget()) {
        w->setParent(parentWidget());
    }
    for (Item& it : m_items) {
        if (it.item->widget() == w) {
            it.anchors = anchors;
            invalidate();
            return;
        }
    }
    addItem(new QWidgetItem(w));
    m_items.last().anchors = anchors;
    invalidate();
}

void AnchorLayout::addItem(QLayoutItem* item) {
    Item it;
    it.item = item;
    m_items.append(it);
    invalidate();
}

int AnchorLayout::count() const { return m_items.size(); }

QLayoutItem* AnchorLayout::itemAt(int index) const {
    if (index < 0 || index >= m_items.size()) return nullptr;
    return m_items.at(index).item;
}

QLayoutItem* AnchorLayout::takeAt(int index) {
    if (index < 0 || index >= m_items.size()) return nullptr;
    Item it = m_items.takeAt(index);
    return it.item;
}

QSize AnchorLayout::sizeHint() const { return QSize(400, 300); }
QSize AnchorLayout::minimumSize() const { return QSize(0, 0); }

int AnchorLayout::getWidgetIndex(QWidget* w) const {
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].item->widget() == w) return i;
    }
    return -1;
}

int AnchorLayout::getEdgeValue(QWidget* target, Edge edge, const QRect& parentRect) const {
    if (!target || target == parentWidget()) {
        switch (edge) {
            case Edge::Left:    return parentRect.left();
            case Edge::Right:   return parentRect.right();
            case Edge::Top:     return parentRect.top();
            case Edge::Bottom:  return parentRect.bottom();
            case Edge::HCenter: return parentRect.center().x();
            case Edge::VCenter: return parentRect.center().y();
            default: return 0;
        }
    }
    int idx = getWidgetIndex(target);
    if (idx == -1) return 0;
    const QRect& r = m_items[idx].geometry;
    switch (edge) {
        case Edge::Left:    return r.left();
        case Edge::Right:   return r.right();
        case Edge::Top:     return r.top();
        case Edge::Bottom:  return r.bottom();
        case Edge::HCenter: return r.center().x();
        case Edge::VCenter: return r.center().y();
        default: return 0;
    }
}

void AnchorLayout::setGeometry(const QRect& rect) {
    QLayout::setGeometry(rect);
    if (m_items.isEmpty()) return;

    QRect parentRect = contentsRect();
    for (Item& it : m_items) {
        it.geometry = QRect(parentRect.topLeft(), it.item->sizeHint());
    }

    for (int pass = 0; pass < 5; ++pass) {
        bool changed = false;
        for (Item& it : m_items) {
            QRect old = it.geometry;
            QSize s = it.item->sizeHint();

            if (it.anchors.fill) {
                it.geometry = parentRect.marginsRemoved(it.anchors.fillMargins);
            } else {
                // 水平
                if (it.anchors.horizontalCenter.edge != Edge::None) {
                    int targetVal = getEdgeValue(it.anchors.horizontalCenter.target, it.anchors.horizontalCenter.edge, parentRect);
                    it.geometry.moveCenter(QPoint(targetVal + it.anchors.horizontalCenter.offset, it.geometry.center().y()));
                } else {
                    bool leftFixed = it.anchors.left.edge != Edge::None;
                    bool rightFixed = it.anchors.right.edge != Edge::None;
                    int leftVal = getEdgeValue(it.anchors.left.target, it.anchors.left.edge, parentRect) + it.anchors.left.offset;
                    int rightVal = getEdgeValue(it.anchors.right.target, it.anchors.right.edge, parentRect) + it.anchors.right.offset;

                    if (leftFixed && rightFixed) {
                        it.geometry.setLeft(leftVal);
                        it.geometry.setRight(rightVal);
                    } else if (leftFixed) {
                        it.geometry.moveLeft(leftVal);
                        it.geometry.setWidth(s.width());
                    } else if (rightFixed) {
                        it.geometry.moveRight(rightVal);
                        it.geometry.setWidth(s.width());
                    }
                }

                // 垂直
                if (it.anchors.verticalCenter.edge != Edge::None) {
                    int targetVal = getEdgeValue(it.anchors.verticalCenter.target, it.anchors.verticalCenter.edge, parentRect);
                    it.geometry.moveCenter(QPoint(it.geometry.center().x(), targetVal + it.anchors.verticalCenter.offset));
                } else {
                    bool topFixed = it.anchors.top.edge != Edge::None;
                    bool bottomFixed = it.anchors.bottom.edge != Edge::None;
                    int topVal = getEdgeValue(it.anchors.top.target, it.anchors.top.edge, parentRect) + it.anchors.top.offset;
                    int bottomVal = getEdgeValue(it.anchors.bottom.target, it.anchors.bottom.edge, parentRect) + it.anchors.bottom.offset;

                    if (topFixed && bottomFixed) {
                        it.geometry.setTop(topVal);
                        it.geometry.setBottom(bottomVal);
                    } else if (topFixed) {
                        it.geometry.moveTop(topVal);
                        it.geometry.setHeight(s.height());
                    } else if (bottomFixed) {
                        it.geometry.moveBottom(bottomVal);
                        it.geometry.setHeight(s.height());
                    }
                }
            }
            if (it.geometry != old) changed = true;
        }
        if (!changed) break;
    }

    for (const Item& it : m_items) {
        it.item->widget()->setGeometry(it.geometry);
    }
}
