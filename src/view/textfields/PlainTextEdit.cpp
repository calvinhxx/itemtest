#include "PlainTextEdit.h"
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextOption>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QFocusEvent>

namespace view::textfields {

PlainTextEdit::PlainTextEdit(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_Hover);

    m_editor = new QPlainTextEdit(this);
    m_editor->setFrameStyle(QFrame::NoFrame);
    m_editor->setBackgroundRole(QPalette::NoRole);
    m_editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_editor->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_editor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_editor->setAutoFillBackground(false);

    // 监听文本变化 / 焦点状态，用于边框刷新
    connect(m_editor, &QPlainTextEdit::textChanged, this, [this]() {
        emit textChanged();
    });
    m_editor->installEventFilter(this);

    // 初始应用当前主题样式
    onThemeUpdated();
}

// 文本 API 转发
void PlainTextEdit::setPlainText(const QString& text) {
    if (m_editor) m_editor->setPlainText(text);
}

QString PlainTextEdit::toPlainText() const {
    return m_editor ? m_editor->toPlainText() : QString();
}

void PlainTextEdit::clear() {
    if (m_editor) m_editor->clear();
}

void PlainTextEdit::setPlaceholderText(const QString& text) {
    if (m_editor) m_editor->setPlaceholderText(text);
}

QString PlainTextEdit::placeholderText() const {
    return m_editor ? m_editor->placeholderText() : QString();
}

void PlainTextEdit::setReadOnly(bool readOnly) {
    if (m_editor) m_editor->setReadOnly(readOnly);
}

bool PlainTextEdit::isReadOnly() const {
    return m_editor ? m_editor->isReadOnly() : false;
}

QScrollBar* PlainTextEdit::verticalScrollBar() const {
    return m_editor ? m_editor->verticalScrollBar() : nullptr;
}

void PlainTextEdit::setFocus(Qt::FocusReason reason) {
    if (m_editor) m_editor->setFocus(reason);
}

void PlainTextEdit::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    paintFrame(p);
    QWidget::paintEvent(event);
}

void PlainTextEdit::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_editor) {
        // 填满整个区域，由外层控件负责绘制边框和背景
        m_editor->setGeometry(rect());
    }
}

void PlainTextEdit::paintFrame(QPainter& painter) {
    const auto& colors = themeColors();
    const auto& radius = themeRadius();
    QRectF bgRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    QColor bgColor, borderColor, bottomBorderColor;
    int bottomBorderWidth = m_unfocusedBorderWidth;
    if (!isEnabled()) {
        bgColor = colors.controlDisabled;
        borderColor = colors.strokeDivider;
        bottomBorderColor = borderColor;
    } else if (isReadOnly()) {
        bgColor = colors.controlAltSecondary;
        borderColor = colors.strokeDefault;
        bottomBorderColor = colors.strokeDivider;
    } else if (m_isFocused) {
        bgColor = (currentTheme() == Dark) ? colors.bgSolid : colors.controlDefault;
        borderColor = colors.strokeSecondary;
        bottomBorderColor = colors.accentDefault;
        bottomBorderWidth = m_focusedBorderWidth;
    } else if (m_isHovered) {
        bgColor = colors.controlSecondary;
        borderColor = colors.strokeSecondary;
        bottomBorderColor = colors.strokeSecondary;
    } else {
        bgColor = colors.controlDefault;
        borderColor = colors.strokeDefault;
        bottomBorderColor = colors.strokeDivider;
    }

    qreal r = radius.control;
    QPainterPath framePath;
    framePath.addRoundedRect(bgRect, r, r);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawPath(framePath);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(borderColor, 1));
    painter.drawPath(framePath);

    if (isEnabled() && !isReadOnly()) {
        QPen pen(bottomBorderColor, bottomBorderWidth);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        qreal bottomY = bgRect.bottom() - (bottomBorderWidth > 1 ? (bottomBorderWidth - 1) / 2.0 : 0);
        QPainterPath bottomPath;
        bottomPath.moveTo(bgRect.left() + r, bottomY);
        bottomPath.lineTo(bgRect.right() - r, bottomY);
        painter.drawPath(bottomPath);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void PlainTextEdit::enterEvent(QEnterEvent* event) {
    m_isHovered = true;
    update();
    QWidget::enterEvent(event);
}
#else
void PlainTextEdit::enterEvent(QEvent* event) {
    m_isHovered = true;
    update();
    QWidget::enterEvent(event);
}
#endif

void PlainTextEdit::leaveEvent(QEvent* event) {
    m_isHovered = false;
    update();
    QWidget::leaveEvent(event);
}

void PlainTextEdit::focusInEvent(QFocusEvent* event) {
    // 外层获得焦点时也更新状态（例如通过 Tab 导航）
    m_isFocused = true;
    update();
    QWidget::focusInEvent(event);
}

void PlainTextEdit::focusOutEvent(QFocusEvent* event) {
    m_isFocused = false;
    update();
    QWidget::focusOutEvent(event);
}

void PlainTextEdit::setContentMargins(const QMargins& margins) {
    if (m_contentMargins == margins)
        return;
    m_contentMargins = margins;
    applyThemeStyle();
    emit contentMarginsChanged();
}

void PlainTextEdit::onThemeUpdated() {
    applyThemeStyle();
}

void PlainTextEdit::setFocusedBorderWidth(int width) {
    if (m_focusedBorderWidth == width)
        return;
    m_focusedBorderWidth = width;
    update();
    emit borderWidthChanged();
}

void PlainTextEdit::setUnfocusedBorderWidth(int width) {
    if (m_unfocusedBorderWidth == width)
        return;
    m_unfocusedBorderWidth = width;
    update();
    emit borderWidthChanged();
}

void PlainTextEdit::applyThemeStyle() {
    const auto& c = themeColors();
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Window, Qt::transparent);
    pal.setColor(QPalette::Text, c.textPrimary);
    pal.setColor(QPalette::PlaceholderText, c.textSecondary);
    pal.setColor(QPalette::Highlight, c.accentDefault);
    pal.setColor(QPalette::HighlightedText, c.textOnAccent);
    pal.setColor(QPalette::Inactive, QPalette::Highlight, c.accentDefault);
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText, c.textOnAccent);
    pal.setColor(QPalette::Disabled, QPalette::Text, c.textDisabled);
    pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, c.textDisabled);
    if (m_editor) {
        m_editor->setPalette(pal);
        m_editor->setFont(themeFont("Body").toQFont());

        QString qss = QString(
                         "QPlainTextEdit { "
                         "background: transparent; "
                         "color: %5; "
                         "selection-background-color: %6; "
                         "selection-color: %7; "
                         "padding-left: %1px; padding-right: %2px; "
                         "padding-top: %3px; padding-bottom: %4px; "
                         "border: none; }")
                         .arg(m_contentMargins.left())
                         .arg(m_contentMargins.right())
                         .arg(m_contentMargins.top())
                         .arg(m_contentMargins.bottom())
                         .arg(c.textPrimary.name(QColor::HexArgb))
                         .arg(c.accentDefault.name(QColor::HexArgb))
                         .arg(c.textOnAccent.name(QColor::HexArgb));
        m_editor->setStyleSheet(qss);

        // 同步 viewport：确保其背景与边框完全透明，否则会遮挡外层自绘的 Fluent 外框
        if (auto* vp = m_editor->viewport()) {
            vp->setAutoFillBackground(false);
            QPalette vpal = vp->palette();
            vpal.setColor(QPalette::Base, Qt::transparent);
            vpal.setColor(QPalette::Window, Qt::transparent);
            vp->setPalette(vpal);
            vp->setStyleSheet("background: transparent; border: none;");
        }
    }
}

} // namespace view::textfields
