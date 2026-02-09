#ifndef FLUENT_MENUBAR_H
#define FLUENT_MENUBAR_H

#include <QMenuBar>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::menus_toolbars {

class FluentMenu;

/**
 * @brief FluentMenuBar - Fluent UI 风格的菜单栏
 */
class FluentMenuBar : public QMenuBar, public FluentElement, public view::QMLPlus {
    Q_OBJECT
public:
    explicit FluentMenuBar(QWidget* parent = nullptr);

    void onThemeUpdated() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
};

} // namespace view::menus_toolbars

#endif // FLUENT_MENUBAR_H
