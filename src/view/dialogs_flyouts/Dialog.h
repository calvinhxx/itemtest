#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include "view/FluentElement.h"
#include "common/Spacing.h"

namespace view::dialogs_flyouts {

/**
 * @brief Dialog - 实现独立阴影逻辑的对话框
 * 
 * 不再依赖外部 Wrapper，直接在内部通过 paintEvent 绘制阴影和圆角背景。
 */
class Dialog : public QDialog, public FluentElement {
    Q_OBJECT
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

protected:
    void paintEvent(QPaintEvent* event) override;
    
    // 鼠标拖拽支持
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawShadow(QPainter& painter, const QRect& contentRect);
    
    const int m_shadowSize = ::Spacing::Small; // 使用全局命名空间，避免与 FluentElement::Spacing 冲突

    bool m_dragEnabled = true;
    QPoint m_dragPosition;
};

} // namespace view::dialogs_flyouts

#endif // DIALOG_H
