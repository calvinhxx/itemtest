#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <QPointer>
#include <QPoint>
#include <QString>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"
#include "common/Typography.h"

class QPropertyAnimation;

namespace view::basicinput {

/**
 * @brief ComboBox — Fluent / WinUI 3 风格下拉框（Qt 自绘 + 设计 Token）
 *
 * 设计对齐（实现时以项目内 FluentElement Token 为准）：
 * - [Windows UI Kit (Community) — Figma](https://www.figma.com/design/urqb6Xq3nerP5OrnbUAxGJ/Windows-UI-kit--Community-)：
 *   ComboBox 组件：圆角 4px、内容区水平约 11px / 垂直约 4px、正文 Body 14/20、Chevron 12px。
 * - [WinUI Gallery — ComboBox 页](https://github.com/microsoft/WinUI-Gallery)：
 *   `ComboBoxPage.xaml`：Header + PlaceholderText、固定宽度示例、可编辑 `IsEditable` 字号列表等。
 *
 * 行为沿用 QComboBox；关闭系统原生绘制（setFrame(false)），由 paintEvent 绘制 Rest / Hover / Pressed /
 * Focus / Disabled。下拉视图默认为 view::collections::ListView；列表行样式由业务设置 ItemDelegate。
 *
 * 下拉浮层（Flyout）视觉对齐 WinUI / view::dialogs_flyouts::Dialog：Overlay 圆角（8px）、多层柔和阴影、
 * stroke 描边；展开后将当前选中行垂直对齐到 ComboBox 中心（类似 WinUI Gallery 字号下拉）。
 */
class ComboBox : public QComboBox, public FluentElement, public view::QMLPlus {
    Q_OBJECT

    /** @brief 文本使用的主题字体 Token 名称，例如 \"Body\"、\"Subtitle\" */
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    /** @brief 左右内容内边距（不含箭头区域），默认 Spacing::Padding::ComboBoxHorizontal（WinUI Kit ComboBox） */
    Q_PROPERTY(int contentPaddingH READ contentPaddingH WRITE setContentPaddingH NOTIFY contentPaddingChanged)
    /** @brief 右侧箭头区域宽度，默认 24px */
    Q_PROPERTY(int arrowWidth READ arrowWidth WRITE setArrowWidth NOTIFY arrowWidthChanged)
    /** @brief 下拉图标的字符编码（默认 ChevronDown），与 DropDownButton 一致 */
    Q_PROPERTY(QString chevronGlyph READ chevronGlyph WRITE setChevronGlyph NOTIFY chevronChanged)
    /** @brief 图标字体家族名称（默认 Segoe Fluent Icons） */
    Q_PROPERTY(QString iconFontFamily READ iconFontFamily WRITE setIconFontFamily NOTIFY chevronChanged)
    /** @brief 下拉图标字号（默认 Caption） */
    Q_PROPERTY(int chevronSize READ chevronSize WRITE setChevronSize NOTIFY chevronChanged)
    /** @brief 下拉图标偏移：x 为与右侧边缘间距，y 为垂直微调，与 DropDownButton 一致 */
    Q_PROPERTY(QPoint chevronOffset READ chevronOffset WRITE setChevronOffset NOTIFY chevronChanged)
    /** @brief 点击/展开时 Chevron 按压动画进度 [0,1]，与 DropDownButton 一致 */
    Q_PROPERTY(qreal pressProgress READ pressProgress WRITE setPressProgress)

public:
    explicit ComboBox(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    int contentPaddingH() const { return m_contentPaddingH; }
    void setContentPaddingH(int padding);

    int arrowWidth() const { return m_arrowWidth; }
    void setArrowWidth(int w);

    QString chevronGlyph() const { return m_chevronGlyph; }
    void setChevronGlyph(const QString& glyph);
    QString iconFontFamily() const { return m_iconFontFamily; }
    void setIconFontFamily(const QString& family);
    int chevronSize() const { return m_chevronSize; }
    void setChevronSize(int size);
    QPoint chevronOffset() const { return m_chevronOffset; }
    void setChevronOffset(const QPoint& offset);

    qreal pressProgress() const { return m_pressProgress; }
    void setPressProgress(qreal value);

    /** 与 QComboBox::setEditable 相同，并在创建 lineEdit 后同步 Fluent 线框样式 */
    void setEditable(bool editable);

protected:
    void paintEvent(QPaintEvent* event) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

    void showPopup() override;
    void hidePopup() override;

private:
    void initAnimation();
    void syncLineEditFromTheme();
    void polishFluentComboPopup();

    /** 浮层阴影边距，与 Dialog::m_shadowSize（Spacing::Standard）一致 */
    QPointer<QWidget> m_popupChrome;

    bool m_isHovered = false;
    bool m_isPressed = false;
    QString m_fontRole = "Body";
    int m_contentPaddingH = ::Spacing::Padding::ComboBoxHorizontal;
    int m_arrowWidth = 24;
    QString m_chevronGlyph = Typography::Icons::ChevronDown;
    QString m_iconFontFamily = Typography::FontFamily::SegoeFluentIcons;
    int m_chevronSize = Typography::FontSize::Caption;
    QPoint m_chevronOffset{::Spacing::Padding::ControlHorizontal, 0};
    qreal m_pressProgress = 0.0;
    QPropertyAnimation* m_pressAnimation = nullptr;

signals:
    void fontRoleChanged();
    void contentPaddingChanged();
    void arrowWidthChanged();
    void chevronChanged();
};

} // namespace view::basicinput

#endif // COMBOBOX_H

