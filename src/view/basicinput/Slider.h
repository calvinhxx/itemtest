#ifndef SLIDER_H
#define SLIDER_H

#include <QSlider>
#include <QPropertyAnimation>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"

class QPainter;
class QStyleOptionSlider;

namespace view::textfields { class TextBlock; }
namespace view::status_info { class ToolTip; }

namespace view::basicinput {

/**
 * @brief Slider - 参考 WinUI 3 Gallery 的 Fluent 风格滑块
 *
 * 仅自绘外观，不改变 QSlider 本身的行为：
 * - 支持水平 / 垂直方向
 * - 轨道前景使用 Accent 颜色，后景使用 Control 颜色
 * - 拇指为圆形，支持禁用 / 悬停 / 按下等状态色
 * - 兼容 QSlider 现有属性（minimum / maximum / singleStep / ticks 等）
 */
class Slider : public QSlider, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(int handleSize READ handleSize WRITE setHandleSize)
    Q_PROPERTY(int trackHeight READ trackHeight WRITE setTrackHeight)
    // 动画属性
    Q_PROPERTY(qreal hoverRatio READ hoverRatio WRITE setHoverRatio)
    Q_PROPERTY(qreal pressRatio READ pressRatio WRITE setPressRatio)

public:
    explicit Slider(Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr);
    explicit Slider(QWidget* parent);
    ~Slider();

    QSize sizeHint() const override;

    void onThemeUpdated() override { update(); }

    int handleSize() const { return m_handleSize; }
    void setHandleSize(int size) { m_handleSize = size; update(); }

    int trackHeight() const { return m_trackHeight; }
    void setTrackHeight(int height) { m_trackHeight = height; update(); }

    qreal hoverRatio() const { return m_hoverRatio; }
    void setHoverRatio(qreal ratio) { m_hoverRatio = ratio; update(); }

    qreal pressRatio() const { return m_pressRatio; }
    void setPressRatio(qreal ratio) { m_pressRatio = ratio; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void drawHorizontal(QPainter& p, const QStyleOptionSlider& opt);
    void drawVertical(QPainter& p, const QStyleOptionSlider& opt);

    int pixelPosToRangeValue(int pos) const;
    int valueToPixelPos(int value) const;
    void showToolTip();
    void updateToolTipPos();
    void hideToolTip();

    int m_handleSize = 20; // WinUI 3 Standard Thumb (5 * BaseUnit)
    int m_trackHeight = ::Spacing::XSmall; // 默认 4px
    int m_visualMargin = 2; // 避免抗锯齿被裁切的边距
    int m_defaultLength = 160; // 默认长度
    bool m_isPressed = false;

    // 动画状态
    qreal m_hoverRatio = 0.0;
    qreal m_pressRatio = 0.0;
    QPropertyAnimation* m_hoverAnim = nullptr;
    QPropertyAnimation* m_pressAnim = nullptr;

    view::status_info::ToolTip* m_toolTip = nullptr;
};

} // namespace view::basicinput

#endif // SLIDER_H

