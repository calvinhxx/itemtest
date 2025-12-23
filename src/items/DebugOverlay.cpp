#include "DebugOverlay.h"
#include <QPainter>
#include <QEvent>
#include <QChildEvent>
#include <QWidget>
#include <QMouseEvent>

DebugOverlay::DebugOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true); // 不阻塞鼠标事件
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_AlwaysStackOnTop, true);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    if (parent) {
        setGeometry(parent->contentsRect());
        parent->installEventFilter(this);
    }
    show();
}

void DebugOverlay::setSelectedWidget(QWidget *w)
{
    if (m_selected == w) return;
    m_selected = w;
    update();
}

bool DebugOverlay::eventFilter(QObject *obj, QEvent *ev)
{
    QWidget *p = parentWidget();
    if (!p) return QWidget::eventFilter(obj, ev);

    if (obj == p) {
        switch (ev->type()) {
        case QEvent::Resize:
        case QEvent::Move:
            setGeometry(p->contentsRect());
            update();
            break;
        case QEvent::ChildAdded:
        case QEvent::ChildRemoved:
            update();
            break;
        case QEvent::MouseButtonPress:
            if (m_selectionEnabled) {
                QMouseEvent *me = static_cast<QMouseEvent*>(ev);
                QPoint pos = me->pos();
                QWidget *w = p->childAt(pos);
                if (w && w != this) {
                    // 如果是深层子控件，找到它直接挂到父上的顶级子控件（便于 overlay 只画直接子 widget 边框）
                    QWidget *top = w;
                    while (top && top->parentWidget() != p && top->parentWidget() != nullptr) {
                        top = top->parentWidget();
                    }
                    setSelectedWidget(top ? top : w);
                } else {
                    clearSelection();
                }
            }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void DebugOverlay::paintEvent(QPaintEvent * /*ev*/)
{
    if (!m_enabled) return;
    QWidget *p = parentWidget();
    if (!p) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 普通虚线边框（红色）
    QPen penNormal(Qt::red);
    penNormal.setStyle(Qt::DashLine);
    penNormal.setWidth(1);

    // 选中控件高亮（蓝色，实线，加半透明填充）
    QPen penSel(Qt::blue);
    penSel.setStyle(Qt::SolidLine);
    penSel.setWidth(2);
    QColor fillSel(0, 120, 215, 40); // 半透明蓝

    const QObjectList &children = p->children();
    for (QObject *o : children) {
        QWidget *w = qobject_cast<QWidget*>(o);
        if (!w) continue;
        if (w == this) continue;
        if (!w->isVisible()) continue;

        QRect r = w->geometry();
        // normal outline
        painter.setPen(penNormal);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(r.adjusted(0, 0, -1, -1));

        if (m_showLabels) {
            painter.setPen(Qt::yellow);
            QString label = w->objectName().isEmpty() ? QString(w->metaObject()->className()) : w->objectName();
            painter.drawText(r.left() + 3, r.top() + 12, label);
        }
    }

    // draw selected on top
    if (m_selected && m_selected->isVisible()) {
        QRect sr = m_selected->geometry();
        painter.setPen(penSel);
        painter.setBrush(fillSel);
        painter.drawRect(sr.adjusted(0, 0, -1, -1));

        // draw size/pos info
        QString info = QString("x:%1 y:%2 w:%3 h:%4").arg(sr.x()).arg(sr.y()).arg(sr.width()).arg(sr.height());
        painter.setPen(Qt::white);
        painter.drawText(sr.left() + 4, sr.bottom() - 6, info);
    }
}