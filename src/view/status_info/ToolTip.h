#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QWidget>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::textfields { class TextBlock; }

namespace view::status_info {

/**
 * @brief ToolTip - Fluent Design ToolTip
 * Reference: WinUI 3 Gallery
 * 
 * Shows information about an element.
 */
class ToolTip : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(QMargins margins READ margins WRITE setMargins NOTIFY marginsChanged)
public:
    explicit ToolTip(QWidget* parent = nullptr);
    
    void setText(const QString& text);
    QString text() const;

    QMargins margins() const { return m_margins; }
    void setMargins(const QMargins& margins);

    void setFont(const QFont& font); // Shadow QWidget::setFont to apply to label

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
