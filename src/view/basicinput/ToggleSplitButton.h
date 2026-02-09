#ifndef TOGGLESPLITBUTTON_H
#define TOGGLESPLITBUTTON_H

#include "SplitButton.h"

namespace view::basicinput {

/**
 * @brief ToggleSplitButton - 具有切换状态的拆分按钮
 * 
 * 左侧区域具有切换（Toggle）状态，点击时切换选中/未选中。
 * 右侧区域依然为下拉菜单。
 */
class ToggleSplitButton : public SplitButton {
    Q_OBJECT
public:
    explicit ToggleSplitButton(const QString& text = "", QWidget* parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
};

} // namespace view::basicinput

#endif // TOGGLESPLITBUTTON_H
