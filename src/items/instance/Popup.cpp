#include "Popup.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEasingCurve>

Popup::Popup(QWidget *parent) : ResponsiveDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(m_shadowWidth, m_shadowWidth, m_shadowWidth, m_shadowWidth);

    // 显示动画预配置
    m_showAnimation = new QPropertyAnimation(this, "animationScale", this);
    m_showAnimation->setDuration(300);
    m_showAnimation->setStartValue(0.0);
    m_showAnimation->setEndValue(1.0);
    m_showAnimation->setEasingCurve(QEasingCurve::OutBack);

    // 关闭动画预配置
    m_closeAnimation = new QPropertyAnimation(this, "animationScale", this);
    m_closeAnimation->setDuration(300);
    m_closeAnimation->setStartValue(1.0);
    m_closeAnimation->setEndValue(0.0);
    m_closeAnimation->setEasingCurve(QEasingCurve::InBack);

    connect(m_closeAnimation, &QPropertyAnimation::finished, this, [this]() {
        ResponsiveDialog::setVisible(false);
    });
}

void Popup::setVisible(bool visible) {
    // 逻辑修正：判断“目标状态”是否与“实际期望状态”一致
    // 即使当前物理窗口 isVisible()，但如果正在执行关闭动画，逻辑上也应该视为要“重新显示”
    bool isEffectivelyVisible = isVisible() && m_closeAnimation->state() != QPropertyAnimation::Running;
    if (visible == isEffectivelyVisible) return;

    if (visible) {
        m_closeAnimation->stop();
        m_showAnimation->setStartValue(m_animationScale); // 从当前缩放位置平滑开始
        ResponsiveDialog::setVisible(true);
        m_showAnimation->start();
    } else {
        m_showAnimation->stop();
        m_closeAnimation->setStartValue(m_animationScale); // 从当前缩放位置平滑收缩
        m_closeAnimation->start();
    }
}

void Popup::done(int r) {
    // 拦截 QDialog 的 accept/reject/close 等产生的 done() 调用
    // 先执行动画，动画结束后再调用基类的 done()
    m_showAnimation->stop();
    m_closeAnimation->setStartValue(m_animationScale);
    
    // 重新连接信号，确保 done(r) 被正确调用
    disconnect(m_closeAnimation, &QPropertyAnimation::finished, nullptr, nullptr);
    connect(m_closeAnimation, &QPropertyAnimation::finished, this, [this, r]() {
        ResponsiveDialog::done(r);
    });
    
    m_closeAnimation->start();
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
    ResponsiveDialog::paintEvent(event);
}

