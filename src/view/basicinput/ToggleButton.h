#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include "Button.h"

namespace view::basicinput {

/**
 * @brief ToggleButton - 具有切换状态的按钮
 * 
 * 行为类似于 CheckBox，外观类似于 Button。
 * 处于 Checked 状态时，默认会表现出类似 Accent 风格的视觉效果。
 */
class ToggleButton : public Button {
    Q_OBJECT
    /** @brief 是否支持三态（选中、未选中、中间态） */
    Q_PROPERTY(bool threeState READ isThreeState WRITE setThreeState NOTIFY threeStateChanged)
    /** @brief 当前状态（针对三态支持） */
    Q_PROPERTY(Qt::CheckState checkState READ checkState WRITE setCheckState NOTIFY checkStateChanged)

public:
    explicit ToggleButton(const QString& text = "", QWidget* parent = nullptr);
    explicit ToggleButton(QWidget* parent = nullptr);

    bool isThreeState() const { return m_threeState; }
    void setThreeState(bool threeState);

    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

    void onThemeUpdated() override;

signals:
    void threeStateChanged();
    void checkStateChanged(Qt::CheckState state);

protected:
    void paintEvent(QPaintEvent* event) override;
    void nextCheckState() override; // 处理三态循环

private:
    bool m_threeState = false;
    Qt::CheckState m_checkState = Qt::Unchecked;
};

} // namespace view::basicinput

#endif // TOGGLEBUTTON_H
