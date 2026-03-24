#include "ComboBox.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QGuiApplication>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>
#include <QEasingCurve>
#include <QtMath>

#include "common/Elevation.h"
#include "common/Spacing.h"
#include "common/Typography.h"
#include "view/collections/ListView.h"

namespace {

/** 与 view::dialogs_flyouts::Dialog::drawShadow 相同思路：多层圆角扩散模拟 Flyout 阴影 */
static void drawFlyoutShadow(QPainter& painter, const QRect& contentRect, int overlayRadius,
                             const Elevation::ShadowParams& s) {
    constexpr int layers = 10;
    constexpr int spreadStep = 1;
    for (int i = 0; i < layers; ++i) {
        const double ratio = 1.0 - static_cast<double>(i) / layers;
        QColor sc = s.color;
        sc.setAlphaF(s.opacity * ratio * 0.35);

        painter.setPen(Qt::NoPen);
        painter.setBrush(sc);

        const int spread = i * spreadStep;
        constexpr int offsetY = 2;
        painter.drawRoundedRect(
            contentRect.adjusted(-spread, -spread, spread, spread).translated(0, offsetY),
            overlayRadius + spread, overlayRadius + spread);
    }
}

/**
 * 置于 QComboBox 弹窗最底层：绘制 Dialog 风格阴影 + Overlay 圆角底板（鼠标穿透）。
 */
class ComboPopupChrome : public QWidget {
public:
    explicit ComboPopupChrome(view::basicinput::ComboBox* host, QWidget* parent = nullptr)
        : QWidget(parent), m_host(host) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        if (!m_host)
            return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const int m = ::Spacing::Standard;
        QRect contentRect = rect().adjusted(m, m, -m, -m);
        const auto& radius = m_host->themeRadius();
        const auto& s = m_host->themeShadow(Elevation::High);
        drawFlyoutShadow(painter, contentRect, radius.overlay, s);

        const auto& colors = m_host->themeColors();
        painter.setBrush(colors.bgLayer);
        painter.setPen(colors.strokeDefault);
        const QRectF crf = QRectF(contentRect).adjusted(0.5, 0.5, -0.5, -0.5);
        painter.drawRoundedRect(crf, radius.overlay, radius.overlay);
    }

private:
    view::basicinput::ComboBox* m_host;
};

} // namespace

namespace view::basicinput {

ComboBox::ComboBox(QWidget* parent)
    : QComboBox(parent) {
    setAttribute(Qt::WA_Hover);
    setEditable(false);
    setFrame(false);
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    // WinUI / Figma：标准行高 32px（Spacing::ControlHeight::Standard）
    setMinimumHeight(::Spacing::ControlHeight::Standard);
    setView(new view::collections::ListView(this));
    initAnimation();
    onThemeUpdated();
}

void ComboBox::initAnimation() {
    m_pressAnimation = new QPropertyAnimation(this, "pressProgress");
    m_pressAnimation->setDuration(themeAnimation().slow);
    m_pressAnimation->setEasingCurve(themeAnimation().decelerate);
    m_pressAnimation->setStartValue(0.0);
    m_pressAnimation->setEndValue(1.0);
}

void ComboBox::syncLineEditFromTheme() {
    if (!isEditable())
        return;
    auto* le = lineEdit();
    if (!le)
        return;

    const auto& fs = themeFont(m_fontRole);
    le->setFrame(false);
    le->setFont(fs.toQFont());
    QPalette pal = le->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Window, Qt::transparent);
    le->setPalette(pal);
    le->setAutoFillBackground(false);
    le->setAttribute(Qt::WA_TranslucentBackground);
    le->setStyleSheet(QStringLiteral("QLineEdit { background: transparent; border: none; }"));
    const int v = ::Spacing::Padding::ComboBoxVertical;
    le->setTextMargins(m_contentPaddingH, v, m_contentPaddingH, v);
}

void ComboBox::onThemeUpdated() {
    const auto& fs = themeFont(m_fontRole);
    setFont(fs.toQFont());
    if (auto* v = view()) {
        v->setFont(fs.toQFont());
    }
    syncLineEditFromTheme();
    if (m_popupChrome)
        m_popupChrome->update();
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
    syncLineEditFromTheme();
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

void ComboBox::setEditable(bool editable) {
    if (isEditable() == editable)
        return;
    QComboBox::setEditable(editable);
    syncLineEditFromTheme();
    update();
}

void ComboBox::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

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

    // 内容区：WinUI Kit ComboBox 水平/垂直内边距（Figma Base: px-11 py-4）
    const int paddingH = m_contentPaddingH;
    const int paddingV = ::Spacing::Padding::ComboBoxVertical;
    const int arrowWidth = m_arrowWidth;
    QRect textRect = rect().adjusted(paddingH,
                                     paddingV,
                                     -paddingH - arrowWidth,
                                     -paddingV);

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
    QRect chevronRect = rect().adjusted(0, paddingV, -m_chevronOffset.x(), -paddingV);
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

void ComboBox::showPopup() {
    QComboBox::showPopup();
    QTimer::singleShot(0, this, [this]() { polishFluentComboPopup(); });
}

void ComboBox::hidePopup() {
    QComboBox::hidePopup();
    m_popupChrome.clear();
}

void ComboBox::polishFluentComboPopup() {
    QAbstractItemView* v = view();
    if (!v)
        return;
    QWidget* pop = v->window();
    if (!pop || pop == window())
        return;

    const int M = ::Spacing::Standard;

    if (m_popupChrome) {
        m_popupChrome->deleteLater();
        m_popupChrome = nullptr;
    }

    pop->setAttribute(Qt::WA_TranslucentBackground);
    pop->setAutoFillBackground(false);

    const QRect g = pop->geometry();
    pop->setGeometry(QRect(g.x() - M, g.y() - M, g.width() + 2 * M, g.height() + 2 * M));

    const QObjectList ch = pop->children();
    for (QObject* o : ch) {
        auto* w = qobject_cast<QWidget*>(o);
        if (!w)
            continue;
        w->setGeometry(w->geometry().translated(M, M));
    }

    auto* chrome = new ComboPopupChrome(this, pop);
    chrome->setGeometry(pop->rect());
    chrome->lower();
    chrome->show();
    m_popupChrome = chrome;

    // WinUI Gallery：当前选中行垂直中心与 ComboBox 垂直中心对齐（浮层在上下方向“包裹”当前项）
    const int idx = currentIndex();
    const QAbstractItemModel* mdl = model();
    if (mdl && idx >= 0 && idx < mdl->rowCount()) {
        const QModelIndex mi = mdl->index(idx, 0);
        QRect vr = v->visualRect(mi);
        if (!vr.isEmpty()) {
            const int comboMidY = mapToGlobal(QPoint(0, height() / 2)).y();
            const int rowCenterY = v->mapToGlobal(vr.center()).y();
            const int delta = comboMidY - rowCenterY;
            if (delta != 0)
                pop->move(pop->x(), pop->y() + delta);
        }
    }

    if (QScreen* scr = QGuiApplication::screenAt(pop->frameGeometry().center())) {
        const QRect ag = scr->availableGeometry();
        // 逐轴夹紧，每次 move 后重新取 frameGeometry()
        auto clampAxis = [&](bool condition, int dx, int dy) {
            if (condition) pop->move(pop->x() + dx, pop->y() + dy);
        };
        QRect fr = pop->frameGeometry();
        clampAxis(fr.top() < ag.top(),     0, ag.top() - fr.top());
        fr = pop->frameGeometry();
        clampAxis(fr.bottom() > ag.bottom(), 0, ag.bottom() - fr.bottom());
        fr = pop->frameGeometry();
        clampAxis(fr.left() < ag.left(),   ag.left() - fr.left(), 0);
        fr = pop->frameGeometry();
        clampAxis(fr.right() > ag.right(), ag.right() - fr.right(), 0);
    }

    chrome->update();

    // 弹层显示后 Qt/macOS 常把 QAbstractScrollArea 内置条再显示出来，强制回到 Fluent 条
    if (auto* lv = qobject_cast<view::collections::ListView*>(v)) {
        lv->refreshFluentScrollChrome();
        QTimer::singleShot(0, lv, [lv]() { lv->refreshFluentScrollChrome(); });
    }
}

} // namespace view::basicinput

