#ifndef FLUENT_MENU_H
#define FLUENT_MENU_H

#include <QMenu>
#include <QWidgetAction>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"

namespace view::menus_toolbars {

/**
 * @brief FluentMenuItem - Fluent UI 风格的菜单项
 *
 * - 继承 QWidgetAction + FluentElement + QMLPlus：
 *   - 可以访问 Fluent Design Token（字体、颜色等）
 *   - 支持通过 QMLPlus 参与 Anchors/States 语义绑定
 * - 当前视觉仍主要由 FluentMenu/FluentMenuBar 统一绘制，但 Action 自身的字体会使用 Fluent 字体，
 *   这样在未自绘菜单或其他容器中使用时，文本也能保持 Fluent 风格。
 */
class FluentMenuItem : public QWidgetAction, public FluentElement, public view::QMLPlus {
    Q_OBJECT
public:
    explicit FluentMenuItem(const QString& text, QObject* parent = nullptr);

    void onThemeUpdated() override;
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
    void showEvent(QShowEvent* event) override;

private:
    void drawShadow(QPainter& painter, const QRect& contentRect);

    // 与 drawShadow 扩散范围一致，略留余量自然淡出
    const int m_shadowSize = ::Spacing::Standard;
};

} // namespace view::menus_toolbars

#endif // FLUENT_MENU_H

