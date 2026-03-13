#include "PlainTextEdit.h"
#include <QPlainTextEdit>
#include <QPlainTextDocumentLayout>
#include <QScrollBar>
#include <QTextOption>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QFocusEvent>
#include "view/scrolling/ScrollBar.h"

namespace view::textfields {

class InnerPlainTextEdit : public QPlainTextEdit {
public:
    explicit InnerPlainTextEdit(PlainTextEdit* owner, QWidget* parent = nullptr)
        : QPlainTextEdit(parent), m_owner(owner) {}

protected:
    void paintEvent(QPaintEvent* e) override {
        QPlainTextEdit::paintEvent(e);
        if (!m_owner)
            return;
        // 仅在未聚焦且文档为空时绘制占位符，避免与输入法预编辑冲突
        if (hasFocus())
            return;
        if (!document() || !document()->isEmpty())
            return;
        const QString text = m_owner->placeholderText();
        if (text.isEmpty())
            return;

        QPainter p(viewport());
        const auto& colors = m_owner->themeColors();
        p.setPen(colors.textSecondary);
        p.setFont(m_owner->themeFont(m_owner->fontRole()).toQFont());

        QMargins m = m_owner->contentMargins();
        QRect r = viewport()->rect().adjusted(m.left(), m.top(),
                                              -m.right(), -m.bottom());
        p.drawText(r, Qt::AlignLeft | Qt::AlignTop, text);
    }

private:
    PlainTextEdit* m_owner = nullptr;
};

PlainTextEdit::PlainTextEdit(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_Hover);

    m_editor = new InnerPlainTextEdit(this, this);
    m_editor->setFrameStyle(QFrame::NoFrame);
    m_editor->setBackgroundRole(QPalette::NoRole);
    m_editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_editor->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_editor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_editor->setAutoFillBackground(false);

    // 监听文本变化 / 焦点状态，用于边框刷新和高度更新
    connect(m_editor, &QPlainTextEdit::textChanged, this, [this]() {
        updateHeightForContent();
        emit textChanged();
    });
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged,
            this, &PlainTextEdit::cursorPositionChanged);
    connect(m_editor, &QPlainTextEdit::selectionChanged,
            this, &PlainTextEdit::selectionChanged);
    m_editor->installEventFilter(this);

    // 自定义 Fluent 滚动条（覆盖内部 QPlainTextEdit 的系统滚动条）
    m_vScrollBar = new ::view::scrolling::ScrollBar(Qt::Vertical, this);
    m_vScrollBar->hide();

    auto* innerVBar = m_editor->verticalScrollBar();
    m_vScrollBar->setRange(innerVBar->minimum(), innerVBar->maximum());
    m_vScrollBar->setPageStep(innerVBar->pageStep());
    m_vScrollBar->setValue(innerVBar->value());

    connect(innerVBar, &QScrollBar::rangeChanged,
            this, [this, innerVBar](int min, int max) {
                if (!m_vScrollBar) return;
                m_vScrollBar->setRange(min, max);
                m_vScrollBar->setPageStep(innerVBar->pageStep());
                m_vScrollBar->setVisible(min != max);
            });

    connect(innerVBar, &QScrollBar::valueChanged,
            this, [this](int v) {
                if (m_vScrollBar && m_vScrollBar->value() != v)
                    m_vScrollBar->setValue(v);
            });

    connect(m_vScrollBar, &QScrollBar::valueChanged,
            this, [this, innerVBar](int v) {
                if (innerVBar->value() != v)
                    innerVBar->setValue(v);
            });

    // 初始应用当前主题样式并根据内容设置高度
    onThemeUpdated();
    updateHeightForContent();
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
    m_placeholderText = text;
}

QString PlainTextEdit::placeholderText() const {
    return m_placeholderText;
}

void PlainTextEdit::setReadOnly(bool readOnly) {
    if (m_editor) m_editor->setReadOnly(readOnly);
}

bool PlainTextEdit::isReadOnly() const {
    return m_editor ? m_editor->isReadOnly() : false;
}

::view::scrolling::ScrollBar* PlainTextEdit::verticalScrollBar() const {
    return m_vScrollBar;
}

void PlainTextEdit::setFocus() {
    if (m_editor) m_editor->setFocus(Qt::OtherFocusReason);
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
        QRect r = rect();
        int sbw = m_vScrollBar ? m_vScrollBar->thickness() : 0;
        // 让编辑器避开自定义滚动条区域
        m_editor->setGeometry(r.adjusted(0, 0, -sbw, 0));
        if (m_vScrollBar) {
            int x = r.right() - sbw + 1;
            int y = r.top() + 2;
            int h = r.height() - 4;
            m_vScrollBar->setGeometry(x, y, sbw, h);
        }
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
    updateHeightForContent();
    emit contentMarginsChanged();
}

void PlainTextEdit::onThemeUpdated() {
    applyThemeStyle();
}

void PlainTextEdit::setFontRole(const QString& role) {
    if (m_fontRole == role)
        return;
    m_fontRole = role;
    applyThemeStyle();
    if (m_editor && m_editor->viewport()) {
        m_editor->viewport()->update();
    }
    emit fontRoleChanged();
}

void PlainTextEdit::setFocusedBorderWidth(int width) {
    if (m_focusedBorderWidth == width)
        return;
    m_focusedBorderWidth = width;
    update();
    emit focusedBorderWidthChanged();
}

void PlainTextEdit::setUnfocusedBorderWidth(int width) {
    if (m_unfocusedBorderWidth == width)
        return;
    m_unfocusedBorderWidth = width;
    update();
    emit unfocusedBorderWidthChanged();
}

void PlainTextEdit::setLineHeight(int height) {
    if (height <= 0 || m_lineHeight == height)
        return;
    m_lineHeight = height;
    updateHeightForContent();
    emit layoutMetricsChanged();
}

void PlainTextEdit::setMinVisibleLines(int lines) {
    if (lines <= 0 || m_minVisibleLines == lines)
        return;
    m_minVisibleLines = lines;
    updateHeightForContent();
    emit layoutMetricsChanged();
}

void PlainTextEdit::setMaxVisibleLines(int lines) {
    if (lines <= 0 || m_maxVisibleLines == lines)
        return;
    m_maxVisibleLines = lines;
    updateHeightForContent();
    emit layoutMetricsChanged();
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
        m_editor->setFont(themeFont(m_fontRole).toQFont());

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

void PlainTextEdit::updateHeightForContent() {
    if (!m_editor)
        return;

    int lines = m_editor->document()->blockCount();
    if (lines < 1)
        lines = 1;

    // 高度在 [minVisibleLines, maxVisibleLines] 范围内自适应
    const int clampedLines = qBound(m_minVisibleLines, lines, m_maxVisibleLines);
    const int h = clampedLines * m_lineHeight
                  + m_contentMargins.top() + m_contentMargins.bottom();
    setFixedHeight(h);
}

} // namespace view::textfields
