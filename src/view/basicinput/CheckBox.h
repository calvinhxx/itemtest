#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QCheckBox>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QPropertyAnimation;

namespace view::basicinput {

/**
 * @brief CheckBox - WinUI 3 风格的复选框
 * 
 * 支持三态、Fluent 设计 Token 以及精致的自绘视觉效果。
 */
class CheckBox : public QCheckBox, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    /** @brief 复选框状态动画进度 [0,1] */
    Q_PROPERTY(qreal checkProgress READ checkProgress WRITE setCheckProgress)
    /** @brief 复选框方框的大小，默认 20px */
    Q_PROPERTY(int boxSize READ boxSize WRITE setBoxSize NOTIFY boxSizeChanged)
    /** @brief 方框左侧间距，默认 8px */
    Q_PROPERTY(int boxMargin READ boxMargin WRITE setBoxMargin NOTIFY boxMarginChanged)
    /** @brief 方框与文字之间的间距，默认 8px */
    Q_PROPERTY(int textGap READ textGap WRITE setTextGap NOTIFY textGapChanged)
    /** @brief 是否启用悬停背景，默认 false */
    Q_PROPERTY(bool hoverBackgroundEnabled READ hoverBackgroundEnabled WRITE setHoverBackgroundEnabled NOTIFY hoverBackgroundEnabledChanged)

public:
    explicit CheckBox(const QString& text = "", QWidget* parent = nullptr);
    explicit CheckBox(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    qreal checkProgress() const { return m_checkProgress; }
    void setCheckProgress(qreal progress);

    int boxSize() const { return m_boxSize; }
    void setBoxSize(int size);

    int boxMargin() const { return m_boxMargin; }
    void setBoxMargin(int margin);

    int textGap() const { return m_textGap; }
    void setTextGap(int gap);

    bool hoverBackgroundEnabled() const { return m_hoverBackgroundEnabled; }
    void setHoverBackgroundEnabled(bool enabled);

    // 提供给布局系统的尺寸提示
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void boxSizeChanged();
    void boxMarginChanged();
    void textGapChanged();
    void hoverBackgroundEnabledChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void nextCheckState() override;

private:
    void initAnimation();

    qreal m_checkProgress = 1.0; 
    int m_boxSize = 20; // 默认 20px
    int m_boxMargin = 8; // 默认 8px
    int m_textGap = 8; // 默认 8px
    bool m_hoverBackgroundEnabled = false; // 默认不启用 hover 背景
    QPropertyAnimation* m_checkAnimation = nullptr;
};

} // namespace view::basicinput

#endif // CHECKBOX_H
