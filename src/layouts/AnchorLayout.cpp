#include "AnchorLayout.h"
#include <QWidgetItem>
#include <QWidget>

AnchorLayout::AnchorLayout(QWidget* parent) : QLayout(parent) {
    // 设置默认边距为 0，因为 AnchorLayout 通常靠 offset 控制
    setContentsMargins(0, 0, 0, 0);
}

AnchorLayout::~AnchorLayout() {
    while (!m_items.isEmpty()) {
        delete takeAt(0);
    }
}

void AnchorLayout::addAnchoredWidget(QWidget* w, const Anchors& anchors) {
    if (!w) return;
    // 先检查是否已经存在，如果存在则更新 anchors
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

int AnchorLayout::count() const {
    return m_items.size();
}

QLayoutItem* AnchorLayout::itemAt(int index) const {
    if (index < 0 || index >= m_items.size()) return nullptr;
    return m_items.at(index).item;
}

QLayoutItem* AnchorLayout::takeAt(int index) {
    if (index < 0 || index >= m_items.size()) return nullptr;
    Item it = m_items.takeAt(index);
    return it.item;
}

QSize AnchorLayout::sizeHint() const {
    if (m_items.isEmpty()) return QSize(400, 300);
    return QSize(600, 600);
}

QSize AnchorLayout::minimumSize() const {
    return QSize(0, 0);
}

int AnchorLayout::getWidgetIndex(QWidget* w) const {
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].item->widget() == w) return i;
    }
    return -1;
}

void AnchorLayout::setGeometry(const QRect& rect) {
    QLayout::setGeometry(rect);
    if (m_items.isEmpty()) return;

    QRect parentRect = contentsRect(); // 获取布局可用区域

    // 1. 初始化几何状态：优先取真实大小，再取建议大小
    for (Item& it : m_items) {
        QWidget* w = it.item->widget();
        QSize s = (w && w->size().isValid()) ? w->size() : it.item->sizeHint();
        it.geometry = QRect(QPoint(parentRect.left(), parentRect.top()), s);
    }

    // 2. 多轮迭代解析锚点
    const int maxPasses = 10;
    for (int pass = 0; pass < maxPasses; ++pass) {
        bool changed = false;

        for (int i = 0; i < m_items.size(); ++i) {
            Item& it = m_items[i];
            QRect oldGeom = it.geometry;
            
            // 获取当前项最可靠的尺寸
            QWidget* currentW = it.item->widget();
            QSize s = (currentW && currentW->size().isValid()) ? currentW->size() : it.item->sizeHint();

            if (it.anchors.fill) {
                it.geometry = parentRect.marginsRemoved(it.anchors.fillMargins);
            } else {
                int left = it.geometry.left();
                int right = it.geometry.right();
                int top = it.geometry.top();
                int bottom = it.geometry.bottom();
                bool leftFixed = false, rightFixed = false, topFixed = false, bottomFixed = false;

                // 水平解析
                if (it.anchors.horizontalCenter) {
                    left = parentRect.left() + (parentRect.width() - s.width()) / 2 + it.anchors.horizontalCenterOffset;
                    leftFixed = true;
                } else {
                    if (it.anchors.leftTo) {
                        if (it.anchors.leftTo == parentWidget()) {
                            left = parentRect.left() + it.anchors.leftOffset;
                        } else {
                            int idx = getWidgetIndex(it.anchors.leftTo);
                            if (idx != -1) {
                                left = m_items[idx].geometry.right() + 1 + it.anchors.leftOffset;
                            }
                        }
                        leftFixed = true;
                    }

                    if (it.anchors.rightTo) {
                        if (it.anchors.rightTo == parentWidget()) {
                            right = parentRect.right() + it.anchors.rightOffset;
                        } else {
                            int idx = getWidgetIndex(it.anchors.rightTo);
                            if (idx != -1) {
                                right = m_items[idx].geometry.left() - 1 + it.anchors.rightOffset;
                            }
                        }
                        rightFixed = true;
                    }
                }

                // 应用水平位置和宽度
                if (leftFixed && rightFixed) {
                    it.geometry.setLeft(left);
                    it.geometry.setRight(right);
                } else if (leftFixed) {
                    it.geometry.moveLeft(left);
                    it.geometry.setWidth(s.width());
                } else if (rightFixed) {
                    it.geometry.moveRight(right);
                    it.geometry.setWidth(s.width());
                } else {
                    it.geometry.moveLeft(parentRect.left());
                    it.geometry.setWidth(s.width());
                }

                // 垂直解析
                if (it.anchors.verticalCenter) {
                    top = parentRect.top() + (parentRect.height() - s.height()) / 2 + it.anchors.verticalCenterOffset;
                    topFixed = true;
                } else {
                    if (it.anchors.topTo) {
                        if (it.anchors.topTo == parentWidget()) {
                            top = parentRect.top() + it.anchors.topOffset;
                        } else {
                            int idx = getWidgetIndex(it.anchors.topTo);
                            if (idx != -1) {
                                top = m_items[idx].geometry.bottom() + 1 + it.anchors.topOffset;
                            }
                        }
                        topFixed = true;
                    }

                    if (it.anchors.bottomTo) {
                        if (it.anchors.bottomTo == parentWidget()) {
                            bottom = parentRect.bottom() + it.anchors.bottomOffset;
                        } else {
                            int idx = getWidgetIndex(it.anchors.bottomTo);
                            if (idx != -1) {
                                bottom = m_items[idx].geometry.top() - 1 + it.anchors.bottomOffset;
                            }
                        }
                        bottomFixed = true;
                    }
                }

                // 应用垂直位置和高度
                if (topFixed && bottomFixed) {
                    it.geometry.setTop(top);
                    it.geometry.setBottom(bottom);
                } else if (topFixed) {
                    it.geometry.moveTop(top);
                    it.geometry.setHeight(s.height());
                } else if (bottomFixed) {
                    it.geometry.moveBottom(bottom);
                    it.geometry.setHeight(s.height());
                } else {
                    it.geometry.moveTop(parentRect.top());
                    it.geometry.setHeight(s.height());
                }
            }

            if (it.geometry != oldGeom) changed = true;
        }

        if (!changed) break;
    }

    // 3. 应用结果到真实的 Widget
    for (const Item& it : m_items) {
        if (QWidget* w = it.item->widget()) {
            w->setGeometry(it.geometry);
        }
    }
}
