#include "Menu.h"

#include <QPainter>
#include <QPainterPath>
#include <QShowEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>

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
    const auto& s = themeSpacing();
    int vPadding = s.gap.tight; // 4px

    // 使用 margins 为阴影和内部 padding 预留空间
    // 这样 QMenu 的 sizeHint 会自动包含这些边距，确保窗口足够大
    setContentsMargins(m_shadowSize, m_shadowSize + vPadding, m_shadowSize, m_shadowSize + vPadding);

    // 仅确保系统样式不再绘制默认边框/背景
    setStyleSheet(QStringLiteral("QMenu { background-color: transparent; border: 0px; }"));
    update();
}

void FluentMenu::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // 1. 透明清屏
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(rect(), Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    const auto& colors = themeColors();
    const auto& spacing = themeSpacing();
    const auto& radius = themeRadius();
    int vPadding = spacing.gap.tight;

    // 2. 计算动态内容区域 (Content Plate)
    // 找出所有可见条目的最小外接矩形
    QRect itemsRect;
    for (QAction* action : actions()) {
        if (action->isVisible()) {
            itemsRect |= actionGeometry(action);
        }
    }
    
    if (itemsRect.isEmpty()) return;

    // 底板矩形：在 Items 范围基础上，上下各扩充 vPadding 的空间
    // 这就是我们要画的白色圆角框区域
    QRect contentRect = itemsRect.adjusted(0, -vPadding, 0, vPadding);

    // 3. 绘制多层软阴影
    drawShadow(p, contentRect);

    // 4. 绘制圆角背景与边框
    int r = radius.topLevel;
    p.save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(contentRect, r, r);
    p.setClipPath(clipPath);

    p.setPen(colors.strokeCard);
    p.setBrush(colors.bgLayer);
    p.drawRoundedRect(contentRect, r, r);

    // 5. 绘制菜单项
    int itemMargin = spacing.gap.tight; 
    int textHPadding = spacing.padding.controlH;

    for (QAction* action : actions()) {
        if (!action->isVisible()) continue;

        QRect itemRect = actionGeometry(action);
        // 如果条目超出了当前的绘制裁剪区则跳过
        if (!contentRect.intersects(itemRect)) continue;

        if (action->isSeparator()) {
            p.setPen(colors.strokeDivider);
            int y = itemRect.center().y();
            p.drawLine(itemRect.left() + itemMargin, y, itemRect.right() - itemMargin, y);
            continue;
        }

        bool isEnabled = action->isEnabled();
        bool isActive = (action == activeAction());

        QColor bg = Qt::transparent;
        if (isEnabled) {
            if (action->isChecked()) bg = colors.subtleTertiary;
            else if (isActive) bg = colors.subtleSecondary;
        }

        if (bg != Qt::transparent) {
            QRectF bgRect = itemRect.adjusted(itemMargin, 0, -itemMargin, 0);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(bgRect, themeRadius().control, themeRadius().control);
        }

        p.setPen(isEnabled ? colors.textPrimary : colors.textDisabled);
        QRect textRect = itemRect.adjusted(textHPadding, 0, -textHPadding, 0);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, action->text());
    }
    p.restore();
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
    posAnim->setDuration(themeAnimation().normal);
    posAnim->setStartValue(startPos);
    posAnim->setEndValue(targetPos);
    // 使用 Entrance 类型 (OutBack)，提供轻微的回弹弹性感
    posAnim->setEasingCurve(themeAnimation().entrance);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);
    
    // 4. 启动透明度动画
    QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "windowOpacity");
    opacityAnim->setDuration(themeAnimation().normal);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    // 透明度建议使用标准的减速曲线，不建议带回弹（避免闪烁感）
    opacityAnim->setEasingCurve(themeAnimation().decelerate);
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

