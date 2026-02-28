#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QColor>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::textfields { class TextBlock; class TextBox; }

namespace view::basicinput {

class Slider;

/**
 * @brief ColorPicker - WinUI 3 风格的颜色选择器（参考 Gallery）
 *
 * 布局：色谱 | 色相条 | 右侧预览 pane（棋盘格 + 透明）
 * 下方：Value 滑块（明度 0–100）、Alpha 滑块（0–255）
 * 底部：Hex、Red、Green、Blue、Alpha 文本输入
 */
class ColorPicker : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT

    using TextBlock = view::textfields::TextBlock;
    using TextBox   = view::textfields::TextBox;

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
    void setValueFromSlider(int percent);   // 0-100 → m_v
    void setAlphaFromSlider(int alpha);     // 0-255

private:
    void initUi();
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
    QWidget* m_previewPane = nullptr;   // 右侧实时颜色预览（棋盘格 + 透明）
    Slider* m_valueSlider = nullptr;    // 明度 (V) 0-100
    Slider* m_alphaSlider = nullptr;    // Alpha 0-255
    QWidget* m_alphaRowWidget = nullptr;     // Alpha Slider 行容器
    QWidget* m_alphaInputRowWidget = nullptr; // Alpha 输入行容器

    TextBox* m_rEdit = nullptr;
    TextBox* m_gEdit = nullptr;
    TextBox* m_bEdit = nullptr;
    TextBox* m_aEdit = nullptr;

    TextBox* m_hexEdit = nullptr;

    bool m_isInternalUpdate = false;
};

} // namespace view::basicinput

#endif // COLORPICKER_H

