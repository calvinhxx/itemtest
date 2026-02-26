#ifndef SPLITBUTTON_H
#define SPLITBUTTON_H

#include "Button.h"
#include <QMenu>

namespace view::basicinput {

/**
 * @brief SplitButton - 具有独立主操作区和下拉区的拆分按钮
 * 
 * 左侧区域点击触发 clicked() 信号，右侧区域点击弹出菜单。
 */
class SplitButton : public Button {
    Q_OBJECT
    /** @brief 绑定的下拉菜单 */
    Q_PROPERTY(QMenu* menu READ menu WRITE setMenu NOTIFY menuChanged)
    /** @brief 右侧下拉区域的宽度，默认 32px */
    Q_PROPERTY(int secondaryWidth READ secondaryWidth WRITE setSecondaryWidth NOTIFY secondaryWidthChanged)

public:
    enum SplitPart { None, Primary, Secondary };

    explicit SplitButton(const QString& text = "", QWidget* parent = nullptr);
    
    QMenu* menu() const { return m_menu; }
    void setMenu(QMenu* menu);

    int secondaryWidth() const { return m_secondaryWidth; }
    void setSecondaryWidth(int width);

signals:
    void menuChanged();
    void secondaryWidthChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    SplitPart getPartAt(const QPoint& pos) const;
    void updateSplitState(SplitPart hoverPart, SplitPart pressPart);

    QMenu* m_menu = nullptr;
    SplitPart m_hoverPart = None;
    SplitPart m_pressPart = None;
    
    int m_secondaryWidth = 32;

private:
};

} // namespace view::basicinput

#endif // SPLITBUTTON_H
