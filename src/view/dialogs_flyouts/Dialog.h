#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QPainter>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "design/Spacing.h"

class QHideEvent;

namespace view::dialogs_flyouts {

// ── SmokeOverlay ──────────────────────────────────────────────────────────────
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
    QColor  m_color{0, 0, 0, 102};
    double  m_progress = 0.0;
};
/**
 * @brief Dialog — Fluent Design 基础对话框
 *
 * 纯 view 层基础控件：圆角背景 + 阴影 + 进/出场动画 + 拖拽。
 * 不包含任何预置子控件，由派生类（如 ContentDialog）或使用方自行组装内容。
 *
 * ── 动画方案（Opacity）────────────────────────────────────────────────────────
 *  进场：opacity 0→1。退场：opacity 1→0。
 *  顶层 QDialog (Qt::Window) 不支持 QGraphicsOpacityEffect，透明度通过
 *  setWindowOpacity 控制；Smoke 蒙层通过自定义 progress 属性 + paintEvent
 *  插值 alpha 实现淡入淡出（由 m_smokeAnim 驱动）。
 */
class Dialog : public QDialog, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(double animationProgress READ animationProgress WRITE setAnimationProgress)
    Q_PROPERTY(bool dragEnabled READ isDragEnabled WRITE setDragEnabled)
    Q_PROPERTY(bool animationEnabled READ isAnimationEnabled WRITE setAnimationEnabled)
public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

    void onThemeUpdated() override;

    int  shadowSize() const { return m_shadowSize; }

    void setDragEnabled(bool e)      { m_dragEnabled = e; }
    bool isDragEnabled()  const      { return m_dragEnabled; }

    void setSmokeEnabled(bool e)     { m_smokeEnabled = e; }
    bool isSmokeEnabled() const      { return m_smokeEnabled; }

    void setAnimationEnabled(bool e) { m_animationEnabled = e; }
    bool isAnimationEnabled() const  { return m_animationEnabled; }

    double animationProgress() const { return m_animationProgress; }
    void   setAnimationProgress(double p);

    void open() override;
    int  exec() override;

    void done(int r) override;

protected:
    bool isAnimating() const { return m_isAnimating; }

    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event)   override;
    void hideEvent(QHideEvent* event)   override;

    void mousePressEvent(QMouseEvent* event)   override;
    void mouseMoveEvent(QMouseEvent* event)    override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void drawShadow(QPainter& painter, const QRect& contentRect);

private:
    void showSmokeOverlay();
    void hideSmokeOverlay();

    const int m_shadowSize = ::Spacing::Standard;

    bool   m_smokeEnabled     = false;
    bool   m_dragEnabled       = true;
    QPoint m_dragPosition;
    SmokeOverlay* m_smokeOverlay   = nullptr;

    bool   m_animationEnabled  = true;
    bool   m_isAnimating       = false;
    bool   m_isClosing         = false;
    double m_animationProgress = 1.0;
    int    m_closingResult     = 0;

    QPropertyAnimation* m_animation;
    QPropertyAnimation* m_smokeAnim       = nullptr;  // Smoke 淡入淡出动画
    bool                m_smokeFadingOut  = false;    // 标记 Smoke 当前是否正在淡出
    QSize               m_targetSize;
    QSize               m_savedMinSize;
    QSize               m_savedMaxSize;
};

} // namespace view::dialogs_flyouts

#endif // DIALOG_H
