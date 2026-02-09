#include "Menu.h"

#include <QPainter>
#include <QPainterPath>
#include <QShowEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include "common/CornerRadius.h"
#include "common/Animation.h"

namespace view::menus_toolbars {

// =============================== FluentMenuItem ===============================

FluentMenuItem::FluentMenuItem(const QString& text, QObject* parent)
    : QWidgetAction(parent) {
    setText(text);

    // 默认使用 Fluent Body 字体，保证在未自绘菜单中也具备 Fluent 文本风格
    auto fs = themeFont("Body");
    setFont(fs.toQFont());
}

void FluentMenuItem::onThemeUpdated() {
    // 当主题切换时，同步更新 Action 的字体（例如 Light/Dark 可能切换字体族）
    auto fs = themeFont("Body");
    setFont(fs.toQFont());
}

// ================================ FluentMenu =================================

FluentMenu::FluentMenu(const QString& title, QWidget* parent)
    : QMenu(title, parent) {
    // 顶层无边框 + 禁用系统阴影，由自身绘制阴影与圆角
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_Hover);
    setAutoFillBackground(false);
    setContentsMargins(m_shadowSize, m_shadowSize, m_shadowSize, m_shadowSize);

    auto fs = themeFont("Body");
    setFont(fs.toQFont());
    onThemeUpdated();
}

void FluentMenu::onThemeUpdated() {
    const auto& c = themeColors();
    Q_UNUSED(c);

    // 使用自绘背景与边框，这里仅确保系统样式不再绘制默认边框/背景
    setStyleSheet(QStringLiteral("QMenu { background-color: transparent; border: 0px; }"));
    update();
}

void FluentMenu::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // 1. 透明清屏（支持圆角与阴影）
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(rect(), Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    const auto& colors = themeColors();
    const auto& spacing = themeSpacing();
    const auto& radius = themeRadius();

    // 内容区域：预留阴影空间
    QRect contentRect = rect().adjusted(m_shadowSize, m_shadowSize, -m_shadowSize, -m_shadowSize);

    // 2. 绘制多层软阴影（与 Dialog 一致的高层阴影）
    drawShadow(p, contentRect);

    // 3. 圆角背景与边框
    int r = radius.topLevel;
    QPainterPath clipPath;
    clipPath.addRoundedRect(contentRect, r, r);
    p.setClipPath(clipPath);

    p.setPen(colors.strokeCard);
    p.setBrush(colors.bgLayer);
    p.drawRoundedRect(contentRect, r, r);

    // 4. 绘制菜单项（保持现有 Fluent 风格，只是在内容区域内绘制）
    QFontMetrics fm(font());
    int hPadding = spacing.small;
    int vPadding = spacing.gap.tight;

    for (QAction* action : actions()) {
        if (!action->isVisible()) continue;

        QRect itemRect = actionGeometry(action);
        if (!contentRect.intersects(itemRect)) continue;

        if (action->isSeparator()) {
            // 绘制分割线
            p.setPen(colors.strokeDivider);
            int y = itemRect.center().y();
            p.drawLine(itemRect.left() + spacing.padding.controlH,
                       y,
                       itemRect.right() - spacing.padding.controlH,
                       y);
            continue;
        }

        bool isEnabled = action->isEnabled();
        bool isActive = (action == activeAction());

        // 背景：Hover 使用 SubtleSecondary，选中/勾选使用 SubtleTertiary
        QColor bg = Qt::transparent;
        if (!isEnabled) {
            bg = Qt::transparent;
        } else if (action->isChecked()) {
            bg = colors.subtleTertiary;
        } else if (isActive) {
            bg = colors.subtleSecondary;
        }

        if (bg != Qt::transparent) {
            // 与 MenuBar 一致：直接使用 CornerRadius 控件圆角规范
            QRectF bgRect = itemRect.adjusted(0, 0, 0, 0);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(bgRect, CornerRadius::Control, CornerRadius::Control);
        }

        // 文本颜色
        QColor textColor = isEnabled ? colors.textPrimary : colors.textDisabled;
        p.setPen(textColor);

        // 当前主要展示文本，后续可扩展 Icon / Shortcut 等
        QRect textRect = itemRect.adjusted(hPadding, vPadding, -hPadding, -vPadding);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, action->text());
    }
}

void FluentMenu::showEvent(QShowEvent* event) {
    QMenu::showEvent(event);
    
    // 1. 计算目标位置
    // 因为菜单有 m_shadowSize 的 margin，需要向左和向上偏移阴影大小，
    // 让内容区域（不包括阴影）紧贴触发控件
    const auto& spacing = themeSpacing();
    QPoint targetPos = pos();
    
    // 调整位置：抵消阴影带来的 margin
    // Y 轴多保留 spacing.gap.tight 的距离，避免阴影紧贴按钮边缘
    targetPos.rx() -= m_shadowSize;
    targetPos.ry() -= (m_shadowSize - spacing.gap.tight);
    
    // 2. 设置动画初始状态：向上偏移 20px，透明度 0
    int slideOffset = 20;
    QPoint startPos = targetPos - QPoint(0, slideOffset);
    move(startPos);
    setWindowOpacity(0.0);
    
    // 3. 启动平移动画
    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(::Animation::Duration::Normal);
    posAnim->setStartValue(startPos);
    posAnim->setEndValue(targetPos);
    // 使用 Entrance 类型 (OutBack)，提供轻微的回弹弹性感
    posAnim->setEasingCurve(::Animation::getEasing(::Animation::EasingType::Entrance));
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);
    
    // 4. 启动透明度动画
    QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "windowOpacity");
    opacityAnim->setDuration(::Animation::Duration::Normal);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    // 透明度建议使用标准的减速曲线，不建议带回弹（避免闪烁感）
    opacityAnim->setEasingCurve(::Animation::getEasing(::Animation::EasingType::Decelerate));
    opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FluentMenu::drawShadow(QPainter& painter, const QRect& contentRect) {
    const auto& s = themeShadow(Elevation::High);
    const int layers = 10;
    const int spreadStep = 1;
    int r = themeRadius().topLevel;

    for (int i = 0; i < layers; ++i) {
        double ratio = 1.0 - static_cast<double>(i) / layers;
        QColor sc = s.color;
        sc.setAlphaF(s.opacity * ratio * 0.35);

        painter.setPen(Qt::NoPen);
        painter.setBrush(sc);

        int spread = i * spreadStep;
        int offsetY = 2;
        painter.drawRoundedRect(
            contentRect.adjusted(-spread, -spread, spread, spread).translated(0, offsetY),
            r + spread, r + spread);
    }
}

} // namespace view::menus_toolbars

