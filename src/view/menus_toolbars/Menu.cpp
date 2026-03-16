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
    setFont(themeFont(m_fontStyle).toQFont());
}

void FluentMenuItem::setFontStyle(const QString& style) {
    if (m_fontStyle == style) return;
    m_fontStyle = style;
    onThemeUpdated();
    emit fontStyleChanged();
}

void FluentMenuItem::onThemeUpdated() {
    setFont(themeFont(m_fontStyle).toQFont());
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

    setFont(themeFont(m_fontStyle).toQFont());
    onThemeUpdated();
}

void FluentMenu::setFontStyle(const QString& style) {
    if (m_fontStyle == style) return;
    m_fontStyle = style;
    onThemeUpdated();
    emit fontStyleChanged();
}

void FluentMenu::onThemeUpdated() {
    const auto& s = themeSpacing();
    int vPadding = s.gap.tight; // 4px

    // 同步菜单字体（影响 QMenu 内部的 actionGeometry 高度计算）
    setFont(themeFont(m_fontStyle).toQFont());

    // 使用 margins 为阴影和内部 padding 预留空间
    // 这样 QMenu 的 sizeHint 会自动包含这些边距，确保窗口足够大
    setContentsMargins(m_shadowSize, m_shadowSize + vPadding, m_shadowSize, m_shadowSize + vPadding);

    // 隐藏系统默认边框/背景；QMenu::separator height 控制分割线的垂直占位，
    // 使 actionGeometry() 分配足够空间，paintEvent 中线画在中央（上下各 gap.normal/2 ≈ 4px）。
    const int separatorH = s.gap.normal + 1; // 8px 间距 + 1px 线 = 9px，上下各约 4px
    setStyleSheet(QStringLiteral(
        "QMenu { background-color: transparent; border: 0px; }"
        "QMenu::separator { height: %1px; }"
    ).arg(separatorH));
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

    // 2. 计算 items 垂直范围
    QRect itemsRect;
    for (QAction* action : actions()) {
        if (action->isVisible()) {
            itemsRect |= actionGeometry(action);
        }
    }

    if (itemsRect.isEmpty()) return;

    // 底板矩形：水平方向始终使用 m_shadowSize 作为边界（不依赖 actionGeometry 的 x 值），
    // 垂直方向在 items 范围上下各扩充 vPadding
    QRect contentRect(
        m_shadowSize,
        itemsRect.top() - vPadding,
        width() - 2 * m_shadowSize,
        itemsRect.height() + 2 * vPadding
    );

    // 3. 绘制多层软阴影
    drawShadow(p, contentRect);

    // 4. 绘制圆角背景与边框
    int r = radius.overlay;
    p.save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(contentRect, r, r);
    p.setClipPath(clipPath);

    p.setPen(colors.strokeCard);
    p.setBrush(colors.bgLayer);
    p.drawRoundedRect(contentRect, r, r);

    // 5. 绘制菜单项
    // bgMargin: 高亮背景距底板边缘的水平缩进（4px，与底板圆角视觉对齐）
    // textPadding: 文字距底板边缘的水平内边距（12px，ControlHorizontal）
    const int plateLeft    = contentRect.left();
    const int plateWidth   = contentRect.width();
    const int bgMargin     = spacing.gap.tight;          // 4
    const int textPadding  = spacing.padding.controlH;   // 12

    // 明确设置绘制字体，防止 shadow 循环中字体被污染
    p.setFont(font());

    for (QAction* action : actions()) {
        if (!action->isVisible()) continue;

        QRect itemRect = actionGeometry(action);
        // 规范化水平范围：统一对齐到底板边界（actionGeometry 可能不含 shadow margin）
        itemRect.setLeft(plateLeft);
        itemRect.setWidth(plateWidth);

        if (!contentRect.intersects(itemRect)) continue;

        if (action->isSeparator()) {
            p.setPen(colors.strokeDivider);
            int y = itemRect.center().y();
            p.drawLine(itemRect.left() + bgMargin, y, itemRect.right() - bgMargin, y);
            continue;
        }

        bool isEnabled = action->isEnabled();
        bool isActive  = (action == activeAction());

        QColor bg = Qt::transparent;
        if (isEnabled) {
            if (action->isChecked()) bg = colors.subtleTertiary;
            else if (isActive)       bg = colors.subtleSecondary;
        }

        if (bg != Qt::transparent) {
            QRectF bgRect = itemRect.adjusted(bgMargin, 1, -bgMargin, -1);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(bgRect, radius.control, radius.control);
        }

        p.setPen(isEnabled ? colors.textPrimary : colors.textDisabled);
        QRect textRect = itemRect.adjusted(textPadding, 0, -textPadding, 0);
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
    int r = themeRadius().overlay;

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

