#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QColor>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::textfields { class TextBlock; }

class QLineEdit;

namespace view::basicinput {

class Slider;

/**
 * @brief ColorPicker - WinUI 3 风格的基础颜色选择器（简化版）
 *
 * 提供 RGB + Alpha 调整、预览矩形和 Hex 文本输入。
 * 视觉样式与 Fluent 设计 Token 对齐。
 */
class ColorPicker : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT

    using TextBlock = view::textfields::TextBlock;

    /** @brief 当前颜色 */
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /** @brief 是否启用 Alpha 通道 */
    Q_PROPERTY(bool alphaEnabled READ alphaEnabled WRITE setAlphaEnabled NOTIFY alphaEnabledChanged)

public:
    explicit ColorPicker(QWidget* parent = nullptr);

    QColor color() const { return m_color; }
    void setColor(const QColor& c);

    bool alphaEnabled() const { return m_alphaEnabled; }
    void setAlphaEnabled(bool enabled);

    // 提供给内部子控件访问的 HSV 信息
    qreal hue() const { return m_h; }          // 0.0 - 1.0
    qreal saturation() const { return m_s; }   // 0.0 - 1.0
    qreal value() const { return m_v; }        // 0.0 - 1.0

    void onThemeUpdated() override;

signals:
    void colorChanged(const QColor& color);
    void alphaEnabledChanged(bool enabled);

private slots:
    void handleChannelEdited();
    void handleHexEdited();

public: // 供内部子控件调用
    // 从色相条/色谱更新 HSV
    void setHueFromBar(qreal h);          // 0.0 - 1.0
    void setSVFromSpectrum(qreal s, qreal v); // 0.0 - 1.0

private:
    void initUi();
    void initSpectrumUi(QVBoxLayout* parentLayout);
    void updateFromColor();
    QString colorToHex(const QColor& c, bool withAlpha) const;
    QColor hexToColor(const QString& text, bool* ok) const;

    QColor m_color;
    bool m_alphaEnabled = true;

    // HSV 分量（与 m_color 保持同步）
    qreal m_h = 0.0;
    qreal m_s = 0.0;
    qreal m_v = 1.0;

    QWidget* m_spectrum = nullptr;
    QWidget* m_hueBar = nullptr;

    QLineEdit* m_rEdit = nullptr;
    QLineEdit* m_gEdit = nullptr;
    QLineEdit* m_bEdit = nullptr;
    QLineEdit* m_aEdit = nullptr;

    QLineEdit* m_hexEdit = nullptr;
    TextBlock* m_hexLabel = nullptr;

    bool m_isInternalUpdate = false;
};

} // namespace view::basicinput

#endif // COLORPICKER_H

