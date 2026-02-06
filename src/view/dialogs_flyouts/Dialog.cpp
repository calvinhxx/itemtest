#include "Dialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLayout>

namespace view::dialogs_flyouts {

Dialog::Dialog(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    
    // 固定边距，动画期间通过 Painter 变换实现位移，而不是修改 Margins
    setContentsMargins(m_shadowSize, m_shadowSize, m_shadowSize, m_shadowSize);
    
    m_animation = new QPropertyAnimation(this, "animationProgress", this);
    
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        // 进场动画结束
        if (m_animationProgress > 0.5) {
            m_isAnimating = false;
            // 恢复真实控件显示
            for (auto* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
                child->show();
            }
            m_snapshot = QPixmap(); // 释放内存
            update();
        } else {
            // 出场动画结束
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

    // 1. 准备捕获快照：此时 m_isAnimating 为 false，paintEvent 会绘制真实阴影和背景
    if (layout()) layout()->activate();
    
    m_snapshot = QPixmap(size() * devicePixelRatio());
    m_snapshot.setDevicePixelRatio(devicePixelRatio());
    m_snapshot.fill(Qt::transparent);
    render(&m_snapshot); // 捕获包含阴影和背景的完整样子
    
    // 2. 进入动画模式
    m_isAnimating = true;
    for (auto* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        child->hide();
    }
    
    const auto& anim = themeAnimation();
    m_animation->stop();
    m_animation->setDuration(anim.normal); 
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(anim.entrance);
    m_animation->start();
}

void Dialog::done(int r) {
    if (!m_animationEnabled) {
        QDialog::done(r);
        return;
    }

    if (m_animation->state() == QAbstractAnimation::Running && m_animation->endValue().toDouble() == 0.0) return;

    m_closingResult = r;
    
    // 1. 准备出场快照
    m_isAnimating = false;
    for (auto* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        child->show();
    }
    
    m_snapshot = QPixmap(size() * devicePixelRatio());
    m_snapshot.setDevicePixelRatio(devicePixelRatio());
    m_snapshot.fill(Qt::transparent);
    render(&m_snapshot);
    
    // 2. 进入动画模式
    m_isAnimating = true;
    for (auto* child : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        child->hide();
    }

    const auto& anim = themeAnimation();
    m_animation->stop();
    m_animation->setDuration(anim.fast); 
    m_animation->setStartValue(m_animationProgress);
    m_animation->setEndValue(0.0);
    m_animation->setEasingCurve(anim.exit);
    m_animation->start();
}

void Dialog::mousePressEvent(QMouseEvent *event) {
    if (m_dragEnabled && event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
#else
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
#endif
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    QDialog::mousePressEvent(event);
}

void Dialog::mouseMoveEvent(QMouseEvent *event) {
    if (cursor().shape() == Qt::ClosedHandCursor) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        move(event->globalPosition().toPoint() - m_dragPosition);
#else
        move(event->globalPos() - m_dragPosition);
#endif
        event->accept();
    }
    QDialog::mouseMoveEvent(event);
}

void Dialog::mouseReleaseEvent(QMouseEvent *event) {
    if (cursor().shape() == Qt::ClosedHandCursor) unsetCursor();
    QDialog::mouseReleaseEvent(event);
}

void Dialog::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 显式清除背景
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    
    if (m_isAnimating && !m_snapshot.isNull()) {
        // --- 动画模式：对称变换 (Slide + Scale + Fade) ---
        // 透明度跟随进度
        painter.setOpacity(m_animationProgress);
        
        // 位移：从下方 30px 滑入
        double translateY = 30.0 * (1.0 - m_animationProgress);
        // 缩放：从 0.9 倍缩放到 1.0 倍 (这样阴影也会跟着缩放)
        double scale = 0.9 + (0.1 * m_animationProgress);
        
        painter.translate(rect().center().x(), rect().center().y() + translateY);
        painter.scale(scale, scale);
        painter.translate(-rect().center().x(), -rect().center().y());
        
        // 性能优化：移动中关闭平滑缩放以保证帧率
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawPixmap(0, 0, m_snapshot);
    } else {
        // --- 正常模式：绘制真实背景与阴影 ---
    QRect contentRect = rect().adjusted(m_shadowSize, m_shadowSize, -m_shadowSize, -m_shadowSize);
    drawShadow(painter, contentRect);

    const auto& colors = themeColors();
    int r = themeRadius().topLevel;
    painter.setBrush(colors.bgLayer);
    painter.setPen(colors.strokeDefault);
        painter.drawRoundedRect(contentRect, r, r);
    }
}

void Dialog::drawShadow(QPainter& painter, const QRect& contentRect) {
    const auto& s = themeShadow(Elevation::High);
    const int layers = 10;   // 层数足够平滑淡出
    const int spreadStep = 1; // 每层扩散 1px，总范围约 10px，配合 16px 边距自然淡出
    int r = themeRadius().topLevel;

    for (int i = 0; i < layers; ++i) {
        double ratio = 1.0 - static_cast<double>(i) / layers;
        QColor sc = s.color;
        sc.setAlphaF(s.opacity * ratio * 0.35);

        painter.setPen(Qt::NoPen);
        painter.setBrush(sc);

        int spread = i * spreadStep;
        int offsetY = 2; // 轻微下偏，立体感又不显重
        painter.drawRoundedRect(
            contentRect.adjusted(-spread, -spread, spread, spread).translated(0, offsetY),
            r + spread, r + spread);
    }
}

} // namespace view::dialogs_flyouts
