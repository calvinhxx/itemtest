#ifndef REPEATBUTTON_H
#define REPEATBUTTON_H

#include "Button.h"

namespace view::basicinput {

/**
 * @brief RepeatButton - 按住时持续触发点击事件的按钮
 * 
 * 视觉上与标准 Button 一致，但在按下状态下会按设定的频率重复发送 clicked 信号。
 */
class RepeatButton : public Button {
    Q_OBJECT
    /** @brief 首次重复前的延迟时间 (ms)，默认 500ms */
    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    /** @brief 重复触发的间隔时间 (ms)，默认 50ms */
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)

public:
    explicit RepeatButton(const QString& text = "", QWidget* parent = nullptr);
    explicit RepeatButton(QWidget* parent = nullptr);

    int delay() const { return autoRepeatDelay(); }
    void setDelay(int d);

    int interval() const { return autoRepeatInterval(); }
    void setInterval(int i);

signals:
    void delayChanged();
    void intervalChanged();
};

} // namespace view::basicinput

#endif // REPEATBUTTON_H
