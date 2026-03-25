#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include <QRadioButton>
#include <QFont>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QPropertyAnimation;

namespace view::basicinput {

/**
 * @brief RadioButton - WinUI 3 风格的单选按钮
 *
 * 支持 Fluent Design Token 自绘，圆形指示器 + 选中态 Accent 填充 + 内圆点动画。
 * text 由 QRadioButton 基类管理，textFont 可独立配置（默认 Body 字体）。
 */
class RadioButton : public QRadioButton, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    /** @brief 选中动画进度 [0,1] */
    Q_PROPERTY(qreal checkProgress READ checkProgress WRITE setCheckProgress)
    /** @brief 内圆点缩放比例，hover 动画驱动 */
    Q_PROPERTY(qreal dotScale READ dotScale WRITE setDotScale)
    /** @brief 外圈直径，默认 20px */
    Q_PROPERTY(int circleSize READ circleSize WRITE setCircleSize NOTIFY circleSizeChanged)
    /** @brief 圆形与文字之间的间距，默认 8px */
    Q_PROPERTY(int textGap READ textGap WRITE setTextGap NOTIFY textGapChanged)
    /** @brief 文字字体，默认使用 Typography::Body */
    Q_PROPERTY(QFont textFont READ textFont WRITE setTextFont NOTIFY textFontChanged)

public:
    explicit RadioButton(const QString& text = "", QWidget* parent = nullptr);
    explicit RadioButton(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    qreal checkProgress() const { return m_checkProgress; }
    void setCheckProgress(qreal progress);

    qreal dotScale() const { return m_dotScale; }
    void setDotScale(qreal scale);

    int circleSize() const { return m_circleSize; }
    void setCircleSize(int size);

    int textGap() const { return m_textGap; }
    void setTextGap(int gap);

    QFont textFont() const { return m_textFont; }
    void setTextFont(const QFont& font);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void circleSizeChanged();
    void textGapChanged();
    void textFontChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void nextCheckState() override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void initAnimation();

    qreal m_checkProgress = 1.0;
    qreal m_dotScale = 1.0;
    int m_circleSize = 20;
    int m_textGap = 8;
    QFont m_textFont;
    QPropertyAnimation* m_checkAnimation = nullptr;
    QPropertyAnimation* m_dotScaleAnimation = nullptr;
};

} // namespace view::basicinput

#endif // RADIOBUTTON_H
