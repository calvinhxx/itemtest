#ifndef POPUP_H
#define POPUP_H

#include "items/interface/ResponsiveDialog.h"

#include <QPoint>
#include <QColor>

/**
 * @brief Popup - 一个没有系统默认标题栏的圆角弹窗。
 * 派生自 ResponsiveDialog，继承了响应式标题和可见性绑定。
 */
class Popup : public ResponsiveDialog {
    Q_OBJECT
public:
    explicit Popup(QWidget *parent = nullptr);

    // 设置圆角半径
    void setCornerRadius(int radius);
    int cornerRadius() const { return m_cornerRadius; }

    // 设置边框颜色
    void setBorderColor(const QColor &color);
    // 设置阴影宽度
    void setShadowWidth(int width);

protected:
    void paintEvent(QPaintEvent *event) override;
    
    // 鼠标拖拽支持
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int m_cornerRadius = 10;
    int m_shadowWidth = 10;
    int m_borderWidth = 1;
    QColor m_borderColor = QColor(200, 200, 200, 150);
    QPoint m_dragPosition; // 记录鼠标拖拽的相对位置
};

#endif // POPUP_H

