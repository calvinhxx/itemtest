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

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawShadow(QPainter& painter, const QRect& contentRect);
    
    const int m_shadowSize = ::Spacing::Small; // 使用全局命名空间，避免与 FluentElement::Spacing 冲突
};

} // namespace view::dialogs_flyouts

#endif // DIALOG_H
