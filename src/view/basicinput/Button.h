#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include <QIcon>
#include <QEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QPainter>
#include <QStyleOptionButton>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::basicinput {

/**
 * @brief Button - 通过 paintEvent 完美还原 WinUI 3 视觉规范
 */
class Button : public QPushButton, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    
    /** @brief 按钮风格：Standard(标准), Accent(强调色), Subtle(透明幽灵) */
    Q_PROPERTY(ButtonStyle fluentStyle READ fluentStyle WRITE setFluentStyle NOTIFY fluentStyleChanged)
    /** @brief 按钮密度：Small(24px), StandardSize(32px), Large(40px) 影响高度和内边距 */
    Q_PROPERTY(ButtonSize fluentSize READ fluentSize WRITE setFluentSize NOTIFY fluentSizeChanged)
    /** @brief 内容排列：TextOnly, IconBefore, IconOnly, IconAfter */
    Q_PROPERTY(ButtonLayout fluentLayout READ fluentLayout WRITE setFluentLayout NOTIFY fluentLayoutChanged)
    /** @brief 是否强制显示焦点框（即便没有物理焦点） */
    Q_PROPERTY(bool focusVisual READ hasFocusVisual WRITE setFocusVisual NOTIFY focusVisualChanged)
    /** @brief 强制交互状态控制（用于引导或演示） */
    Q_PROPERTY(InteractionState interactionState READ interactionState WRITE setInteractionState NOTIFY interactionStateChanged)

public:
    enum ButtonStyle { Standard, Accent, Subtle };
    Q_ENUM(ButtonStyle)

    enum ButtonSize { Small, StandardSize, Large };
    Q_ENUM(ButtonSize)

    enum ButtonLayout { 
        TextOnly,       
        IconBefore,     
        IconOnly,       
        IconAfter       
    };
    Q_ENUM(ButtonLayout)

    enum InteractionState { 
        Rest,           
        Hover,          
        Pressed,        
        Disabled        
    };
    Q_ENUM(InteractionState)

    explicit Button(const QString& text, QWidget* parent = nullptr);
    explicit Button(QWidget* parent = nullptr);

    // 属性访问器
    ButtonStyle fluentStyle() const { return m_style; }
    void setFluentStyle(ButtonStyle style);

    ButtonSize fluentSize() const { return m_size; }
    void setFluentSize(ButtonSize size);

    ButtonLayout fluentLayout() const { return m_layout; }
    void setFluentLayout(ButtonLayout layout);

    bool hasFocusVisual() const { return m_focusVisual; }
    void setFocusVisual(bool focus);

    InteractionState interactionState() const { return m_interactionState; }
    void setInteractionState(InteractionState state);

    void onThemeUpdated() override { update(); }

    // 提供给布局系统的尺寸提示
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void fluentStyleChanged();
    void fluentSizeChanged();
    void fluentLayoutChanged();
    void focusVisualChanged();
    void interactionStateChanged();

protected:
    // 核心绘制逻辑
    void paintEvent(QPaintEvent* event) override;

    // 状态触发重绘
    void focusInEvent(QFocusEvent* event) override { QPushButton::focusInEvent(event); update(); }
    void focusOutEvent(QFocusEvent* event) override { QPushButton::focusOutEvent(event); update(); }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override { QPushButton::enterEvent(event); update(); }
#else
    void enterEvent(QEvent* event) override { QPushButton::enterEvent(event); update(); }
#endif
    void leaveEvent(QEvent* event) override { QPushButton::leaveEvent(event); update(); }
    void mousePressEvent(QMouseEvent* event) override { QPushButton::mousePressEvent(event); update(); }
    void mouseReleaseEvent(QMouseEvent* event) override { QPushButton::mouseReleaseEvent(event); update(); }

private:
    ButtonStyle m_style = Standard;
    ButtonSize m_size = StandardSize;
    ButtonLayout m_layout = TextOnly;
    InteractionState m_interactionState = Rest;
    bool m_focusVisual = false;
};

} // namespace view::basicinput

#endif // BUTTON_H
