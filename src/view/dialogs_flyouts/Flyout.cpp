#include "Flyout.h"

#include <QApplication>
#include "design/Spacing.h"

namespace view::dialogs_flyouts {

// 与 Popup.cpp 内部 kShadowMargin 保持一致（绘制阴影预留的边距）。
// Popup::open() 在使用 m_targetPos 时会减去 (kShadowMargin, kShadowMargin)，
// 但在 computePosition() 路径上不会自动补偿，所以子类返回值已经是含阴影偏移的最终 move 坐标。
static constexpr int kShadowMargin = ::Spacing::Standard;

Flyout::Flyout(QWidget* parent) : Popup(parent) {
    // WinUI Flyout 默认 light-dismiss：非 modal、不变暗
    setModal(false);
    setDim(false);
    setClosePolicy(ClosePolicy(CloseOnPressOutside | CloseOnEscape));
}

Flyout::~Flyout() = default;

void Flyout::setPlacement(Placement p) {
    if (m_placement == p) return;
    m_placement = p;
    emit placementChanged(p);
}

void Flyout::setAnchor(QWidget* anchor) {
    m_anchor = anchor;
}

void Flyout::showAt(QWidget* anchor) {
    setAnchor(anchor);
    open();
}

// ── 几何辅助 ─────────────────────────────────────────────────────────────────

QRect Flyout::anchorRectInTopLevel() const {
    if (!m_anchor) return QRect();
    QWidget* top = m_anchor->window();
    if (!top) return QRect();
    const QPoint tl = m_anchor->mapTo(top, QPoint(0, 0));
    return QRect(tl, m_anchor->size());
}

QPoint Flyout::clampCardPos(const QPoint& cardTopLeft) const {
    if (!m_clampToWindow) return cardTopLeft;
    QWidget* top = m_anchor ? m_anchor->window() : parentWidget();
    if (!top) return cardTopLeft;

    // 卡片可见尺寸 = 整体 size - 两侧 shadow margin
    const int cardW = width()  - kShadowMargin * 2;
    const int cardH = height() - kShadowMargin * 2;

    int x = cardTopLeft.x();
    int y = cardTopLeft.y();
    const int margin = 4;  // 与窗口边缘留点呼吸空间
    x = qBound(margin, x, top->width()  - cardW - margin);
    y = qBound(margin, y, top->height() - cardH - margin);
    return QPoint(x, y);
}

Flyout::Placement Flyout::resolveAutoPlacement() const {
    if (!m_anchor) return Bottom;
    QWidget* top = m_anchor->window();
    if (!top) return Bottom;

    const QRect a = anchorRectInTopLevel();
    const int cardH = height() - kShadowMargin * 2;
    const int needed = cardH + m_anchorOffset;

    const int spaceBelow = top->height() - a.bottom();
    const int spaceAbove = a.top();

    if (spaceBelow >= needed)            return Bottom;
    if (spaceAbove >= needed)            return Top;
    return spaceBelow >= spaceAbove ? Bottom : Top;
}

// ── 位置计算 ─────────────────────────────────────────────────────────────────

QPoint Flyout::computePosition() const {
    // 没有 anchor → 退化到基类（居中）
    if (!m_anchor || !m_anchor->window()) {
        return Popup::computePosition();
    }

    Placement p = m_placement;
    if (p == Auto) p = resolveAutoPlacement();

    if (p == Full) {
        return Popup::computePosition();
    }

    const QRect a = anchorRectInTopLevel();
    const int cardW = width()  - kShadowMargin * 2;
    const int cardH = height() - kShadowMargin * 2;

    // 卡片左上角（top-level 坐标）
    QPoint card;
    switch (p) {
        case Top:
            card = QPoint(a.center().x() - cardW / 2,
                          a.top()    - m_anchorOffset - cardH);
            break;
        case Bottom:
            card = QPoint(a.center().x() - cardW / 2,
                          a.bottom() + m_anchorOffset);
            break;
        case Left:
            card = QPoint(a.left()  - m_anchorOffset - cardW,
                          a.center().y() - cardH / 2);
            break;
        case Right:
            card = QPoint(a.right() + m_anchorOffset,
                          a.center().y() - cardH / 2);
            break;
        case Full:
        case Auto:
            // 已在前面处理 / 转换
            return Popup::computePosition();
    }

    card = clampCardPos(card);

    // 转换为 widget.move() 坐标：减去 shadow margin
    return card - QPoint(kShadowMargin, kShadowMargin);
}

} // namespace view::dialogs_flyouts
