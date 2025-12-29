#include "Popup.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEasingCurve>

Popup::Popup(QWidget *parent) : ResponsiveDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(m_shadowWidth, m_shadowWidth, m_shadowWidth, m_shadowWidth);

    // 单个动画对象，处理所有缩放逻辑
    m_animation = new QPropertyAnimation(this, "animationScale", this);
    m_animation->setDuration(300);

    // 统一的动画结束处理
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        if (m_animationScale < 0.01) {
            // 只有在收缩到 0 时，才执行真正的 QDialog 结束逻辑
            ResponsiveDialog::done(m_resultCode);
        }
    });
}

void Popup::setVisible(bool visible) {
    if (visible) {
        m_animation->stop();
        m_animation->setStartValue(m_animationScale);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::OutBack);
        
        ResponsiveDialog::setVisible(true);
        m_animation->start();
    } else {
        // 如果当前比例已经接近 0，说明已经是“最终隐藏”阶段，直接调用基类
        if (m_animationScale < 0.01 || !isVisible()) {
            ResponsiveDialog::setVisible(false);
            return;
        }

        // 否则，拦截隐藏请求，启动收缩动画
        m_animation->stop();
        m_animation->setStartValue(m_animationScale);
        m_animation->setEndValue(0.0);
        m_animation->setEasingCurve(QEasingCurve::InBack);
        m_animation->start();
    }
}

void Popup::done(int r) {
    m_resultCode = r;
    // hide() 本质会调用 setVisible(false)，从而触发上面的动画逻辑
    hide();
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
    ResponsiveDialog::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

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

