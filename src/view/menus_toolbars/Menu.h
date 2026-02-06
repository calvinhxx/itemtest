#ifndef FLUENT_MENU_H
#define FLUENT_MENU_H

#include <QMenu>
#include <QWidgetAction>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"

namespace view::menus_toolbars {

/**
 * @brief FluentMenuItem - Fluent UI 风格的菜单项（继承 QWidgetAction）
 *
 * 当前主要作为语义占位，样式由 FluentMenu 统一绘制。
 */
class FluentMenuItem : public QWidgetAction {
public:
    explicit FluentMenuItem(const QString& text, QObject* parent = nullptr);
};

/**
 * @brief FluentMenu - Fluent UI 风格的下拉菜单
 *
 * 使用 FluentElement 设计 Token 自绘菜单背景和条目，并通过 QMLPlus 接入 Anchors/States。
 */
class FluentMenu : public QMenu, public FluentElement, public view::QMLPlus {
    Q_OBJECT
public:
    explicit FluentMenu(const QString& title, QWidget* parent = nullptr);

    void onThemeUpdated() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawShadow(QPainter& painter, const QRect& contentRect);

    // 顶层菜单同 Dialog，预留阴影空间（24px）
    const int m_shadowSize = ::Spacing::Large;
};

} // namespace view::menus_toolbars

#endif // FLUENT_MENU_H

