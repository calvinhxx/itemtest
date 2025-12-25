#ifndef DEBUGOVERLAY_H
#define DEBUGOVERLAY_H

#include <QWidget>
#include <QPointer>
#include <QColor>

/**
 * @brief DebugOverlay - 用于高亮显示指定控件边框的调试组件
 */
class DebugOverlay : public QWidget {
    Q_OBJECT
public:
    // 指定要高亮的控件和颜色
    explicit DebugOverlay(QWidget *target, const QColor &color = Qt::red, QWidget *parent = nullptr);
    
    void setColor(const QColor &c) { m_color = c; update(); }
    void setThickness(int t) { m_thickness = t; update(); }

protected:
    // 核心逻辑：追踪目标控件的变化
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateGeometryToTarget();

    QPointer<QWidget> m_target; // 使用弱引用，防止目标销毁后崩溃
    QColor m_color;
    int m_thickness = 1;
};

#endif // DEBUGOVERLAY_H