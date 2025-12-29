#ifndef POPUP_H
#define POPUP_H

#include "items/interface/ResponsiveDialog.h"
#include <QPoint>
#include <QColor>
#include <QPropertyAnimation>

/**
 * @brief Popup - 一个没有系统默认标题栏的圆角弹窗。
 */
class Popup : public ResponsiveDialog {
    Q_OBJECT
    Q_PROPERTY(qreal animationScale READ animationScale WRITE setAnimationScale)
public:
    explicit Popup(QWidget *parent = nullptr);

    // 标准 API 重写
    void setVisible(bool visible) override;
    // 动画属性访问器
    qreal animationScale() const { return m_animationScale; }
    void setAnimationScale(qreal s) { m_animationScale = s; update(); }

    void setCornerRadius(int radius) { m_cornerRadius = radius; update(); }
    void setBorderColor(const QColor &color) { m_borderColor = color; update(); }
    void setShadowWidth(int width) { m_shadowWidth = width; setContentsMargins(width, width, width, width); update(); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    int m_cornerRadius = 10;
    int m_shadowWidth = 10;
    int m_borderWidth = 1;
    QColor m_borderColor = QColor(200, 200, 200, 150);
    QPoint m_dragPosition; 
    
    qreal m_animationScale = 0.0;
    QPropertyAnimation *m_showAnimation = nullptr;
    QPropertyAnimation *m_closeAnimation = nullptr;
};

#endif // POPUP_H

