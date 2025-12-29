#include "Popup.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEasingCurve>

Popup::Popup(QWidget *parent) : ResponsiveDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(m_shadowWidth, m_shadowWidth, m_shadowWidth, m_shadowWidth);

    m_animation = new QPropertyAnimation(this, "animationScale", this);
    m_animation->setDuration(300);

    // 只在这里连接一次，逻辑更优雅
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        if (m_animationScale < 0.1) {
            ResponsiveDialog::setVisible(false);
        }
    });
}

void Popup::setVisible(bool visible) {
    if (visible == isVisible() && m_animation->state() != QPropertyAnimation::Running) return;

    m_animation->stop();
    if (visible) {
        m_animation->setStartValue(m_animationScale);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::OutBack);
        
        ResponsiveDialog::setVisible(true);
        m_animation->start();
    } else {
        m_animation->setStartValue(m_animationScale);
        m_animation->setEndValue(0.0);
        m_animation->setEasingCurve(QEasingCurve::InBack);
        m_animation->start();
    }
}

void Popup::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void Popup::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void Popup::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    if (m_animationScale < 0.01) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 关键简化：通过变换实现缩放，而不是操作 geometry
    painter.translate(rect().center());
    painter.scale(m_animationScale, m_animationScale);
    painter.translate(-rect().center());

    QRect contentRect = rect().adjusted(m_shadowWidth, m_shadowWidth, -m_shadowWidth, -m_shadowWidth);
    
    // 1. 阴影
    for (int i = 0; i < m_shadowWidth; ++i) {
        int alpha = 30 * (m_shadowWidth - i) / m_shadowWidth;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, alpha));
        painter.drawRoundedRect(contentRect.adjusted(-i, -i, i, i), m_cornerRadius + i, m_cornerRadius + i);
    }
    
    // 2. 背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().window());
    painter.drawRoundedRect(contentRect, m_cornerRadius, m_cornerRadius);
    
    // 3. 边框
    if (m_borderWidth > 0) {
        painter.setPen(QPen(m_borderColor, m_borderWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(contentRect, m_cornerRadius, m_cornerRadius);
    }
}

