#include "Dialog.h"
#include <QPainter>
#include <QMouseEvent>
#include <QLayout>
#include <QScreen>
#include <QPointer>
#include <memory>
#include "common/Material.h"

namespace view::dialogs_flyouts {

// 进场缩放起始系数（0.90 → 1.0），与 WinUI 3 ContentDialog pop-in/out 保持一致
static constexpr double kScaleFrom = 0.90;

// ── Smoke 蒙层 ──────────────────────────────────────────────────────────────
//
// SmokeOverlay 通过自定义 progress (0..1) 属性 + paintEvent alpha 插值实现淡入淡出。
// 由 Dialog::m_smokeAnim (QPropertyAnimation) 驱动该属性。
// （顶层窗口不支持 QGraphicsOpacityEffect，因此即便 SmokeOverlay 是子控件，
//   为保持与 Dialog 一致的实现风格，统一使用 paintEvent 自绘 alpha 的方案。）

class SmokeOverlay : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double progress READ progress WRITE setProgress)
public:
    explicit SmokeOverlay(QWidget* parent) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
    void setColor(const QColor& c) { m_color = c; update(); }
    double progress() const { return m_progress; }
    void   setProgress(double p) {
        if (qFuzzyCompare(m_progress, p)) return;
        m_progress = p;
        update();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        QColor c = m_color;
        c.setAlpha(int(m_color.alpha() * qBound(0.0, m_progress, 1.0)));
        p.fillRect(rect(), c);
    }
private:
    QColor m_color{0, 0, 0, 102};
    double m_progress = 0.0;
};

Dialog::Dialog(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    setContentsMargins(m_shadowSize, m_shadowSize, m_shadowSize, m_shadowSize);

    m_animation = new QPropertyAnimation(this, "animationProgress", this);

    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        if (m_isClosing) {
            // 退场完成：先 done() 隐藏窗口，再恢复尺寸/透明度以便 Dialog 复用
            m_isClosing   = false;
            m_isAnimating = false;
            const auto targetSize = m_targetSize;
            m_targetSize  = QSize();
            QDialog::done(m_closingResult);
            // 窗口已隐藏，compositor 看不到下面的恢复操作
            if (!targetSize.isEmpty())
                resize(targetSize);
            setMinimumSize(m_savedMinSize);
            setMaximumSize(m_savedMaxSize);
            setWindowOpacity(1.0);
        } else {
            // 进场完成：恢复尺寸约束
            resize(m_targetSize);
            setMinimumSize(m_savedMinSize);
            setMaximumSize(m_savedMaxSize);
            m_isAnimating = false;
            m_targetSize  = QSize();
        }
    });

    onThemeUpdated();
}

Dialog::~Dialog() {
    // 同步清理烟雾蒙层：Dialog 可能在 smoke 淡出未完成时被销毁
    // （如栈上 ContentDialog，exec() 返回后立即出栈）。
    // m_smokeAnim 以 Dialog 为 parent，随 Dialog 销毁而死，finished 不再触发，
    // 导致 overlay（parent=parentWidget）孤儿残留。因此此处立即销毁。
    if (m_smokeOverlay) {
        m_smokeOverlay->hide();
        delete m_smokeOverlay;
        m_smokeOverlay = nullptr;
    }
}

void Dialog::setAnimationProgress(double p) {
    m_animationProgress = p;

    if (m_isAnimating && !m_targetSize.isEmpty()) {
        // 尺寸插值：kScaleFrom(0.90) → 1.0
        const double scale = kScaleFrom + (1.0 - kScaleFrom) * p;
        const QSize newSize(qRound(m_targetSize.width() * scale),
                            qRound(m_targetSize.height() * scale));

        const QPoint c = geometry().center();
        resize(newSize);
        // 用 size()（实际尺寸）计算居中，因为 setFixedSize 可能约束了 resize
        const QSize actual = size();
        move(c.x() - actual.width() / 2, c.y() - actual.height() / 2);

        setWindowOpacity(p);
    }

    update();
}

// ── 公开显示入口 ─────────────────────────────────────────────────────────────────

void Dialog::open() {
    if (m_smokeEnabled) showSmokeOverlay();
    if (m_animationEnabled && !isVisible()) {
        m_isAnimating       = true;
        m_animationProgress = 0.0;
        setWindowOpacity(0.0);          // compositor 看不到第一帧
    }
    QDialog::open();
}

int Dialog::exec() {
    if (m_smokeEnabled) showSmokeOverlay();
    if (m_animationEnabled && !isVisible()) {
        m_isAnimating       = true;
        m_animationProgress = 0.0;
        setWindowOpacity(0.0);
    }
    int result = QDialog::exec();
    hideSmokeOverlay();
    return result;
}

// ── 显示事件 ─────────────────────────────────────────────────────────────────

void Dialog::showEvent(QShowEvent *event) {
    // 蒙层模式：居中于父窗口
    if (m_smokeEnabled && parentWidget()) {
        QPoint center = parentWidget()->mapToGlobal(parentWidget()->rect().center());
        move(center.x() - width() / 2, center.y() - height() / 2);
    }

    QDialog::showEvent(event);

    if (!m_animationEnabled || !m_isAnimating) {
        m_animationProgress = 1.0;
        m_isAnimating = false;
        setWindowOpacity(1.0);
        return;
    }

    m_isClosing = false;

    // Widget 已在窗口体系中 — 递归 polish + 字体初始化
    ensurePolished();
    for (auto* w : findChildren<QWidget*>()) {
        w->ensurePolished();
        if (auto* fe = dynamic_cast<FluentElement*>(w))
            fe->onThemeUpdated();
    }
    if (layout()) layout()->activate();
    for (auto* w : findChildren<QWidget*>()) {
        if (w->layout()) w->layout()->activate();
    }

    // 记录目标尺寸，临时放开 min/max 约束以允许动画期间 resize
    m_targetSize   = size();
    m_savedMinSize = minimumSize();
    m_savedMaxSize = maximumSize();
    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    // 缩小到起始尺寸（90%），保持中心位置不变
    const QSize startSize(qRound(m_targetSize.width() * kScaleFrom),
                          qRound(m_targetSize.height() * kScaleFrom));
    const QPoint c = geometry().center();
    resize(startSize);
    move(c.x() - startSize.width() / 2, c.y() - startSize.height() / 2);

    const auto& anim = themeAnimation();
    m_animation->stop();
    m_animation->setDuration(anim.fast);     // 入场使用 fast(150ms)，贴近 WinUI 3 GIF 节奏
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(anim.entrance);
    m_animation->start();
}

// ── 隐藏事件 ─────────────────────────────────────────────────────────────────

void Dialog::hideEvent(QHideEvent* event) {
    hideSmokeOverlay();
    QDialog::hideEvent(event);
}

// ── 关闭 ─────────────────────────────────────────────────────────────────────

void Dialog::done(int r) {
    if (!m_animationEnabled) {
        // macOS Core Animation 会在 hide() 时对当前帧播放系统窗口消失动画。
        // 先将窗口设为全透明，避免内容 "闪缩"。
        setWindowOpacity(0.0);
        QDialog::done(r);
        setWindowOpacity(1.0);   // 恢复，以便 Dialog 复用
        return;
    }

    if (m_isClosing) return;
    m_isClosing     = true;
    m_closingResult = r;

    m_animation->stop();

    if (!m_isAnimating) {
        m_targetSize        = size();
        m_savedMinSize      = minimumSize();
        m_savedMaxSize      = maximumSize();
        setMinimumSize(QSize(0, 0));
        setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
        m_isAnimating       = true;
        m_animationProgress = 1.0;
    }

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

// ── Smoke 蒙层管理 ──────────────────────────────────────────────────────────

void Dialog::showSmokeOverlay() {
    if (!parentWidget() || !parentWidget()->isVisible()) return;

    SmokeOverlay* overlay = static_cast<SmokeOverlay*>(m_smokeOverlay);

    // 若 overlay 不存在则创建；若存在（可能正在淡出中）则复用并反向
    if (!overlay) {
        overlay = new SmokeOverlay(parentWidget());
        const auto& smoke = themeSmoke();
        QColor c = smoke.baseColor;
        c.setAlphaF(smoke.opacity);
        overlay->setColor(c);
        overlay->setGeometry(parentWidget()->rect());
        overlay->setProgress(0.0);
        overlay->show();
        overlay->raise();
        m_smokeOverlay = overlay;
    }

    // 懒创建动画
    if (!m_smokeAnim) {
        m_smokeAnim = new QPropertyAnimation(this);
        m_smokeAnim->setPropertyName("progress");
    }
    m_smokeAnim->stop();
    m_smokeAnim->setTargetObject(overlay);
    const auto& anim = themeAnimation();
    m_smokeAnim->setDuration(anim.fast);     // 与 Dialog 主动画同步用 fast(150ms)
    m_smokeAnim->setStartValue(overlay->progress());
    m_smokeAnim->setEndValue(1.0);
    m_smokeAnim->setEasingCurve(anim.entrance);
    m_smokeFadingOut = false;
    m_smokeAnim->start();
}

void Dialog::hideSmokeOverlay() {
    SmokeOverlay* overlay = static_cast<SmokeOverlay*>(m_smokeOverlay);
    if (!overlay) return;
    if (m_smokeFadingOut) return;  // 已经在淡出中

    if (!m_smokeAnim) {
        m_smokeAnim = new QPropertyAnimation(this);
        m_smokeAnim->setPropertyName("progress");
    }
    m_smokeAnim->stop();
    m_smokeAnim->setTargetObject(overlay);
    const auto& anim = themeAnimation();
    m_smokeAnim->setDuration(anim.fast);
    m_smokeAnim->setStartValue(overlay->progress());
    m_smokeAnim->setEndValue(0.0);
    m_smokeAnim->setEasingCurve(anim.exit);
    m_smokeFadingOut = true;

    // 一次性 finished 连接，淡出完成后销毁 overlay
    QPointer<SmokeOverlay> safeOverlay(overlay);
    auto conn = std::make_shared<QMetaObject::Connection>();
    *conn = connect(m_smokeAnim, &QPropertyAnimation::finished, this,
                    [this, safeOverlay, conn]() {
        QObject::disconnect(*conn);
        if (!m_smokeFadingOut) return;  // 中途被 showSmokeOverlay 反向
        m_smokeFadingOut = false;
        if (safeOverlay) {
            safeOverlay->hide();
            safeOverlay->deleteLater();
        }
        if (m_smokeOverlay == safeOverlay.data())
            m_smokeOverlay = nullptr;
    });

    m_smokeAnim->start();
}

void Dialog::onThemeUpdated() {
    update();
    if (m_smokeOverlay) {
        const auto& smoke = themeSmoke();
        QColor c = smoke.baseColor;
        c.setAlphaF(smoke.opacity);
        static_cast<SmokeOverlay*>(m_smokeOverlay)->setColor(c);
    }
}

// ── 绘制 ─────────────────────────────────────────────────────────────────────

void Dialog::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

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

// SmokeOverlay 的 Q_OBJECT 在 .cpp 中声明，需要包含 moc 生成文件
#include "Dialog.moc"
