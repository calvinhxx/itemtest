#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPropertyAnimation>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"

namespace view::dialogs_flyouts {

/**
 * @brief Dialog - 现代设计风格的对话框
 *
 * ── 动画方案（全平台统一）──────────────────────────────────────────────────
 *  快照 + painter 变换：setOpacity + translate，纯软件合成，不依赖任何 OS 合成器
 *  API，Windows / macOS 行为完全一致。
 *
 * ── 卡顿 / 闪屏根因及修复 ───────────────────────────────────────────────────
 *  macOS Core Animation 的 orderFront: 会在 showEvent 触发之前就将窗口提交到
 *  compositor。若此时 Qt backing store 里存的是真实内容，用户就会看到一帧真实
 *  对话框再消失（即"开场闪烁"）。
 *
 *  修复：在 exec() / open() 调用时（widget 仍为 hidden 状态），用 render() 把
 *  真实内容预先渲染到 m_snapshot，并将 m_isAnimating 置为 true。showEvent 到来
 *  时 backing store 的第一帧直接就是动画初始态（透明），compositor 永远看不到
 *  真实内容。
 *
 *  关闭闪烁：done() 里在 hide 子控件之前先同步 repaint()，确保 compositor 下一
 *  帧拿到的是快照帧，而不是子控件消失后的空白背景帧。
 */
class Dialog : public QDialog, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(double animationProgress READ animationProgress WRITE setAnimationProgress)
    Q_PROPERTY(bool dragEnabled READ isDragEnabled WRITE setDragEnabled)
    Q_PROPERTY(bool animationEnabled READ isAnimationEnabled WRITE setAnimationEnabled)
public:
    explicit Dialog(QWidget *parent = nullptr);

    void onThemeUpdated() override { update(); }

    int  shadowSize() const { return m_shadowSize; }

    void setDragEnabled(bool e)      { m_dragEnabled = e; }
    bool isDragEnabled()  const      { return m_dragEnabled; }

    void setAnimationEnabled(bool e) { m_animationEnabled = e; }
    bool isAnimationEnabled() const  { return m_animationEnabled; }

    double animationProgress() const { return m_animationProgress; }
    void   setAnimationProgress(double p);

    // 重写以在显示前预渲染快照
    void open() override;
    int  exec() override;

    void done(int r) override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event)   override;

    void mousePressEvent(QMouseEvent* event)   override;
    void mouseMoveEvent(QMouseEvent* event)    override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void drawShadow(QPainter& painter, const QRect& contentRect);

private:
    /**
     * @brief 在 widget 仍为 hidden 状态时预渲染快照并进入动画模式。
     *        由 open() / exec() 调用，确保第一个 paintEvent 就已是动画帧。
     */
    void prepareAnimation();

    /** @brief 渲染当前真实内容到像素图（忽略 m_isAnimating 状态）。 */
    QPixmap renderSnapshot() const;

    const int m_shadowSize = ::Spacing::Standard;

    bool   m_dragEnabled      = true;
    QPoint m_dragPosition;

    bool   m_animationEnabled  = true;
    bool   m_isAnimating       = false;
    bool   m_isClosing         = false;
    double m_animationProgress = 1.0;
    int    m_closingResult     = 0;

    QPropertyAnimation* m_animation;
    QPixmap             m_snapshot;
};

} // namespace view::dialogs_flyouts

#endif // DIALOG_H
