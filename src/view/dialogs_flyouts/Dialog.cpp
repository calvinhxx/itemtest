#include "Dialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLayout>
#include <QScreen>

namespace view::dialogs_flyouts {

// 进场上滑距离（px）：取 shadowSize(16px) 内，不超出窗口边界产生裁切
static constexpr double kSlideDistance = 10.0;

Dialog::Dialog(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    setContentsMargins(m_shadowSize, m_shadowSize, m_shadowSize, m_shadowSize);

    m_animation = new QPropertyAnimation(this, "animationProgress", this);

    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        if (m_isClosing) {
            // 出场结束：真正关闭
            m_isClosing = false;
            QDialog::done(m_closingResult);
        } else {
            // 进场结束：恢复正常模式，显示真实子控件
            m_isAnimating = false;
            for (auto* w : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
                w->show();
            m_snapshot = QPixmap();
            update();
        }
    });

    onThemeUpdated();
}

void Dialog::setAnimationProgress(double p) {
    m_animationProgress = p;
    update();
}

// ── 公开显示入口（在 show 之前预渲染） ───────────────────────────────────────

void Dialog::open() {
    if (m_animationEnabled && !isVisible())
        prepareAnimation();
    QDialog::open();
}

int Dialog::exec() {
    if (m_animationEnabled && !isVisible())
        prepareAnimation();
    return QDialog::exec();
}

// ── 预渲染核心 ────────────────────────────────────────────────────────────────

void Dialog::prepareAnimation() {
    // 此时 widget 仍为 hidden，OS 还未显示任何内容
    ensurePolished();
    if (layout()) layout()->activate();

    // 若尺寸尚未确定（初次显示前），用 sizeHint 确保渲染尺寸正确
    if (size().isEmpty())
        adjustSize();

    // 用 render() 渲染真实内容到快照（不走屏幕，不触发 OS 合成）
    m_snapshot = renderSnapshot();

    // 进入动画模式：此后任何 paintEvent 都使用快照，不会渲染真实内容
    m_isAnimating      = true;
    m_animationProgress = 0.0;

    // 隐藏子控件（进场动画结束后恢复）
    for (auto* w : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
        w->hide();
}

QPixmap Dialog::renderSnapshot() const {
    // 临时关闭动画模式，让 paintEvent 绘制真实背景
    const_cast<Dialog*>(this)->m_isAnimating = false;

    QPixmap pix(size() * devicePixelRatio());
    pix.setDevicePixelRatio(devicePixelRatio());
    pix.fill(Qt::transparent);
    const_cast<Dialog*>(this)->render(&pix);

    const_cast<Dialog*>(this)->m_isAnimating = true;
    return pix;
}

// ── 显示事件 ─────────────────────────────────────────────────────────────────

void Dialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    if (!m_animationEnabled) {
        m_animationProgress = 1.0;
        m_isAnimating = false;
        return;
    }

    m_isClosing = false;

    // 若 exec()/open() 未被调用（如直接 show()），在此补做快照
    if (m_snapshot.isNull()) {
        if (layout()) layout()->activate();
        m_snapshot = renderSnapshot();
        m_isAnimating = true;
        m_animationProgress = 0.0;
        for (auto* w : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
            w->hide();
    }

    // ── 关键：同步刷新 backing store ─────────────────────────────────────────
    // macOS Core Animation 的 orderFront: 在 showEvent 之前就已将窗口提交给
    // compositor，但实际合成帧要等到当前 run loop 结束。
    // repaint() 强制在此之前把 backing store 更新为动画初始态（透明），
    // compositor 看到的第一帧便是"透明"，而不是真实对话框内容。
    repaint();

    // 启动进场动画
    const auto& anim = themeAnimation();
    m_animation->stop();
    m_animation->setDuration(anim.normal);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(anim.entrance);
    m_animation->start();
}

// ── 关闭 ─────────────────────────────────────────────────────────────────────

void Dialog::done(int r) {
    if (!m_animationEnabled) {
        QDialog::done(r);
        return;
    }

    if (m_isClosing) return;
    m_isClosing     = true;
    m_closingResult = r;

    m_animation->stop();

    if (!m_isAnimating) {
        // 正常模式：子控件可见，先捕获快照
        m_snapshot         = renderSnapshot();
        m_isAnimating      = true;
        m_animationProgress = 1.0;

        // ── 关键：同步刷新 backing store ─────────────────────────────────────
        // 先 repaint()，让 compositor 拿到"有快照、无子控件"的一帧；
        // 再 hide 子控件，避免出现"子控件消失但快照还没提交"导致的空白闪帧。
        repaint();

        for (auto* w : findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
            w->hide();
    }
    // else：进场动画被打断，复用当前快照从当前进度反向播放

    // 启动出场动画
    const auto& anim = themeAnimation();
    m_animation->setDuration(anim.fast);
    m_animation->setStartValue(m_animationProgress);
    m_animation->setEndValue(0.0);
    m_animation->setEasingCurve(anim.exit);
    m_animation->start();
}

// ── 鼠标拖拽 ─────────────────────────────────────────────────────────────────

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

// ── 绘制 ─────────────────────────────────────────────────────────────────────

void Dialog::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    if (m_isAnimating) {
        if (!m_snapshot.isNull()) {
            // 动画帧：淡入 + 轻微上滑（纯 painter 操作，全平台一致）
            painter.setOpacity(m_animationProgress);
            const double ty = kSlideDistance * (1.0 - m_animationProgress);
            painter.translate(0.0, ty);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawPixmap(0, 0, m_snapshot);
        }
        // 快照尚未就绪（极端情况）：画透明，避免任何真实内容出现
        return;
    }

    // 普通模式：绘制阴影 + 圆角背景
    const QRect contentRect = rect().adjusted(m_shadowSize, m_shadowSize, -m_shadowSize, -m_shadowSize);
    drawShadow(painter, contentRect);

    const auto& colors = themeColors();
    const int r = themeRadius().overlay;
    painter.setBrush(colors.bgLayer);
    painter.setPen(colors.strokeDefault);
    painter.drawRoundedRect(contentRect, r, r);
}

void Dialog::drawShadow(QPainter& painter, const QRect& contentRect) {
    const auto& s = themeShadow(Elevation::High);
    const int layers     = 10;
    const int spreadStep = 1;
    const int r          = themeRadius().overlay;

    for (int i = 0; i < layers; ++i) {
        const double ratio = 1.0 - static_cast<double>(i) / layers;
        QColor sc = s.color;
        sc.setAlphaF(s.opacity * ratio * 0.35);

        painter.setPen(Qt::NoPen);
        painter.setBrush(sc);

        const int spread  = i * spreadStep;
        const int offsetY = 2;
        painter.drawRoundedRect(
            contentRect.adjusted(-spread, -spread, spread, spread).translated(0, offsetY),
            r + spread, r + spread);
    }
}

} // namespace view::dialogs_flyouts
