#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QPixmap>
#include "view/FluentElement.h"
#include "common/Spacing.h"

namespace view::dialogs_flyouts {

/**
 * @brief Dialog - 实现独立阴影逻辑且支持同步缩放动画的对话框
 */
class Dialog : public QDialog, public FluentElement {
    Q_OBJECT
    Q_PROPERTY(double animationProgress READ animationProgress WRITE setAnimationProgress)
public:
    explicit Dialog(QWidget *parent = nullptr);
    
    void onThemeUpdated() override { update(); }

    /**
     * @brief 获取阴影预留的大小
     */
    int shadowSize() const { return m_shadowSize; }

    /**
     * @brief 设置/获取是否允许鼠标拖拽移动
     */
    void setDragEnabled(bool enabled) { m_dragEnabled = enabled; }
    bool isDragEnabled() const { return m_dragEnabled; }

    /**
     * @brief 设置/获取是否启用弹窗动画
     */
    void setAnimationEnabled(bool enabled) { m_animationEnabled = enabled; }
    bool isAnimationEnabled() const { return m_animationEnabled; }

    // 动画属性访问
    double animationProgress() const { return m_animationProgress; }
    void setAnimationProgress(double progress);

    // 拦截 done 以播放退出动画
    void done(int r) override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    
    // 鼠标拖拽支持
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawShadow(QPainter& painter, const QRect& contentRect);
    
    const int m_shadowSize = ::Spacing::Small;

    bool m_dragEnabled = true;
    QPoint m_dragPosition;

    // 动画相关
    bool m_animationEnabled = true;
    double m_animationProgress = 0.0;
    QPropertyAnimation* m_animation;
    int m_closingResult = 0;
    
    // 快照逻辑相关 (用于内容同步缩放)
    bool m_isAnimating = false;
    QPixmap m_snapshot;
};

} // namespace view::dialogs_flyouts

#endif // DIALOG_H
