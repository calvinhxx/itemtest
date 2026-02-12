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
    const auto& s = themeSpacing();
    
    // 关键：通过 QSS 设置 padding 和 margin，让 QMenuBar 内部逻辑计算出正确的点击区域
    // padding 控制文本周围的间距，margin 控制菜单项之间的间距
    setStyleSheet(QString(
        "QMenuBar { background-color: %1; border: none; }"
        "QMenuBar::item { "
        "   background: transparent; "
        "   padding: %2px %3px; "
        "   margin: 0px %4px; "
        "   border-radius: %5px; "
        "}"
    ).arg(c.bgCanvas.name())
     .arg(s.padding.controlV)    // 上下 padding
     .arg(s.padding.controlH)    // 左右 padding
     .arg(s.gap.tight / 2)       // 菜单项之间的间距 (控制整体间距的一半)
     .arg(CornerRadius::Control));

    updateGeometry(); 
    update();
}

void FluentMenuBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    const auto& colors = themeColors();

    // 1. 填充整个菜单条背景
    p.fillRect(rect(), colors.bgCanvas);

    for (QAction* action : actions()) {
        if (!action->isVisible()) continue;

        // 使用 QMenuBar 内部计算好的几何区域，确保绘制位置与点击判定位置完全重合
        QRect itemRect = actionGeometry(action);
        if (itemRect.isEmpty() || !rect().intersects(itemRect)) continue;

        bool isEnabled = action->isEnabled();
        bool isActive = (action == activeAction());
        bool isMenuOpen = action->menu() && action->menu()->isVisible();

        // 2. 绘制背景：Hover / Pressed(Open) 使用 Subtle 填充
        QColor bg = Qt::transparent;
        if (isEnabled) {
            if (isMenuOpen) {
            bg = colors.subtleTertiary; // 按下 / 打开
        } else if (isActive) {
            bg = colors.subtleSecondary; // Hover
            }
        }

        if (bg != Qt::transparent) {
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(itemRect, CornerRadius::Control, CornerRadius::Control);
        }

        // 3. 绘制文本
        QString text = action->text();
        QColor textColor = isEnabled ? colors.textPrimary : colors.textDisabled;
        p.setPen(textColor);
        p.setFont(font());
        
        // 在 itemRect 内居中绘制文本
        p.drawText(itemRect, Qt::AlignCenter, text);
    }
}

QSize FluentMenuBar::sizeHint() const {
    // 基础高度计算：字体高度 + 上下 Padding
    QFontMetrics fm(font());
    const auto& s = themeSpacing();
    int h = fm.height() + s.padding.controlV * 2;
    
    // 宽度由 QMenuBar 默认逻辑根据 item 数量和 padding 自动计算即可
    QSize baseSize = QMenuBar::sizeHint();
    return QSize(baseSize.width(), h);
}

} // namespace view::menus_toolbars
