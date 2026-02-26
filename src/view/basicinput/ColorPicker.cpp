#include "ColorPicker.h"

#include "view/textfields/TextBlock.h"
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QMouseEvent>
#include <QPainter>

namespace view::basicinput {

// 内部辅助控件：色谱和色相条 -------------------------------

class ColorSpectrumWidget : public QWidget {
public:
    explicit ColorSpectrumWidget(ColorPicker* picker, QWidget* parent = nullptr)
        : QWidget(parent), m_picker(picker) {
        setMouseTracking(true);
        setCursor(Qt::CrossCursor);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        if (!m_picker) return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QRectF r = rect().adjusted(1, 1, -1, -1);

        // 基础：从白色到当前色相的水平渐变（S）
        QColor hueColor = QColor::fromHsvF(m_picker->hue(), 1.0, 1.0);
        QLinearGradient gradH(r.topLeft(), r.topRight());
        gradH.setColorAt(0.0, Qt::white);
        gradH.setColorAt(1.0, hueColor);
        p.fillRect(r, gradH);

        // 叠加：从透明到黑色的垂直渐变（V）
        QLinearGradient gradV(r.topLeft(), r.bottomLeft());
        gradV.setColorAt(0.0, QColor(0, 0, 0, 0));
        gradV.setColorAt(1.0, QColor(0, 0, 0, 255));
        p.fillRect(r, gradV);

        // 绘制当前选择位置
        qreal sx = m_picker->saturation();
        qreal sv = 1.0 - m_picker->value();
        QPointF pos(r.left() + sx * r.width(),
                    r.top() + sv * r.height());
        p.setPen(QPen(Qt::white, 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(pos, 6, 6);
    }

    void mousePressEvent(QMouseEvent* e) override {
        setCursor(Qt::BlankCursor);
        handlePos(e->pos());
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) handlePos(e->pos());
    }
    void mouseReleaseEvent(QMouseEvent* e) override {
        setCursor(Qt::CrossCursor);
    }

private:
    void handlePos(const QPoint& pnt) {
        if (!m_picker) return;
        QRectF r = rect().adjusted(1, 1, -1, -1);
        qreal sx = (pnt.x() - r.left()) / r.width();
        qreal sv = (pnt.y() - r.top()) / r.height();
        sx = std::clamp(sx, 0.0, 1.0);
        sv = std::clamp(sv, 0.0, 1.0);
        m_picker->setSVFromSpectrum(sx, 1.0 - sv);
        update();
    }

    ColorPicker* m_picker = nullptr;
};

class HueBarWidget : public QWidget {
public:
    explicit HueBarWidget(ColorPicker* picker, QWidget* parent = nullptr)
        : QWidget(parent), m_picker(picker) {
        setMouseTracking(true);
        setCursor(Qt::ArrowCursor);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QRectF r = rect().adjusted(2, 2, -2, -2);

        QLinearGradient grad(r.topLeft(), r.bottomLeft());
        grad.setColorAt(0.0, QColor::fromHsvF(0.0, 1.0, 1.0));   // red
        grad.setColorAt(1.0/6.0, QColor::fromHsvF(1.0/6.0, 1.0, 1.0)); // yellow
        grad.setColorAt(2.0/6.0, QColor::fromHsvF(2.0/6.0, 1.0, 1.0)); // green
        grad.setColorAt(3.0/6.0, QColor::fromHsvF(3.0/6.0, 1.0, 1.0)); // cyan
        grad.setColorAt(4.0/6.0, QColor::fromHsvF(4.0/6.0, 1.0, 1.0)); // blue
        grad.setColorAt(5.0/6.0, QColor::fromHsvF(5.0/6.0, 1.0, 1.0)); // magenta
        grad.setColorAt(1.0, QColor::fromHsvF(1.0, 1.0, 1.0));   // back to red

        p.fillRect(r, grad);

        if (m_picker) {
            qreal h = m_picker->hue();
            qreal y = r.top() + h * r.height();
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(QPointF(r.left(), y), QPointF(r.right(), y));
        }
    }

    void mousePressEvent(QMouseEvent* e) override {
        setCursor(Qt::BlankCursor);
        handlePos(e->pos());
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) handlePos(e->pos());
    }
    void mouseReleaseEvent(QMouseEvent* e) override {
        setCursor(Qt::ArrowCursor);
    }

private:
    void handlePos(const QPoint& pnt) {
        if (!m_picker) return;
        QRectF r = rect().adjusted(2, 2, -2, -2);
        qreal h = (pnt.y() - r.top()) / r.height();
        h = std::clamp(h, 0.0, 1.0);
        m_picker->setHueFromBar(h);
        update();
    }

    ColorPicker* m_picker = nullptr;
};

ColorPicker::ColorPicker(QWidget* parent)
    : QWidget(parent) {
    initUi();
    setColor(QColor(255, 255, 255, 255));
}

void ColorPicker::initUi() {
    auto* mainLayout = new QVBoxLayout(this);
    const auto& spacing = themeSpacing();
    mainLayout->setContentsMargins(spacing.padding.card, spacing.padding.card,
                                   spacing.padding.card, spacing.padding.card);
    mainLayout->setSpacing(spacing.gap.normal);

    // 上半部分：色谱 + 色相条
    initSpectrumUi(mainLayout);

    // Hex 输入
    auto* hexRow = new QHBoxLayout();
    hexRow->setContentsMargins(0, 0, 0, 0);
    hexRow->setSpacing(spacing.gap.tight);
    m_hexLabel = new view::textfields::TextBlock("Hex:", this);
    m_hexEdit = new QLineEdit(this);
    m_hexEdit->setMaxLength(9); // #RRGGBB or #RRGGBBAA
    m_hexEdit->setFixedHeight(spacing.standard); // spacing.standard is usually 32px
    hexRow->addWidget(m_hexLabel);
    hexRow->addWidget(m_hexEdit, 1);
    mainLayout->addLayout(hexRow);

    connect(m_hexEdit, &QLineEdit::editingFinished, this, &ColorPicker::handleHexEdited);

    // RGB / A 文本输入框
    auto createChannelRow = [&](const QString& labelText, QLineEdit*& edit) {
        auto* row = new QHBoxLayout();
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(spacing.gap.tight);

        edit = new QLineEdit(this);
        edit->setValidator(new QIntValidator(0, 255, this));
        edit->setFixedHeight(spacing.standard); // spacing.standard is 32px
        row->addWidget(edit, 1);

        auto* label = new view::textfields::TextBlock(labelText, this);
        label->setFixedWidth(48); // Ensure enough space for "Green"
        row->addWidget(label);

        connect(edit, &QLineEdit::editingFinished, this, &ColorPicker::handleChannelEdited);
        mainLayout->addLayout(row);
    };

    createChannelRow("Red", m_rEdit);
    createChannelRow("Green", m_gEdit);
    createChannelRow("Blue", m_bEdit);
    createChannelRow("Alpha", m_aEdit);

    onThemeUpdated();
}

void ColorPicker::initSpectrumUi(QVBoxLayout* parentLayout) {
    const auto& spacing = themeSpacing();

    auto* row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(spacing.gap.normal);

    m_spectrum = new ColorSpectrumWidget(this, this);
    m_spectrum->setMinimumSize(240, 180);
    m_spectrum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_hueBar = new HueBarWidget(this, this);
    m_hueBar->setFixedWidth(24);

    row->addWidget(m_spectrum, 1);
    row->addWidget(m_hueBar);

    parentLayout->addWidget(new view::textfields::TextBlock("Color spectrum:", this));
    parentLayout->addLayout(row);
}

void ColorPicker::onThemeUpdated() {
    const auto& colors = themeColors();
    const auto& fs = themeFont("Body");
    QFont f = fs.toQFont();

    // 1. 更新可能无法自动处理父级字体更改的内部主题感知子控件
    const auto labels = findChildren<view::textfields::TextBlock*>();
    for (auto* label : labels) {
        label->onThemeUpdated();
    }

    setFont(f);
    
    // 2. 显式更新 LineEdits 确保它们立即应用新的字体/大小
    // 这修复了启动时的布局问题（LineEdits 可能保留默认的字体大小提示）
    if (m_hexEdit) m_hexEdit->setFont(f);
    if (m_rEdit) m_rEdit->setFont(f);
    if (m_gEdit) m_gEdit->setFont(f);
    if (m_bEdit) m_bEdit->setFont(f);
    if (m_aEdit) m_aEdit->setFont(f);

    updateFromColor();
}

void ColorPicker::setColor(const QColor& c) {
    if (c == m_color)
        return;
    m_color = c;

    // 同步 HSV 分量
    float h, s, v, a;
    c.getHsvF(&h, &s, &v, &a);

    // 1. 如果新颜色是灰度 (h < 0)，保持现有色相，避免色相丢失
    if (h < 0.0f) {
        h = m_h;
    }

    // 2. 如果新颜色是黑色 (v ~ 0)，保持现有饱和度，避免面板拖拽到底部时 Handle 跳到左侧 (s=0)
    if (qFuzzyIsNull(v)) {
        s = m_s;
    }

    m_h = h;
    m_s = s;
    m_v = v;

    updateFromColor();
    emit colorChanged(m_color);
}

void ColorPicker::setAlphaEnabled(bool enabled) {
    if (m_alphaEnabled == enabled)
        return;
    m_alphaEnabled = enabled;
    
    // Todo: Implement visibility toggling for Alpha input row if strictly required.
    // For now, we adjust the Hex label support.

    if (m_hexLabel) {
        m_hexLabel->setText(enabled ? "Hex ARGB:" : "Hex RGB:");
    }
    updateFromColor();
    emit alphaEnabledChanged(enabled);
}

void ColorPicker::setHueFromBar(qreal h) {
    m_h = h;
    // 基于当前 HSV 重建颜色
    QColor c = QColor::fromHsvF(m_h, m_s, m_v, m_color.alphaF());
    setColor(c);
    if (m_spectrum) m_spectrum->update();
}

void ColorPicker::setSVFromSpectrum(qreal s, qreal v) {
    m_s = s;
    m_v = v;
    QColor c = QColor::fromHsvF(m_h, m_s, m_v, m_color.alphaF());
    setColor(c);
}

void ColorPicker::handleChannelEdited() {
    if (m_isInternalUpdate) return;

    if (!m_rEdit || !m_gEdit || !m_bEdit || !m_aEdit)
        return;

    int r = m_rEdit->text().toInt();
    int g = m_gEdit->text().toInt();
    int b = m_bEdit->text().toInt();
    int a = m_alphaEnabled ? m_aEdit->text().toInt() : 255;

    // 验证器通常会限制 0-255 的范围，但 QIntValidator 允许中间输入，因此为了安全再次约束
    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);
    a = std::clamp(a, 0, 255);

    QColor c(r, g, b, a);
    setColor(c);
}

void ColorPicker::handleHexEdited() {
    if (m_isInternalUpdate) return; // 内部更新时忽略，保持状态一致

    bool ok = false;
    QColor c = hexToColor(m_hexEdit->text().trimmed(), &ok);
    if (ok) {
        setColor(c);
    } else {
        // 回退到当前颜色的 Hex
        updateFromColor();
    }
}

void ColorPicker::updateFromColor() {
    m_isInternalUpdate = true;
    if (m_rEdit && m_gEdit && m_bEdit && m_aEdit) {
        m_rEdit->setText(QString::number(m_color.red()));
        m_gEdit->setText(QString::number(m_color.green()));
        m_bEdit->setText(QString::number(m_color.blue()));
        m_aEdit->setText(QString::number(m_color.alpha()));
    }

    // 更新 Hex
    if (m_hexEdit) {
        m_hexEdit->setText(colorToHex(m_color, m_alphaEnabled));
    }

    if (m_spectrum) m_spectrum->update();
    if (m_hueBar) m_hueBar->update();
    m_isInternalUpdate = false;
}

QString ColorPicker::colorToHex(const QColor& c, bool withAlpha) const {
    QString hex = QString("#%1%2%3")
                      .arg(c.red(), 2, 16, QLatin1Char('0'))
                      .arg(c.green(), 2, 16, QLatin1Char('0'))
                      .arg(c.blue(), 2, 16, QLatin1Char('0'))
                      .toUpper();
    if (withAlpha) {
        hex += QString("%1").arg(c.alpha(), 2, 16, QLatin1Char('0')).toUpper();
    }
    return hex;
}

QColor ColorPicker::hexToColor(const QString& text, bool* ok) const {
    QString t = text;
    if (t.startsWith('#'))
        t.remove(0, 1);
    bool localOk = false;
    int len = t.length();
    if (len != 6 && len != 8) {
        if (ok) *ok = false;
        return QColor();
    }
    bool okR, okG, okB, okA = true;
    int r = t.mid(0, 2).toInt(&okR, 16);
    int g = t.mid(2, 2).toInt(&okG, 16);
    int b = t.mid(4, 2).toInt(&okB, 16);
    int a = 255;
    if (len == 8) {
        a = t.mid(6, 2).toInt(&okA, 16);
    }
    localOk = okR && okG && okB && okA;
    if (ok) *ok = localOk;
    if (!localOk) return QColor();
    return QColor(r, g, b, a);
}

} // namespace view::basicinput

