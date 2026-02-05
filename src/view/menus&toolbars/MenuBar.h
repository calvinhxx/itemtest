#ifndef FLUENT_MENUBAR_H
#define FLUENT_MENUBAR_H

#include <QMenuBar>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::menus_toolbars {

/**
 * @brief FluentMenuBar - Fluent UI 风格的菜单栏
 *
 * 参考 WinUI 3 Gallery 中 MenuBar 的设计：
 * - 顶部为一条浅色背景条
 * - 顶层菜单项为纯文本按钮，Hover/Pressed 有 Subtle 背景
 * - 与 FluentElement / QMLPlus 的主题与布局系统联动
 */
class FluentMenuBar : public QMenuBar, public FluentElement, public view::QMLPlus {
    Q_OBJECT
public:
    explicit FluentMenuBar(QWidget* parent = nullptr);

    void onThemeUpdated() override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace view::menus_toolbars

#endif // FLUENT_MENUBAR_H

