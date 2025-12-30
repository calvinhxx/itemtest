#include "Popup.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEasingCurve>
#include <QLayout>

Popup::Popup(QWidget *parent) : Dialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(m_shadowWidth, m_shadowWidth, m_shadowWidth, m_shadowWidth);

    m_animation = new QPropertyAnimation(this, "animationScale", this);
    m_animation->setDuration(300);

    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        m_isAnimating = false;
        if (m_animationScale < 0.01) {
            m_isFinalizing = true;
            Dialog::done(m_resultCode);
            m_isFinalizing = false;
        } else {
            // 动画结束，显示真实子控件，恢复交互
            for (auto* child : findChildren<QWidget*>()) {
                if (child->parent() == this) child->show();
            }
            update();
        }
    });
}

void Popup::updateSnapshot() {
    // 隐藏自身背景，仅捕捉内容
    m_isAnimating = false;
    for (auto* child : findChildren<QWidget*>()) {
        if (child->parent() == this) child->show();
    }
    
    // 强制布局生效并渲染快照
    if (layout()) layout()->activate();
    m_contentSnapshot = this->grab(rect());
    
    // 进入动画模式：隐藏真实控件
    m_isAnimating = true;
    for (auto* child : findChildren<QWidget*>()) {
        if (child->parent() == this) child->hide();
    }
}

void Popup::setVisible(bool visible) {
    if (m_isFinalizing) {
        Dialog::setVisible(visible);
        return;
    }

    if (visible) {
        if (isVisible() && m_animation->state() == QPropertyAnimation::Running && m_animation->endValue().toReal() > 0.5) return;

        m_animation->stop();
        Dialog::setVisible(true); // 必须先 show 才能 grab
        updateSnapshot();

        m_animation->setStartValue(m_animationScale);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::OutBack);
        m_animation->start();
    } else {
        if (!isVisible() || (m_animation->state() == QPropertyAnimation::Running && m_animation->endValue().toReal() < 0.5)) return;

        m_animation->stop();
        updateSnapshot();

        m_animation->setStartValue(m_animationScale);
        m_animation->setEndValue(0.0);
        m_animation->setEasingCurve(QEasingCurve::InBack);
        m_animation->start();
    }
}

void Popup::done(int r) {
    m_resultCode = r;
    setVisible(false);
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
    painter.setRenderHint(QPainter::SmoothPixmapTransform); // 保证快照缩放平滑

    // 整个窗口内容的中心缩放
    painter.translate(rect().center());
    painter.scale(m_animationScale, m_animationScale);
    painter.translate(-rect().center());

    if (m_isAnimating && !m_contentSnapshot.isNull()) {
        // 动画模式：绘制快照（包含阴影、背景和所有子控件）
        painter.drawPixmap(0, 0, m_contentSnapshot);
    } else {
        // 非动画模式：绘制基础背景（子控件由 Qt 自动渲染）
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
}
