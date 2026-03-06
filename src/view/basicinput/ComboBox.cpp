#include "ComboBox.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionComboBox>
#include <QListView>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QtMath>

#include "common/Spacing.h"
#include "common/Typography.h"

namespace view::basicinput {

ComboBox::ComboBox(QWidget* parent)
    : QComboBox(parent) {
    setAttribute(Qt::WA_Hover);
    setEditable(false);
    setFrame(false);
    setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // 使用 QListView 以便后续统一主题
    if (!view()) {
        auto* listView = new QListView(this);
        setView(listView);
    }

    initAnimation();
    onThemeUpdated();
}

void ComboBox::initAnimation() {
    if (m_pressAnimation)
        return;
    m_pressAnimation = new QPropertyAnimation(this, "pressProgress");
    m_pressAnimation->setDuration(themeAnimation().slow);
    m_pressAnimation->setEasingCurve(themeAnimation().decelerate);
}

void ComboBox::onThemeUpdated() {
    const auto& fs = themeFont(m_fontRole);
    setFont(fs.toQFont());
    if (auto* v = view()) {
        v->setFont(fs.toQFont());
    }

    // 可编辑模式下让内部 QLineEdit 复用外部绘制的背景，避免出现“双层背景”
    if (isEditable()) {
        if (auto* le = lineEdit()) {
            le->setFrame(false);
            le->setFont(fs.toQFont());
            QPalette pal = le->palette();
            pal.setColor(QPalette::Base, Qt::transparent);
            pal.setColor(QPalette::Window, Qt::transparent);
            le->setPalette(pal);
             // 避免样式再次填充背景
            le->setAutoFillBackground(false);
            le->setAttribute(Qt::WA_TranslucentBackground);
            le->setStyleSheet(QStringLiteral("QLineEdit { background: transparent; border: none; }"));
            // 让文本与外部 contentPadding 之间保持合理间距
            le->setTextMargins(m_contentPaddingH, 0, m_contentPaddingH, 0);
        }
    }
    update();
}

void ComboBox::setFontRole(const QString& role) {
    if (m_fontRole == role)
        return;
    m_fontRole = role;
    onThemeUpdated();
    emit fontRoleChanged();
}

void ComboBox::setContentPaddingH(int padding) {
    if (m_contentPaddingH == padding)
        return;
    m_contentPaddingH = padding;
    update();
    emit contentPaddingChanged();
}

void ComboBox::setArrowWidth(int w) {
    if (m_arrowWidth == w)
        return;
    m_arrowWidth = w;
    update();
    emit arrowWidthChanged();
}

void ComboBox::setChevronGlyph(const QString& glyph) {
    if (m_chevronGlyph == glyph)
        return;
    m_chevronGlyph = glyph;
    update();
    emit chevronChanged();
}

void ComboBox::setIconFontFamily(const QString& family) {
    if (m_iconFontFamily == family)
        return;
    m_iconFontFamily = family;
    update();
    emit chevronChanged();
}

void ComboBox::setChevronSize(int size) {
    if (m_chevronSize == size)
        return;
    m_chevronSize = size;
    update();
    emit chevronChanged();
}

void ComboBox::setChevronOffset(const QPoint& offset) {
    if (m_chevronOffset == offset)
        return;
    m_chevronOffset = offset;
    update();
    emit chevronChanged();
}

void ComboBox::setPressProgress(qreal value) {
    qreal clamped = std::clamp(value, 0.0, 1.0);
    if (qFuzzyCompare(m_pressProgress, clamped))
        return;
    m_pressProgress = clamped;
    update();
}

void ComboBox::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    const auto& colors = themeColors();
    const auto& radius = themeRadius();

    QRectF bgRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    // 视觉风格尽量贴近 WinUI 3 文本输入控件
    QColor bgColor, borderColor, bottomBorderColor;
    int bottomBorderWidth = 1;

    if (!isEnabled()) {
        bgColor = colors.controlDisabled;
        borderColor = colors.strokeDivider;
        bottomBorderColor = borderColor;
    } else if (m_isPressed) {
        bgColor = colors.controlTertiary;
        borderColor = colors.strokeSecondary;
        bottomBorderColor = colors.strokeSecondary;
    } else if (hasFocus()) {
        bgColor = (currentTheme() == Dark) ? colors.bgSolid : colors.controlDefault;
        borderColor = colors.strokeSecondary;
        bottomBorderColor = colors.accentDefault;
        bottomBorderWidth = 2;
    } else if (m_isHovered) {
        bgColor = colors.controlSecondary;
        borderColor = colors.strokeSecondary;
        bottomBorderColor = colors.strokeSecondary;
    } else {
        bgColor = colors.controlDefault;
        borderColor = colors.strokeDefault;
        bottomBorderColor = colors.strokeDivider;
    }

    // 背景 + 边框
    qreal r = radius.control;
    QPainterPath framePath;
    framePath.addRoundedRect(bgRect, r, r);

    p.setPen(Qt::NoPen);
    p.setBrush(bgColor);
    p.drawPath(framePath);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(borderColor, 1));
    p.drawPath(framePath);

    // 底部高亮线（聚焦时使用 Accent，与 WinUI 3 风格保持一致）
    if (isEnabled()) {
        QPen pen(bottomBorderColor, bottomBorderWidth);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);

        QPainterPath bottomPath;
        qreal bottomY = bgRect.bottom() - (bottomBorderWidth > 1 ? (bottomBorderWidth - 1) / 2.0 : 0);
        bottomPath.moveTo(bgRect.left() + radius.control, bottomY);
        bottomPath.lineTo(bgRect.right() - radius.control, bottomY);
        p.drawPath(bottomPath);
    }

    // 内容区域（去掉左右 padding 和箭头区域）
    const int paddingH = m_contentPaddingH;
    const int arrowWidth = m_arrowWidth;
    QRect textRect = rect().adjusted(paddingH,
                                     0,
                                     -paddingH - arrowWidth,
                                     0);

    QString displayText = currentText();
    QColor textColor = isEnabled() ? colors.textPrimary : colors.textDisabled;
    if (displayText.isEmpty()) {
        displayText = placeholderText();
        textColor = colors.textSecondary;
    }

    p.setPen(textColor);
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, displayText);

    // 右侧 ChevronDown 图标（与 DropDownButton 一致：字体、矩形、按压动画、对齐方式）
    QFont iconFont(m_iconFontFamily);
    iconFont.setPixelSize(m_chevronSize);
    p.setFont(iconFont);
    QColor arrowColor = isEnabled() ? colors.textSecondary : colors.textDisabled;
    if (isEnabled() && m_pressProgress > 0.0) {
        qreal alphaFactor = 1.0 - 0.5 * m_pressProgress;
        arrowColor.setAlpha(static_cast<int>(255 * alphaFactor));
    }
    p.setPen(arrowColor);
    QRect chevronRect = rect().adjusted(0, 0, -m_chevronOffset.x(), 0);
    const qreal maxOffset = 3.0;
    qreal pressOffset = maxOffset * qSin(m_pressProgress * M_PI);
    chevronRect.translate(0, static_cast<int>(pressOffset) + m_chevronOffset.y());
    p.drawText(chevronRect, Qt::AlignRight | Qt::AlignVCenter, m_chevronGlyph);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ComboBox::enterEvent(QEnterEvent* event) {
    m_isHovered = true;
    QComboBox::enterEvent(event);
    update();
}
#else
void ComboBox::enterEvent(QEvent* event) {
    m_isHovered = true;
    QComboBox::enterEvent(event);
    update();
}
#endif

void ComboBox::leaveEvent(QEvent* event) {
    m_isHovered = false;
    QComboBox::leaveEvent(event);
    update();
}

void ComboBox::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPressed = true;
        if (m_pressAnimation) {
            m_pressAnimation->stop();
            m_pressAnimation->setStartValue(0.0);
            m_pressAnimation->setEndValue(1.0);
            m_pressAnimation->start();
        }
        update();
    }
    QComboBox::mousePressEvent(event);
}

void ComboBox::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPressed = false;
        update();
    }
    QComboBox::mouseReleaseEvent(event);
}

void ComboBox::focusInEvent(QFocusEvent* event) {
    QComboBox::focusInEvent(event);
    update();
}

void ComboBox::focusOutEvent(QFocusEvent* event) {
    QComboBox::focusOutEvent(event);
    update();
}

} // namespace view::basicinput

