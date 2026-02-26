#include "TextBox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractTextDocumentLayout>
#include <QTextOption>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QEvent>
#include <QFocusEvent>

#include "view/basicinput/Button.h"
#include "common/Typography.h"

namespace view::textfields {

TextBox::TextBox(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    initUi();
}

void TextBox::initUi() {
    auto* layout = new ::view::AnchorLayout(this);
    
    m_clearButton = new ::view::basicinput::Button(this);
    m_clearButton->setFluentStyle(::view::basicinput::Button::Subtle);
    m_clearButton->setFluentSize(::view::basicinput::Button::Small);
    m_clearButton->setFocusPolicy(Qt::NoFocus);
    m_clearButton->setIconGlyph(::Typography::Icons::Dismiss, 
                                 ::Typography::FontSize::Body, 
                                 ::Typography::FontFamily::SegoeFluentIcons);
    m_clearButton->setFixedSize(m_clearButtonSize, m_clearButtonSize);
    
    // 配置清除按钮锚点
    m_clearButton->anchors()->right = {this, ::view::AnchorLayout::Edge::Right, -m_clearButtonOffset.x()};
    m_clearButton->anchors()->verticalCenter = {this, ::view::AnchorLayout::Edge::VCenter, m_clearButtonOffset.y()};
    layout->addAnchoredWidget(m_clearButton, *m_clearButton->anchors());
    
    m_clearButton->hide();

    connect(m_clearButton, &::view::basicinput::Button::clicked, this, [this]() {
        if (m_multiLine && m_textEdit) m_textEdit->clear();
        else if (!m_multiLine && m_lineEdit) m_lineEdit->clear();
        setFocus();
    });

    setAttribute(Qt::WA_Hover);
    ensureEditor();
}

void TextBox::ensureEditor() {
    auto* layout = qobject_cast<::view::AnchorLayout*>(this->layout());
    if (!layout) return;

    if (m_multiLine) {
        if (!m_textEdit) {
            m_textEdit = new QPlainTextEdit(this);
            m_textEdit->setFrameStyle(QFrame::NoFrame);
            m_textEdit->setBackgroundRole(QPalette::NoRole);
            m_textEdit->installEventFilter(this);
            m_textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
            m_textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            if (m_textEdit->verticalScrollBar()) {
                m_textEdit->verticalScrollBar()->setStyleSheet("QScrollBar { background: transparent; width: 8px; }");
            }
            connect(m_textEdit, &QPlainTextEdit::textChanged, this, [this]() {
                updateClearButtonVisibility();
                emit textChanged(text());
                updateHeight();
            });
            
            ::view::AnchorLayout::Anchors anchors;
            anchors.fill = true;
            anchors.fillMargins = QMargins(0, 1, 0, 1); // 留出边框位置
            layout->addAnchoredWidget(m_textEdit, anchors);
        }
        if (m_lineEdit) m_lineEdit->hide();
        m_textEdit->show();
        m_textEdit->raise(); // 放在底层，让 clearButton 在上
    } else {
        if (!m_lineEdit) {
            m_lineEdit = new QLineEdit(this);
            m_lineEdit->setFrame(false);
            m_lineEdit->setAttribute(Qt::WA_Hover);
            m_lineEdit->installEventFilter(this);
            connect(m_lineEdit, &QLineEdit::textChanged, this, [this]() {
                updateClearButtonVisibility();
                emit textChanged(text());
            });
            connect(m_lineEdit, &QLineEdit::returnPressed, this, &TextBox::returnPressed);

            ::view::AnchorLayout::Anchors anchors;
            anchors.fill = true;
            anchors.fillMargins = QMargins(0, 1, 0, 1);
            layout->addAnchoredWidget(m_lineEdit, anchors);
        }
        if (m_textEdit) m_textEdit->hide();
        m_lineEdit->show();
        m_lineEdit->raise();
    }
    
    m_clearButton->raise(); // 确保清除按钮始终在最上层
    onThemeUpdated();
}

void TextBox::setFocus(Qt::FocusReason reason) {
    if (m_multiLine && m_textEdit) m_textEdit->setFocus(reason);
    else if (!m_multiLine && m_lineEdit) m_lineEdit->setFocus(reason);
}

void TextBox::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

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

    // 仅底部两个角为圆角，顶部保持直角
    qreal r = radius.control;
    QPainterPath framePath;
    framePath.moveTo(bgRect.left(), bgRect.top());
    framePath.lineTo(bgRect.right(), bgRect.top());
    framePath.lineTo(bgRect.right(), bgRect.bottom() - r);
    framePath.quadTo(QPointF(bgRect.right(), bgRect.bottom()),
                     QPointF(bgRect.right() - r, bgRect.bottom()));
    framePath.lineTo(bgRect.left() + r, bgRect.bottom());
    framePath.quadTo(QPointF(bgRect.left(), bgRect.bottom()),
                     QPointF(bgRect.left(), bgRect.bottom() - r));
    framePath.closeSubpath();

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawPath(framePath);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(borderColor, 1));
    painter.drawPath(framePath);

    if (isEnabled() && !isReadOnly()) {
        QPen pen(bottomBorderColor, bottomBorderWidth);
        // 使用圆角端点，让底部高亮线两端呈半圆，而非直角矩形
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);

        QPainterPath bottomPath;
        qreal bottomY = bgRect.bottom() - (bottomBorderWidth > 1 ? (bottomBorderWidth - 1) / 2.0 : 0);
        bottomPath.moveTo(bgRect.left() + r, bottomY);
        bottomPath.lineTo(bgRect.right() - r, bottomY);
        painter.drawPath(bottomPath);
    }
}

QString TextBox::text() const {
    if (m_multiLine && m_textEdit) return m_textEdit->toPlainText();
    if (!m_multiLine && m_lineEdit) return m_lineEdit->text();
    return QString();
}

void TextBox::setText(const QString& text) {
    if (m_multiLine) {
        if (m_textEdit) m_textEdit->setPlainText(text);
    } else {
        if (m_lineEdit) m_lineEdit->setText(text);
    }
}

QString TextBox::placeholderText() const {
    if (m_lineEdit) return m_lineEdit->placeholderText();
    if (m_textEdit) return m_textEdit->placeholderText();
    return QString();
}

void TextBox::setPlaceholderText(const QString& placeholder) {
    if (m_lineEdit) m_lineEdit->setPlaceholderText(placeholder);
    if (m_textEdit) m_textEdit->setPlaceholderText(placeholder);
}

bool TextBox::isReadOnly() const {
    if (m_lineEdit) return m_lineEdit->isReadOnly();
    if (m_textEdit) return m_textEdit->isReadOnly();
    return false;
}

void TextBox::setReadOnly(bool readOnly) {
    if (m_lineEdit) m_lineEdit->setReadOnly(readOnly);
    if (m_textEdit) m_textEdit->setReadOnly(readOnly);
    updateClearButtonVisibility();
    update();
    emit readOnlyChanged();
}

void TextBox::setMultiLine(bool multiLine) {
    if (m_multiLine == multiLine) return;
    QString currentText = text();
    m_multiLine = multiLine;
    
    ensureEditor();
    setText(currentText);
    updateClearButtonVisibility();
    emit multiLineChanged();
}

void TextBox::setClearButtonSize(int size) {
    if (m_clearButtonSize != size) {
        m_clearButtonSize = size;
        m_clearButton->setFixedSize(size, size);
        onThemeUpdated();
        emit clearButtonSizeChanged();
    }
}

void TextBox::setTextMargins(const QMargins& margins) {
    if (m_textMargins != margins) {
        m_textMargins = margins;
        onThemeUpdated();
        emit textMarginsChanged();
    }
}

void TextBox::setClearButtonOffset(const QPoint& offset) {
    if (m_clearButtonOffset != offset) {
        m_clearButtonOffset = offset;
        if (m_clearButton) {
            m_clearButton->anchors()->right = {this, ::view::AnchorLayout::Edge::Right, -m_clearButtonOffset.x()};
            m_clearButton->anchors()->verticalCenter = {this, ::view::AnchorLayout::Edge::VCenter, m_clearButtonOffset.y()};
            if (layout()) layout()->invalidate();
        }
        emit clearButtonOffsetChanged();
    }
}

void TextBox::setFocusedBorderWidth(int width) {
    if (m_focusedBorderWidth != width) {
        m_focusedBorderWidth = width;
        update();
        emit borderWidthChanged();
    }
}

void TextBox::setUnfocusedBorderWidth(int width) {
    if (m_unfocusedBorderWidth != width) {
        m_unfocusedBorderWidth = width;
        update();
        emit borderWidthChanged();
    }
}

void TextBox::setClearButtonEnabled(bool enabled) {
    if (m_clearButtonEnabled != enabled) {
        m_clearButtonEnabled = enabled;
        updateClearButtonVisibility();
        emit clearButtonEnabledChanged();
    }
}

void TextBox::updateHeight() {
    if (!m_multiLine || !m_textEdit) return;

    const auto& s = themeSpacing();

    // 使用文档布局计算像素高度，能同时反映显式换行和自动换行
    auto* docLayout = m_textEdit->document()->documentLayout();
    if (!docLayout) return;

    QFontMetrics fm(m_textEdit->font());
    int lineHeight = fm.lineSpacing();

    // 文档内容高度（像素），包含自动换行
    int docHeight = static_cast<int>(docLayout->documentSize().height());
    if (docHeight < lineHeight) {
        docHeight = lineHeight;
    }

    // 至少展示 2 行，使其在视觉上明显是多行输入
    int minLines = 2;
    int minH = minLines * lineHeight
               + m_textMargins.top() + m_textMargins.bottom()
               + 8; // 焦点条等额外空间

    // 期望高度 = 内容高度 + padding
    int desiredHeight = docHeight
                        + m_textMargins.top() + m_textMargins.bottom()
                        + 8;

    // 默认最大高度按 10 行估算，除非外部显式设置了 maximumHeight
    int maxPixel;
    if (maximumHeight() > 0 && maximumHeight() < 16777215) {
        maxPixel = maximumHeight();
    } else {
        maxPixel = 10 * lineHeight
                   + m_textMargins.top() + m_textMargins.bottom()
                   + 8;
    }
    if (maxPixel < minH) maxPixel = minH;

    int finalHeight = qBound(minH, desiredHeight, maxPixel);

    if (height() != finalHeight) {
        setFixedHeight(finalHeight);
        updateGeometry();

        if (auto* parent = parentWidget()) {
            if (auto* lay = parent->layout()) {
                lay->invalidate();
            }
            parent->updateGeometry();
        }
    }

    // 内容高度大于最终高度时，允许滚动
    bool overflow = desiredHeight > finalHeight + 1;
    m_textEdit->setVerticalScrollBarPolicy(overflow ? Qt::ScrollBarAsNeeded
                                                    : Qt::ScrollBarAlwaysOff);
}

void TextBox::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_multiLine) {
        updateHeight();
    }
}

void TextBox::updateClearButtonVisibility() {
    bool hasText = !text().isEmpty();
    bool shouldShow = !m_multiLine && m_clearButtonEnabled && hasText && !isReadOnly() && (m_isFocused || m_isHovered);
    m_clearButton->setVisible(shouldShow);
}

void TextBox::onThemeUpdated() {
    const auto& c = themeColors();
    const auto& s = themeSpacing();
    
    m_clearButton->setFixedSize(m_clearButtonSize, m_clearButtonSize);

    if (m_multiLine) {
        updateHeight();
    } else {
        QFontMetrics fm(themeFont("Body").toQFont());
        int dynamicHeight = fm.height() + m_textMargins.top() + m_textMargins.bottom() + 2; 
        setFixedHeight(qMax(dynamicHeight, s.standard * 2));
    }

    auto applyStyle = [&](QWidget* w, const QString& className) {
        if (!w) return;
        QPalette pal = w->palette();
        pal.setColor(QPalette::Text, c.textPrimary);
        pal.setColor(QPalette::PlaceholderText, c.textSecondary);
        pal.setColor(QPalette::Highlight, c.accentDefault);
        pal.setColor(QPalette::HighlightedText, c.textOnAccent);
        pal.setColor(QPalette::Inactive, QPalette::Highlight, c.accentDefault);
        pal.setColor(QPalette::Inactive, QPalette::HighlightedText, c.textOnAccent);
        
        w->setPalette(pal);
        w->setFont(themeFont("Body").toQFont());
        
        QString qss = QString("%1 { background: transparent; "
                              "color: %6; "
                              "selection-background-color: %7; "
                              "selection-color: %8; "
                              "padding-left: %2px; padding-right: %3px; "
                              "padding-top: %4px; padding-bottom: %5px; "
                              "border: none; }")
                        .arg(className)
                        .arg(m_textMargins.left())
                        .arg((!m_multiLine && m_clearButtonEnabled) ? (m_textMargins.right() + m_clearButtonSize) : m_textMargins.right())
                        .arg(m_textMargins.top())
                        .arg(m_textMargins.bottom())
                        .arg(c.textPrimary.name(QColor::HexArgb))
                        .arg(c.accentDefault.name(QColor::HexArgb))
                        .arg(c.textOnAccent.name(QColor::HexArgb));
        w->setStyleSheet(qss);
    };

    applyStyle(m_lineEdit, "QLineEdit");
    applyStyle(m_textEdit, "QPlainTextEdit");

    update();
}

bool TextBox::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_lineEdit || watched == m_textEdit) {
        if (event->type() == QEvent::FocusIn) {
            m_isFocused = true;
            updateClearButtonVisibility();
            update();
        } else if (event->type() == QEvent::FocusOut) {
            m_isFocused = false;
            updateClearButtonVisibility();
            update();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TextBox::enterEvent(QEnterEvent* event) {
    m_isHovered = true;
    updateClearButtonVisibility();
    update();
    QWidget::enterEvent(event);
}

void TextBox::leaveEvent(QEvent* event) {
    m_isHovered = false;
    updateClearButtonVisibility();
    update();
    QWidget::leaveEvent(event);
}

} // namespace view::textfields
