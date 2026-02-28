#include "TextBox.h"
#include <QPainter>
#include <QPainterPath>
#include <QTextOption>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QValidator>
#include <QEvent>
#include <QFocusEvent>

#include "view/basicinput/Button.h"
#include "view/scrolling/ScrollBar.h"
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

    // 预先创建 overlay 垂直滚动条，锚点方式与 TextBox 其他元素保持一致
    m_vScrollBar = new ::view::scrolling::ScrollBar(Qt::Vertical, this);
    m_vScrollBar->anchors()->right  = {this, ::view::AnchorLayout::Edge::Right, -2};
    // 跟随 TextBox 内容区域的上下边界（与 m_textEdit 的 fillMargins 中 top/bottom=1 对齐）
    m_vScrollBar->anchors()->top    = {this, ::view::AnchorLayout::Edge::Top, 2};
    m_vScrollBar->anchors()->bottom = {this, ::view::AnchorLayout::Edge::Bottom, -2};
    layout->addAnchoredWidget(m_vScrollBar, *m_vScrollBar->anchors());
    m_vScrollBar->hide();

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
            // 隐藏内部滚动条，仅保留滚动行为（滚轮 / 触控板等仍可滚动）
            m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            connect(m_textEdit, &QPlainTextEdit::textChanged, this, [this]() {
                updateClearButtonVisibility();
                emit textChanged(text());
                updateHeight();
            });
            
            ::view::AnchorLayout::Anchors anchors;
            anchors.fill = true;
            anchors.fillMargins = QMargins(0, 1, 0, 1); // 留出边框位置
            layout->addAnchoredWidget(m_textEdit, anchors);

            // 绑定内部滚动条与 overlay 滚动条
            auto* innerVBar = m_textEdit->verticalScrollBar();

            // 初始同步范围、步长和值
            m_vScrollBar->setRange(innerVBar->minimum(), innerVBar->maximum());
            m_vScrollBar->setPageStep(innerVBar->pageStep());
            m_vScrollBar->setValue(innerVBar->value());

            // 范围与步长：内部滚动条 -> 自定义滚动条（一向同步即可）
            connect(innerVBar, &QScrollBar::rangeChanged,
                    this, [this, innerVBar](int min, int max) {
                        if (!m_vScrollBar) return;
                        m_vScrollBar->setRange(min, max);
                        m_vScrollBar->setPageStep(innerVBar->pageStep());
                    });

            // 数值：使用 QMLPlus 提供的 PropertyBinder 做双向绑定
            view::PropertyBinder::bind(innerVBar, "value",
                                       m_vScrollBar, "value",
                                       view::PropertyBinder::TwoWay);
        }
        if (m_lineEdit) m_lineEdit->hide();
        m_textEdit->show();
        m_textEdit->raise(); // 放在底层，让 clearButton / ScrollBar 在上
        if (m_vScrollBar) m_vScrollBar->show(), m_vScrollBar->raise();
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
            if (m_validator) m_lineEdit->setValidator(m_validator);

            ::view::AnchorLayout::Anchors anchors;
            anchors.fill = true;
            anchors.fillMargins = QMargins(0, 1, 0, 1);
            layout->addAnchoredWidget(m_lineEdit, anchors);
        } else if (m_validator) {
            m_lineEdit->setValidator(m_validator);
        }
        if (m_textEdit) m_textEdit->hide();
        if (m_vScrollBar) m_vScrollBar->hide();
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

    // 四角均为圆角
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

void TextBox::setValidator(QValidator* validator) {
    if (m_validator == validator) return;
    m_validator = validator;
    if (m_lineEdit) m_lineEdit->setValidator(m_validator);
}

void TextBox::setClearButtonEnabled(bool enabled) {
    if (m_clearButtonEnabled != enabled) {
        m_clearButtonEnabled = enabled;
        updateClearButtonVisibility();
        emit clearButtonEnabledChanged();
    }
}

void TextBox::updateHeight() {
    // 多行模式不再根据内容自适应高度，交由外部布局 / setFixedHeight 控制。
    // 这里保留空实现，仅作为兼容旧调用点的占位。
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
