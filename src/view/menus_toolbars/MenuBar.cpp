#include "MenuBar.h"

#include <QPainter>
#include <QStyleOptionMenuItem>
#include "common/CornerRadius.h"

namespace view::menus_toolbars {

FluentMenuBar::FluentMenuBar(QWidget* parent)
    : QMenuBar(parent) {
    setAttribute(Qt::WA_Hover);
#ifdef Q_OS_MAC
    // 在 macOS 上默认会使用系统全局菜单栏，需要关闭以便在窗口内显示
    setNativeMenuBar(false);
#endif
    // 为了获得更一致的高度，使用 Body 字体
    auto fs = themeFont("Body");
    setFont(fs.toQFont());
    onThemeUpdated();
}

void FluentMenuBar::onThemeUpdated() {
    const auto& c = themeColors();
    // 仅设置背景，文字颜色在 paintEvent 中控制
    setStyleSheet(QString("QMenuBar { background-color: %1; }")
                      .arg(c.bgCanvas.name()));
    update();
}

void FluentMenuBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    const auto& colors = themeColors();
    const auto& spacing = themeSpacing();

    // 填充整个菜单条背景
    p.fillRect(rect(), colors.bgCanvas);

    // 顶层菜单项样式：Subtle 背景 + 文本色
    QFontMetrics fm(font());
    int hPadding = spacing.small;

    for (QAction* action : actions()) {
        if (!action->isVisible()) continue;

        QRect itemRect = actionGeometry(action);
        if (!rect().intersects(itemRect)) continue;

        bool isEnabled = action->isEnabled();
        bool isActive = (action == activeAction());
        bool isMenuOpen = action->menu() && action->menu()->isVisible();

        // 1. 背景：Hover / Pressed(Open) 使用 Subtle 填充
        QColor bg = Qt::transparent;
        if (!isEnabled) {
            bg = Qt::transparent;
        } else if (isMenuOpen) {
            bg = colors.subtleTertiary; // 按下 / 打开
        } else if (isActive) {
            bg = colors.subtleSecondary; // Hover
        }

        if (bg != Qt::transparent) {
            QRectF bgRect = itemRect.adjusted(0, 0, 0, 0);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            // 使用设计 Token，而不是 magic number
            p.drawRoundedRect(bgRect, CornerRadius::Control, CornerRadius::Control);
        }

        // 2. 文本
        QString text = action->text();
        QColor textColor = isEnabled ? colors.textPrimary : colors.textDisabled;
        p.setPen(textColor);

        // 只在左侧留内边距，避免有效宽度过小导致文本被裁剪
        QRect textRect = itemRect.adjusted(hPadding, 0, 0, 0);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
    }
}

} // namespace view::menus_toolbars

