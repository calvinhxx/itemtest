#include "Popup.h"
#include <QPainter>
#include <QPainterPath>
#include <QRegion>
#include <QMouseEvent>

Popup::Popup(QWidget *parent) : ResponsiveDialog(parent) {
    // 1. 去掉系统默认标题栏
    // 增加 Qt::CustomizeWindowHint 以确保在所有 Windows 版本上都能隐藏标题栏
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::CustomizeWindowHint);
    
    // 2. 设置背景透明，以便实现自定义的圆角效果
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置默认边距，给阴影留出空间
    setContentsMargins(m_shadowWidth, m_shadowWidth, m_shadowWidth, m_shadowWidth);
}

void Popup::setCornerRadius(int radius) {
    if (m_cornerRadius == radius) return;
    m_cornerRadius = radius;
    update();
}

void Popup::setBorderColor(const QColor &color) {
    m_borderColor = color;
    update();
}

void Popup::setShadowWidth(int width) {
    m_shadowWidth = width;
    setContentsMargins(m_shadowWidth, m_shadowWidth, m_shadowWidth, m_shadowWidth);
    update();
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
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 1. 绘制阴影效果
    // 实际内容区域需要缩进 m_shadowWidth
    QRect contentRect = rect().adjusted(m_shadowWidth, m_shadowWidth, -m_shadowWidth, -m_shadowWidth);
    
    for (int i = 0; i < m_shadowWidth; ++i) {
        QPainterPath shadowPath;
        // 阴影逐层扩散，透明度逐层降低
        int alpha = 30 * (m_shadowWidth - i) / m_shadowWidth;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, alpha));
        
        // 绘制比内容区域稍大的圆角矩形作为阴影层
        shadowPath.addRoundedRect(contentRect.adjusted(-i, -i, i, i), m_cornerRadius + i, m_cornerRadius + i);
        painter.drawPath(shadowPath);
    }
    
    // 2. 绘制主体背景
    QPainterPath path;
    path.addRoundedRect(contentRect, m_cornerRadius, m_cornerRadius);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().window());
    painter.drawPath(path);
    
    // 3. 绘制边框
    if (m_borderWidth > 0) {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(m_borderColor, m_borderWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
}

