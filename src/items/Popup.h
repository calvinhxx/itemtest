#ifndef POPUP_H
#define POPUP_H

#include "Dialog.h"
#include <QPropertyAnimation>
#include <QColor>
#include <QPixmap>

/**
 * @brief Popup - 采用快照渲染方案的圆角弹窗
 * 特点：动画期间使用快照缩放，解决子控件不同步和布局混乱问题。
 */
class Popup : public Dialog {
    Q_OBJECT
    Q_PROPERTY(qreal animationScale READ animationScale WRITE setAnimationScale)
public:
    explicit Popup(QWidget *parent = nullptr);

    void setVisible(bool visible) override;
    void done(int r) override;

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
    void updateSnapshot(); // 捕捉当前界面快照

    int m_cornerRadius = 10;
    int m_shadowWidth = 10;
    int m_borderWidth = 1;
    QColor m_borderColor = QColor(200, 200, 200, 150);
    
    QPoint m_dragPosition;
    QPropertyAnimation *m_animation = nullptr;
    qreal m_animationScale = 0.0;
    int m_resultCode = 0;
    bool m_isFinalizing = false;
    bool m_isAnimating = false; // 是否处于动画渲染模式
    QPixmap m_contentSnapshot;  // 内容快照
};

#endif // POPUP_H
