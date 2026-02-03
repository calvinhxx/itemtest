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
 * 采用高性能的位移+淡入动画，放弃昂贵的位图快照，确保在低配电脑上也能达到 60fps。
 */
class Dialog : public QDialog, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(double animationProgress READ animationProgress WRITE setAnimationProgress)
public:
    explicit Dialog(QWidget *parent = nullptr);
    
    void onThemeUpdated() override { update(); }

    /**
     * @brief 获取内容区阴影预留空间（固定 24px 以支持现代软阴影）
     */
    int shadowSize() const { return m_shadowSize; }

    void setDragEnabled(bool enabled) { m_dragEnabled = enabled; }
    bool isDragEnabled() const { return m_dragEnabled; }

    void setAnimationEnabled(bool enabled) { m_animationEnabled = enabled; }
    bool isAnimationEnabled() const { return m_animationEnabled; }

    double animationProgress() const { return m_animationProgress; }
    void setAnimationProgress(double progress);

    void done(int r) override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawShadow(QPainter& painter, const QRect& contentRect);
    
    // 现代审美：使用更宽阔的阴影预留空间 (24px)
    const int m_shadowSize = ::Spacing::Large; 

    bool m_dragEnabled = true;
    QPoint m_dragPosition;

    // 动画相关
    bool m_animationEnabled = true;
    double m_animationProgress = 0.0;
    QPropertyAnimation* m_animation;
    int m_closingResult = 0;

    // 高性能快照逻辑
    bool m_isAnimating = false;
    QPixmap m_snapshot;
};

} // namespace view::dialogs_flyouts

#endif // DIALOG_H
