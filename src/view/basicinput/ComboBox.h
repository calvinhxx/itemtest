#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <QPoint>
#include <QPointer>
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
 * 视觉规范来源：
 * - Windows UI Kit (Community) Figma：ComboBox 组件
 *   圆角 4px、水平内边距 11px / 垂直 4px、正文 Body 14/20、Chevron 12px
 * - WinUI Gallery：microsoft/WinUI-Gallery — ComboBoxPage.xaml
 *
 * 行为沿用 QComboBox；关闭系统原生绘制（setFrame(false)），由 paintEvent 绘制
 * Rest / Hover / Pressed / Focus / Disabled 五态。下拉视图默认为 view::collections::ListView。
 *
 * Flyout 弹层：OverlayCornerRadius(8px)、多层柔和阴影、stroke 描边；展开后将当前选中行
 * 垂直对齐到 ComboBox 中心。
 */
class ComboBox : public QComboBox, public FluentElement, public view::QMLPlus {
    Q_OBJECT

    /** @brief 文本使用的主题字体 Token 名称，例如 "Body"、"Subtitle" */
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    /** @brief 左右内容内边距（不含箭头区域），默认 Spacing::Padding::ComboBoxHorizontal */
    Q_PROPERTY(int contentPaddingH READ contentPaddingH WRITE setContentPaddingH NOTIFY contentPaddingChanged)
    /** @brief 右侧箭头区域宽度，默认 24px */
    Q_PROPERTY(int arrowWidth READ arrowWidth WRITE setArrowWidth NOTIFY arrowWidthChanged)
    /** @brief 下拉图标的字符编码（默认 ChevronDown） */
    Q_PROPERTY(QString chevronGlyph READ chevronGlyph WRITE setChevronGlyph NOTIFY chevronChanged)
    /** @brief 下拉图标字号（默认 Caption 12px） */
    Q_PROPERTY(int chevronSize READ chevronSize WRITE setChevronSize NOTIFY chevronChanged)
    /** @brief 下拉图标相对默认位置的偏移量 */
    Q_PROPERTY(QPoint chevronOffset READ chevronOffset WRITE setChevronOffset NOTIFY chevronChanged)
    /** @brief 点击/展开时 Chevron 按压动画进度 [0,1] */
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
    int chevronSize() const { return m_chevronSize; }
    void setChevronSize(int size);
    QPoint chevronOffset() const { return m_chevronOffset; }
    void setChevronOffset(const QPoint& offset);

    qreal pressProgress() const { return m_pressProgress; }
    void setPressProgress(qreal value);

    /** 与 QComboBox::setEditable 相同，并在创建 lineEdit 后同步 Fluent 线框样式 */
    void setEditable(bool editable);

signals:
    void fontRoleChanged();
    void contentPaddingChanged();
    void arrowWidthChanged();
    void chevronChanged();

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

    QPointer<QWidget> m_popupChrome;

    bool m_isHovered = false;
    bool m_isPressed = false;

    QString m_fontRole = "Body";
    int m_contentPaddingH = ::Spacing::Padding::ComboBoxHorizontal;
    int m_arrowWidth = 24;
    QString m_chevronGlyph = Typography::Icons::ChevronDown;
    int m_chevronSize = Typography::FontSize::Caption;
    QPoint m_chevronOffset;

    qreal m_pressProgress = 0.0;
    QPropertyAnimation* m_pressAnimation = nullptr;
};

} // namespace view::basicinput

#endif // COMBOBOX_H

