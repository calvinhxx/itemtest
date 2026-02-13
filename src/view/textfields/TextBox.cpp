#include "TextBox.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QResizeEvent>
#include "view/textfields/TextBlock.h"
#include <QVariant>
#include "common/Typography.h"
#include "common/Spacing.h"

namespace view::textfields {

TextBox::TextBox(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(themeSpacing().gap.tight);

    m_headerLabel = new TextBlock(this);
    m_headerLabel->setFluentTypography("BodyStrong"); // Header usually uses BodyStrong
    layout->addWidget(m_headerLabel);
    m_headerLabel->hide();

    // Default to Single Line
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->installEventFilter(this);
    m_lineEdit->setFrame(false); // We draw our own frame
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false); // Disable Mac default
    layout->addWidget(m_lineEdit);
    
    // MultiLine editor (initially hidden or created on demand)
    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->installEventFilter(this);
    m_textEdit->setFrameShape(QFrame::NoFrame);
    m_textEdit->hide();
    layout->addWidget(m_textEdit);

    setAttribute(Qt::WA_Hover);
    setFocusPolicy(Qt::StrongFocus);

    onThemeUpdated();
}

QString TextBox::text() const {
    if (m_isMultiLine && m_textEdit) return m_textEdit->toPlainText();
    if (m_lineEdit) return m_lineEdit->text();
    return QString();
}

void TextBox::setText(const QString& text) {
    if (m_isMultiLine && m_textEdit) {
        if (m_textEdit->toPlainText() != text) {
            m_textEdit->setPlainText(text);
            emit textChanged(text);
        }
    } else if (m_lineEdit) {
        if (m_lineEdit->text() != text) {
            m_lineEdit->setText(text);
            emit textChanged(text);
        }
    }
}

QString TextBox::placeholderText() const {
    if (m_isMultiLine && m_textEdit) return m_textEdit->placeholderText();
    if (m_lineEdit) return m_lineEdit->placeholderText();
    return QString();
}

void TextBox::setPlaceholderText(const QString& text) {
    if (m_lineEdit) m_lineEdit->setPlaceholderText(text);
    if (m_textEdit) m_textEdit->setPlaceholderText(text);
    emit placeholderTextChanged();
}

QString TextBox::header() const {
    return m_headerLabel->text();
}

void TextBox::setHeader(const QString& header) {
    m_headerLabel->setText(header);
    m_headerLabel->setVisible(!header.isEmpty());
    emit headerChanged();
}

bool TextBox::isReadOnly() const {
    if (m_isMultiLine && m_textEdit) return m_textEdit->isReadOnly();
    if (m_lineEdit) return m_lineEdit->isReadOnly();
    return false;
}

void TextBox::setReadOnly(bool readOnly) {
    if (m_lineEdit) m_lineEdit->setReadOnly(readOnly);
    if (m_textEdit) m_textEdit->setReadOnly(readOnly);
    updateStyle();
    emit readOnlyChanged();
}

void TextBox::setMultiLine(bool multiLine) {
    if (m_isMultiLine == multiLine) return;
    m_isMultiLine = multiLine;

    // Switch widget visibility
    if (m_isMultiLine) {
        // Transfer state
        m_textEdit->setPlainText(m_lineEdit->text());
        m_lineEdit->hide();
        m_textEdit->show();
    } else {
        m_lineEdit->setText(m_textEdit->toPlainText());
        m_textEdit->hide();
        m_lineEdit->show();
    }
    
    updateStyle();
    emit multiLineChanged();
}

void TextBox::onThemeUpdated() {
    const auto& c = themeColors();
    const auto& s = themeSpacing();

    // Text Colors
    QPalette p = palette();
    p.setColor(QPalette::Text, c.textPrimary);
    p.setColor(QPalette::PlaceholderText, c.textSecondary);
    p.setColor(QPalette::Highlight, c.accentDefault);
    p.setColor(QPalette::HighlightedText, c.textOnAccent);
    
    if (m_lineEdit) {
        m_lineEdit->setPalette(p);
        m_lineEdit->setFont(themeFont("Body").toQFont());
        // Add padding inside line edit
        m_lineEdit->setStyleSheet(QString("QLineEdit { padding: %1px; background: transparent; selection-background-color: %2; }")
                                  .arg(s.padding.controlH)
                                  .arg(c.accentDefault.name()));
    }
    if (m_textEdit) {
        m_textEdit->setPalette(p);
        m_textEdit->setFont(themeFont("Body").toQFont());
        m_textEdit->setStyleSheet(QString("QPlainTextEdit { padding: %1px; background: transparent; selection-background-color: %2; }")
                                  .arg(s.padding.controlH)
                                  .arg(c.accentDefault.name()));
    }

    // Border/Background Colors
    m_bgColor = c.controlDefault; 
    m_borderColor = c.strokeDefault;
    m_borderActiveColor = c.accentDefault; 
    m_bottomBorderColor = c.strokeStrong;

    update();
}

bool TextBox::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_lineEdit || watched == m_textEdit) {
        if (event->type() == QEvent::FocusIn) {
            m_isFocused = true;
            update();
        } else if (event->type() == QEvent::FocusOut) {
            m_isFocused = false;
            update();
        } else if (event->type() == QEvent::HoverEnter) {
            m_isHovered = true;
            update();
        } else if (event->type() == QEvent::HoverLeave) {
            m_isHovered = false;
            update();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TextBox::updateStyle() {
    // Style switching logic if read-only or disabled
    // For now handled in paintEvent based on state
    update();
}

void TextBox::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Determine the geometry of the input area (excluding header)
    QWidget* activeWidget = m_isMultiLine ? (QWidget*)m_textEdit : (QWidget*)m_lineEdit;
    if (!activeWidget || !activeWidget->isVisible()) return;

    QRect r = activeWidget->geometry();
    // Adjust rect slightly to draw border *outside* the content area if needed, 
    // or we assume the layout gave it enough space. 
    // Actually, QLineEdit with no frame paints background? No, we set background: transparent.
    // So we need to paint background behind the widget.

    // Background
    QColor bg = m_bgColor;
    if (isReadOnly()) {
        bg = themeColors().controlDisabled; // Lighter/Distinct for ReadOnly
    } else if (m_isHovered && !m_isFocused) {
        bg = themeColors().controlSecondary; // Slightly darker on hover
    }
    
    // Draw Background
    const qreal radius = themeRadius().control; // Use control radius, overlay is too big maybe? But WinUI text box is fairly rounded. Overlay is usually 8px, control 4px.
    // WinUI 3 TextBox corner radius is 4px. So 'control' (4px typically) is better.
    // Wait, check FluentElement.h radius values.
    // But let's stick to replacing lines.
    
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(r, radius, radius);

    // Draw Border
    // Standard: 1px bottom border (Strong), others normal.
    // Focused: 2px accent bottom border.
    
    if (m_isFocused) {
        // Full border? WinUI 3 TextBox has a bottom underline highlight.
        // Let's draw a full thin border and a thick bottom border.
        p.setPen(QPen(m_borderColor, 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(0,0,-1,-1), radius, radius);

        // Bottom Accent Strok
        p.setPen(QPen(m_borderActiveColor, 2));
        // Draw line at bottom
        // Adjusted for rounded corners? 
        // Simple 2px line at bottom.
        int y = r.bottom();
        p.drawLine(r.left() + radius, y, r.right() - radius, y);
    } else {
        // Rest State
        p.setPen(QPen(m_borderColor, 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(0,0,-1,-1), radius, radius);

        // Bottom Strong Border (1px but darker)
        if (!isReadOnly()) {
            p.setPen(QPen(m_bottomBorderColor, 1));
            int y = r.bottom();
            p.drawLine(r.left() + radius, y, r.right() - radius, y);
        }
    }
}

void TextBox::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void TextBox::updateLayout() {
    // handled by QVBoxLayout
}

} // namespace view::textfields
