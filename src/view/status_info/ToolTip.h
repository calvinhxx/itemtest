#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QWidget>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::textfields { class TextBlock; }

namespace view::status_info {

/**
 * @brief ToolTip - Fluent 设计风格的工具提示
 * 
 * 参考 WinUI 3 规范，显示有关元素的简短说明。
 */
class ToolTip : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    /** @brief ToolTip 的内边距 */
    Q_PROPERTY(QMargins margins READ margins WRITE setMargins NOTIFY marginsChanged)
public:
    explicit ToolTip(QWidget* parent = nullptr);
    
    void setText(const QString& text);
    QString text() const;

    QMargins margins() const { return m_margins; }
    void setMargins(const QMargins& margins);

    // 影子 QWidget::setFont 以应用到内部的标签
    void setFont(const QFont& font);

    void onThemeUpdated() override;

signals:
    void marginsChanged();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    view::textfields::TextBlock* m_textBlock;
    QMargins m_margins;

    QColor m_bgColor;
    QColor m_borderColor;
    QColor m_textColor;
};

} // namespace view::status_info

#endif // TOOLTIP_H
