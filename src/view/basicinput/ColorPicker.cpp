#include "ColorPicker.h"

#include <QSlider>
#include <QLineEdit>
#include <QLabel>
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

    void mousePressEvent(QMouseEvent* e) override { handlePos(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) handlePos(e->pos());
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

    void mousePressEvent(QMouseEvent* e) override { handlePos(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) handlePos(e->pos());
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

    // 预览矩形
    m_preview = new QWidget(this);
    m_preview->setFixedHeight(spacing.standard);
    m_preview->setAutoFillBackground(true);

    mainLayout->addWidget(new QLabel("Preview:", this));
    mainLayout->addWidget(m_preview);

    // RGB / A 滑块
    auto createChannelRow = [&](const QString& name, QSlider*& slider) {
        auto* row = new QHBoxLayout();
        row->setSpacing(spacing.gap.tight);

        auto* label = new QLabel(name, this);
        label->setFixedWidth(40);
        row->addWidget(label);

        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 255);
        slider->setSingleStep(1);
        row->addWidget(slider, 1);

        auto* valueLabel = new QLabel("0", this);
        valueLabel->setFixedWidth(32);
        row->addWidget(valueLabel);

        connect(slider, &QSlider::valueChanged, valueLabel,
                [valueLabel](int v) { valueLabel->setText(QString::number(v)); });
        connect(slider, &QSlider::valueChanged, this, &ColorPicker::handleSliderChanged);

        mainLayout->addLayout(row);
    };

    createChannelRow("R", m_rSlider);
    createChannelRow("G", m_gSlider);
    createChannelRow("B", m_bSlider);
    createChannelRow("A", m_aSlider);

    // Hex 输入
    auto* hexRow = new QHBoxLayout();
    hexRow->setSpacing(spacing.gap.tight);
    m_hexLabel = new QLabel("Hex:", this);
    m_hexEdit = new QLineEdit(this);
    m_hexEdit->setMaxLength(9); // #RRGGBB or #RRGGBBAA
    hexRow->addWidget(m_hexLabel);
    hexRow->addWidget(m_hexEdit, 1);
    mainLayout->addLayout(hexRow);

    connect(m_hexEdit, &QLineEdit::editingFinished, this, &ColorPicker::handleHexEdited);

    onThemeUpdated();
}

void ColorPicker::initSpectrumUi(QVBoxLayout* parentLayout) {
    const auto& spacing = themeSpacing();

    auto* row = new QHBoxLayout();
    row->setSpacing(spacing.gap.normal);

    m_spectrum = new ColorSpectrumWidget(this, this);
    m_spectrum->setMinimumSize(240, 180);
    m_spectrum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_hueBar = new HueBarWidget(this, this);
    m_hueBar->setFixedWidth(24);

    row->addWidget(m_spectrum, 1);
    row->addWidget(m_hueBar);

    parentLayout->addWidget(new QLabel("Color spectrum:", this));
    parentLayout->addLayout(row);
}

void ColorPicker::onThemeUpdated() {
    const auto& colors = themeColors();
    const auto& fs = themeFont("Body");

    QPalette pal = m_preview->palette();
    pal.setColor(QPalette::Window, m_color);
    pal.setColor(QPalette::WindowText, colors.textPrimary);
    m_preview->setPalette(pal);

    setFont(fs.toQFont());
    updateFromColor();
}

void ColorPicker::setColor(const QColor& c) {
    if (c == m_color)
        return;
    m_color = c;

    // 同步 HSV 分量
    float h, s, v, a;
    c.getHsvF(&h, &s, &v, &a);
    if (h < 0.0) {
        // 灰度颜色：保留现有色相
        h = m_h;
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
    if (m_aSlider) {
        m_aSlider->setVisible(enabled);
    }
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

void ColorPicker::handleSliderChanged() {
    if (!m_rSlider || !m_gSlider || !m_bSlider || !m_aSlider)
        return;
    QColor c(m_rSlider->value(), m_gSlider->value(), m_bSlider->value(),
             m_alphaEnabled ? m_aSlider->value() : 255);
    setColor(c);
}

void ColorPicker::handleHexEdited() {
    bool ok = false;
    QColor c = hexToColor(m_hexEdit->text().trimmed(), &ok);
    if (ok) {
        setColor(c);
    } else {
        // 回退到当前颜色的 Hex
        updateFromColor();
    }
}

void ColorPicker::updateFromColor(bool) {
    if (m_rSlider && m_gSlider && m_bSlider && m_aSlider) {
        m_rSlider->blockSignals(true);
        m_gSlider->blockSignals(true);
        m_bSlider->blockSignals(true);
        m_aSlider->blockSignals(true);

        m_rSlider->setValue(m_color.red());
        m_gSlider->setValue(m_color.green());
        m_bSlider->setValue(m_color.blue());
        m_aSlider->setValue(m_color.alpha());

        m_rSlider->blockSignals(false);
        m_gSlider->blockSignals(false);
        m_bSlider->blockSignals(false);
        m_aSlider->blockSignals(false);
    }

    // 更新预览
    QPalette pal = m_preview->palette();
    pal.setColor(QPalette::Window, m_color);
    m_preview->setPalette(pal);

    // 更新 Hex
    if (m_hexEdit) {
        m_hexEdit->setText(colorToHex(m_color, m_alphaEnabled));
    }

    if (m_spectrum) m_spectrum->update();
    if (m_hueBar) m_hueBar->update();
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

