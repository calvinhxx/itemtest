#include "Dialog.h"
#include <QPainter>
#include <QMouseEvent>

namespace view::dialogs_flyouts {

Dialog::Dialog(QWidget *parent) : QDialog(parent) {
    // 1. 配置顶层窗口属性
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    
    // 2. 直接在窗口上预留阴影占位边距
    setContentsMargins(m_shadowSize, m_shadowSize, m_shadowSize, m_shadowSize);
    
    onThemeUpdated();
}

void Dialog::mousePressEvent(QMouseEvent *event) {
    if (m_dragEnabled && event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    QDialog::mousePressEvent(event);
}

void Dialog::mouseMoveEvent(QMouseEvent *event) {
    if (cursor().shape() == Qt::ClosedHandCursor) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
    QDialog::mouseMoveEvent(event);
}

void Dialog::mouseReleaseEvent(QMouseEvent *event) {
    if (cursor().shape() == Qt::ClosedHandCursor) {
        unsetCursor();
    }
    QDialog::mouseReleaseEvent(event);
}

void Dialog::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 显式清除整个窗口背景为透明（解决白色底色问题的核心）
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    
    // 内容区域 = 控件大小 - 阴影边距
    QRect contentRect = rect().adjusted(m_shadowSize, m_shadowSize, -m_shadowSize, -m_shadowSize);
    
    // 2. 绘制阴影
    painter.save();
    drawShadow(painter, contentRect);
    painter.restore();

    // 3. 绘制主体圆角背景与描边
    const auto& colors = themeColors();
    int r = themeRadius().topLevel;
    
    painter.setBrush(colors.bgLayer);
    painter.setPen(colors.strokeDefault);
    painter.drawRoundedRect(contentRect.adjusted(1, 1, -1, -1), r, r);
}

void Dialog::drawShadow(QPainter& painter, const QRect& contentRect) {
    const auto& s = themeShadow(Elevation::High);
    int blur = m_shadowSize;
    int r = themeRadius().topLevel;
    
    for (int i = 0; i < blur; ++i) {
        double ratio = (1.0 - (double)i / blur);
        QColor shadowColor = s.color;
        shadowColor.setAlphaF(s.opacity * ratio);
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadowColor);
        
        QRect shadowRect = contentRect.adjusted(-i, -i, i, i);
        shadowRect.translate(s.offsetX, s.offsetY);
        painter.drawRoundedRect(shadowRect, r + i, r + i);
    }
}

} // namespace view::dialogs_flyouts
