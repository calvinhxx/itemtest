#ifndef DROPDOWNBUTTON_H
#define DROPDOWNBUTTON_H

#include "Button.h"
#include <QMenu>
#include "common/Typography.h"
#include "common/Spacing.h"

class QPropertyAnimation;

namespace view::basicinput {

/**
 * @brief DropDownButton - 带有下拉菜单的现代按钮
 * 
 * 参考 WinUI 3 规范，在按钮尾部自动添加 ChevronDown 图标。
 */
class DropDownButton : public Button {
    Q_OBJECT
    /** @brief 菜单是否处于打开状态 */
    Q_PROPERTY(bool isOpen READ isOpen WRITE setOpen NOTIFY openChanged)
    /** @brief 下拉图标的字符编码 (默认 E70D - ChevronDown) */
    Q_PROPERTY(QString chevronGlyph READ chevronGlyph WRITE setChevronGlyph NOTIFY chevronChanged)
    /** @brief 图标字体家族名称 (默认 Segoe Fluent Icons) */
    Q_PROPERTY(QString iconFontFamily READ iconFontFamily WRITE setIconFontFamily NOTIFY chevronChanged)
    /** @brief 下拉图标的大小 (默认 12) */
    Q_PROPERTY(int chevronSize READ chevronSize WRITE setChevronSize NOTIFY chevronChanged)
    /** @brief 下拉图标距离右侧的边距 (默认 themeSpacing().padding.controlH) */
    Q_PROPERTY(int chevronPadding READ chevronPadding WRITE setChevronPadding NOTIFY chevronChanged)
    /** @brief 下拉图标在垂直方向上的微调偏移量（像素，向下为正） */
    Q_PROPERTY(int chevronOffset READ chevronOffset WRITE setChevronOffset NOTIFY chevronChanged)
    /** @brief 点击/展开动画进度 [0,1]，用于驱动 icon 大小与颜色的细微过渡 */
    Q_PROPERTY(qreal pressProgress READ pressProgress WRITE setPressProgress)

public:
    explicit DropDownButton(const QString& text, QWidget* parent = nullptr);
    explicit DropDownButton(QWidget* parent = nullptr);

    void setMenu(QMenu* menu);
    QMenu* menu() const { return m_menu; }

    bool isOpen() const { return m_isOpen; }
    void setOpen(bool open);

    QString chevronGlyph() const { return m_chevronGlyph; }
    void setChevronGlyph(const QString& glyph);

    QString iconFontFamily() const { return m_iconFontFamily; }
    void setIconFontFamily(const QString& family);

    int chevronSize() const { return m_chevronSize; }
    void setChevronSize(int size);

    int chevronPadding() const { return m_chevronPadding; }
    void setChevronPadding(int padding);

    int chevronOffset() const { return m_chevronOffset; }
    void setChevronOffset(int offset);

    qreal pressProgress() const { return m_pressProgress; }
    void setPressProgress(qreal value);

signals:
    void openChanged();
    void chevronChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void initAnimation();

    QMenu* m_menu = nullptr;
    bool m_isOpen = false;
    QString m_chevronGlyph = ::Typography::Icons::ChevronDown;
    QString m_iconFontFamily = ::Typography::FontFamily::SegoeFluentIcons;
    int m_chevronSize = ::Typography::FontSize::Caption;
    int m_chevronPadding = ::Spacing::Padding::ControlHorizontal;
    int m_chevronOffset = 0;                     // 垂直方向像素偏移，解决某些 icon 与文本不完全对齐的问题
    qreal m_pressProgress = 0.0;              // 0 = 静止, 1 = 点击/展开高亮
    QPropertyAnimation* m_pressAnimation = nullptr;
};

} // namespace view::basicinput

#endif // DROPDOWNBUTTON_H
