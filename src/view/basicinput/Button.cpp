#include "Button.h"

namespace view::basicinput {

Button::Button(const QString& text, QWidget* parent) : QPushButton(text, parent) {
    setAttribute(Qt::WA_Hover);
}

Button::Button(QWidget* parent) : QPushButton(parent) {
    setAttribute(Qt::WA_Hover);
}

void Button::setFluentStyle(ButtonStyle style) {
    if (m_style != style) { m_style = style; update(); emit fluentStyleChanged(); }
}

void Button::setFluentSize(ButtonSize size) {
    if (m_size != size) { 
        m_size = size; 
        updateGeometry(); // 关键：通知布局系统尺寸已变化
        update(); 
        emit fluentSizeChanged(); 
    }
}

void Button::setFluentLayout(ButtonLayout layout) {
    if (m_layout != layout) { 
        m_layout = layout; 
        updateGeometry();
        update(); 
        emit fluentLayoutChanged(); 
    }
}

void Button::setFocusVisual(bool focus) {
    if (m_focusVisual != focus) { m_focusVisual = focus; update(); emit focusVisualChanged(); }
}

void Button::setInteractionState(InteractionState state) {
    if (m_interactionState != state) { m_interactionState = state; update(); emit interactionStateChanged(); }
}

QSize Button::sizeHint() const {
    const auto& spacing = themeSpacing();
    const auto& typo = themeFont(m_size == Large ? "BodyStrong" : "Body");
    QFontMetrics fm(typo.toQFont());

    // 1. 基于 Token 计算内边距 (Padding)
    // 水平内边距：Small(8px), Standard(12px), Large(16px)
    int hPadding = (m_size == Small) ? spacing.small : (m_size == Large ? spacing.standard : spacing.padding.controlH);
    // 垂直内边距：Small(4px), Standard(6px), Large(8px)
    int vPadding = (m_size == Small) ? spacing.gap.tight : (m_size == Large ? spacing.small : spacing.padding.controlV);
    // 图标间距：Small(4px), Standard(8px)
    int iconGap = (m_size == Small) ? spacing.gap.tight : spacing.gap.normal;

    // 2. 计算动态高度：高度 = 字体高度 + 上下内边距
    int dynamicHeight = fm.height() + vPadding * 2;

    // 3. 计算内容所需的总宽度
    QString txt = text();
    QSize icSize = iconSize();
    int contentWidth = fm.horizontalAdvance(txt);
    if (!txt.isEmpty() && !icon().isNull()) contentWidth += iconGap;
    if (!icon().isNull()) contentWidth += icSize.width();

    return QSize(contentWidth + hPadding * 2, dynamicHeight);
}

QSize Button::minimumSizeHint() const {
    return sizeHint();
}

void Button::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const auto& colors = themeColors();
    const auto& radius = themeRadius();
    const auto& spacing = themeSpacing();
    
    // 1. 确定交互状态
    InteractionState state = m_interactionState;
    if (!isEnabled()) {
        state = Disabled;
    } else if (state == Rest) {
        if (isDown()) state = Pressed;
        else if (underMouse()) state = Hover;
    }

    // 2. 获取色值和字体
    const auto& typo = themeFont(m_size == Large ? "BodyStrong" : "Body");
    painter.setFont(typo.toQFont());

    QColor bgColor, textColor, borderColor;
    if (m_style == Accent) {
        bgColor = colors.accentDefault;
        textColor = colors.textOnAccent;
        borderColor = colors.strokeStrong;
        if (state == Hover) bgColor = colors.accentSecondary;
        if (state == Pressed) bgColor = colors.accentTertiary;
    } else if (m_style == Subtle) {
        bgColor = Qt::transparent;
        textColor = colors.textPrimary;
        borderColor = Qt::transparent;
        if (state == Hover) bgColor = colors.subtleSecondary;
        if (state == Pressed) bgColor = colors.subtleTertiary;
    } else {
        bgColor = colors.controlDefault;
        textColor = colors.textPrimary;
        borderColor = colors.strokeDefault;
        if (state == Hover) bgColor = colors.controlSecondary;
        if (state == Pressed) bgColor = colors.controlTertiary;
    }

    if (state == Disabled) {
        bgColor = colors.controlDisabled;
        textColor = colors.textDisabled;
        borderColor = colors.strokeDivider;
    }

    // 3. 绘制背景和边框
    QRectF contentRect = rect();
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(contentRect, radius.control, radius.control);

    if (borderColor != Qt::transparent) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(borderColor, 1)); 
        painter.drawRoundedRect(contentRect.adjusted(0.5, 0.5, -0.5, -0.5), radius.control, radius.control);
    }

    if (state != Disabled && (hasFocus() || m_focusVisual)) {
        painter.setPen(QPen(colors.textPrimary, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(contentRect.adjusted(1.5, 1.5, -1.5, -1.5), radius.control - 1, radius.control - 1);
    }

    // 4. 计算并绘制图标和文字 (使用 Token 间距)
    QString txt = (m_layout == IconOnly) ? "" : text();
    QPixmap pix = (m_layout == TextOnly || icon().isNull()) ? QPixmap() : icon().pixmap(iconSize());
    int gap = (m_size == Small) ? spacing.gap.tight : spacing.gap.normal;

    QFontMetrics fm = painter.fontMetrics();
    int txtWidth = txt.isEmpty() ? 0 : fm.horizontalAdvance(txt);
    int pixWidth = pix.isNull() ? 0 : pix.width() / (double)pix.devicePixelRatio();
    int totalContentWidth = txtWidth + pixWidth + ((!txt.isEmpty() && !pix.isNull()) ? gap : 0);
    
    double startX = contentRect.left() + (contentRect.width() - totalContentWidth) / 2.0;
    double centerY = contentRect.center().y();

    painter.setPen(textColor);
    if (m_layout == IconAfter) {
        if (!txt.isEmpty()) {
            painter.drawText(QRectF(startX, 0, txtWidth, height()), Qt::AlignCenter, txt);
            startX += txtWidth + gap;
        }
        if (!pix.isNull()) {
            painter.drawPixmap(startX, centerY - pix.height() / (2.0 * pix.devicePixelRatio()), pix);
        }
    } else {
        if (!pix.isNull()) {
            painter.drawPixmap(startX, centerY - pix.height() / (2.0 * pix.devicePixelRatio()), pix);
            startX += pixWidth + gap;
        }
        if (!txt.isEmpty()) {
            painter.drawText(QRectF(startX, 0, txtWidth, height()), Qt::AlignCenter, txt);
        }
    }
}

} // namespace view::basicinput
