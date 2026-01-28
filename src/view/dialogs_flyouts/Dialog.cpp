#include "Dialog.h"
#include <QPainter>
#include <QMouseEvent>

namespace view::dialogs_flyouts {

Dialog::Dialog(QWidget *parent) : QDialog(parent) {
    // 1. 配置顶层窗口属性
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    
    // 2. 直接在窗口上预留阴影占位边距
    setContentsMargins(m_shadowSize, m_shadowSize, m_shadowSize, m_shadowSize);
    
    // 3. 初始化动画
    m_animation = new QPropertyAnimation(this, "animationProgress", this);
    
    // 处理动画结束回调
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        if (m_animationProgress > 0.5) { 
            // 进场动画结束：恢复真实控件并隐藏快照
            m_isAnimating = false;
            for (auto* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
                child->show();
            }
            m_snapshot = QPixmap();
            update();
        } else {
            // 出场动画结束：正式关闭窗口
            QDialog::done(m_closingResult);
        }
    });
    
    onThemeUpdated();
}

void Dialog::setAnimationProgress(double progress) {
    m_animationProgress = progress;
    update();
}

void Dialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    
    if (!m_animationEnabled) {
        m_animationProgress = 1.0;
        m_isAnimating = false;
        return;
    }

    // 捕获快照
    if (layout()) layout()->activate();
    m_isAnimating = true;
    m_snapshot = this->grab(rect());
    
    // 动画期间隐藏真实子控件，防止它们以 1:1 比例在缩放层之上重复绘制
    for (auto* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        child->hide();
    }
    
    m_animation->stop();
    m_animation->setDuration(themeAnimation().normal);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(themeAnimation().entrance);
    m_animation->start();
}

void Dialog::done(int r) {
    if (!m_animationEnabled) {
        QDialog::done(r);
        return;
    }

    if (m_animation->state() == QAbstractAnimation::Running && m_animation->endValue().toDouble() == 0.0) {
        return;
    }

    m_closingResult = r;
    
    // 捕获关闭时的快照
    m_isAnimating = true;
    m_snapshot = this->grab(rect());
    for (auto* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        child->hide();
    }
    
    m_animation->stop();
    m_animation->setDuration(themeAnimation().fast);
    m_animation->setStartValue(m_animationProgress);
    m_animation->setEndValue(0.0);
    m_animation->setEasingCurve(themeAnimation().exit);
    m_animation->start();
}

void Dialog::mousePressEvent(QMouseEvent *event) {
    if (m_dragEnabled && event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    QDialog::mousePressEvent(event);
}

void Dialog::mouseMoveEvent(QMouseEvent *event) {
    if (cursor().shape() == Qt::ClosedHandCursor) {
        move(event->globalPosition().toPoint() - m_dragPosition);
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
    painter.setRenderHint(QPainter::SmoothPixmapTransform); 

    // --- 应用增强版动画效果 ---
    if (m_animationEnabled) {
        painter.setOpacity(m_animationProgress);
        
        // 1. 空间位移：向上滑入效果（从下方 20px 位置滑入原位）
        double translateY = 20.0 * (1.0 - m_animationProgress);
        painter.translate(0, translateY);
        
        // 2. 缩放增强：从 0.9 倍缩放到 1.0 倍
        double scale = 0.90 + (0.10 * m_animationProgress);
        painter.translate(rect().center());
        painter.scale(scale, scale);
        painter.translate(-rect().center());
    }

    // 1. 显式清除背景
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    if (m_isAnimating && !m_snapshot.isNull()) {
        // --- 动画模式：绘制整体快照（内容随背景同步缩放） ---
        painter.drawPixmap(0, 0, m_snapshot);
    } else {
        // --- 正常模式：绘制自绘层（子控件由 Qt 自动渲染） ---
        QRect contentRect = rect().adjusted(m_shadowSize, m_shadowSize, -m_shadowSize, -m_shadowSize);
        
        painter.save();
        drawShadow(painter, contentRect);
        painter.restore();

        const auto& colors = themeColors();
        int r = themeRadius().topLevel;
        
        painter.setBrush(colors.bgLayer);
        painter.setPen(colors.strokeDefault);
        painter.drawRoundedRect(contentRect.adjusted(1, 1, -1, -1), r, r);
    }
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
