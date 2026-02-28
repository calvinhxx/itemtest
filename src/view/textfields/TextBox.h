#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <QWidget>
#include <QMargins>
#include <QString>
#include <QPoint>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#else
#include <QEvent>
typedef QEvent QEnterEvent;
#endif

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QLineEdit;
class QPlainTextEdit;
class QValidator;
class QPaintEvent;
class QResizeEvent;

namespace view::basicinput { class Button; }
namespace view::scrolling  { class ScrollBar; }

namespace view::textfields {

/**
 * @brief TextBox - WinUI 3 风格的文本输入框
 * 
 * 具有以下特性：
 * - 支持 Placeholder（占位文本）
 * - 支持单行 (QLineEdit) 与多行 (QPlainTextEdit) 模式
 * - 内置 Clear Button（清除按钮）
 * - 完美的 Fluent 设计语言视觉效果（背景、圆角、焦点高亮）
 */
class TextBox : public QWidget, public ::FluentElement, public ::view::QMLPlus {
    Q_OBJECT
    
    /** @brief 输入框中的文本内容 */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    /** @brief 当输入框为空时显示的提示文本 */
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    /** @brief 是否为只读模式 */
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged)
    /** @brief 聚焦时底部高亮条的宽度 (px) */
    Q_PROPERTY(int focusedBorderWidth READ focusedBorderWidth WRITE setFocusedBorderWidth NOTIFY borderWidthChanged)
    /** @brief 未聚焦时底部边框的宽度 (px) */
    Q_PROPERTY(int unfocusedBorderWidth READ unfocusedBorderWidth WRITE setUnfocusedBorderWidth NOTIFY borderWidthChanged)
    /** @brief 是否启用清除按钮 (右侧的 X) */
    Q_PROPERTY(bool clearButtonEnabled READ isClearButtonEnabled WRITE setClearButtonEnabled NOTIFY clearButtonEnabledChanged)
    /** @brief 是否支持多行输入 */
    Q_PROPERTY(bool multiLine READ isMultiLine WRITE setMultiLine NOTIFY multiLineChanged)
    /** @brief 清除按钮的大小 (px) */
    Q_PROPERTY(int clearButtonSize READ clearButtonSize WRITE setClearButtonSize NOTIFY clearButtonSizeChanged)
    /** @brief 编辑器文字的内边距 */
    Q_PROPERTY(QMargins textMargins READ textMargins WRITE setTextMargins NOTIFY textMarginsChanged)
    /** @brief 清除按钮的偏移量 (相对于右侧) */
    Q_PROPERTY(QPoint clearButtonOffset READ clearButtonOffset WRITE setClearButtonOffset NOTIFY clearButtonOffsetChanged)

public:
    explicit TextBox(QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    QString placeholderText() const;
    void setPlaceholderText(const QString& placeholder);

    bool isReadOnly() const;
    void setReadOnly(bool readOnly);

    bool isClearButtonEnabled() const { return m_clearButtonEnabled; }
    void setClearButtonEnabled(bool enabled);

    bool isMultiLine() const { return m_multiLine; }
    void setMultiLine(bool multiLine);

    int clearButtonSize() const { return m_clearButtonSize; }
    void setClearButtonSize(int size);

    QMargins textMargins() const { return m_textMargins; }
    void setTextMargins(const QMargins& margins);

    QPoint clearButtonOffset() const { return m_clearButtonOffset; }
    void setClearButtonOffset(const QPoint& offset);

    int focusedBorderWidth() const { return m_focusedBorderWidth; }
    void setFocusedBorderWidth(int width);

    int unfocusedBorderWidth() const { return m_unfocusedBorderWidth; }
    void setUnfocusedBorderWidth(int width);

    /** @brief 设置输入验证器（仅单行模式生效，所有权由调用方或 TextBox 管理） */
    void setValidator(QValidator* validator);
    QValidator* validator() const { return m_validator; }

    void onThemeUpdated() override;
    void setFocus(Qt::FocusReason reason = Qt::OtherFocusReason);

signals:
    void textChanged(const QString& text);
    void placeholderTextChanged();
    void readOnlyChanged();
    void borderWidthChanged();
    void clearButtonEnabledChanged();
    void multiLineChanged();
    void clearButtonSizeChanged();
    void textMarginsChanged();
    void clearButtonOffsetChanged();
    void returnPressed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void initUi();
    void updateHeight();
    void updateClearButtonVisibility();
    void ensureEditor(); // 按需创建编辑器

    QLineEdit* m_lineEdit = nullptr;
    QPlainTextEdit* m_textEdit = nullptr;
    ::view::basicinput::Button* m_clearButton = nullptr;
    ::view::scrolling::ScrollBar* m_vScrollBar = nullptr;
    
    // 状态缓存
    bool m_isHovered = false;
    bool m_isFocused = false;
    bool m_clearButtonEnabled = true;
    bool m_multiLine = false;

    // 配置属性
    int m_focusedBorderWidth = 2;
    int m_unfocusedBorderWidth = 1;
    int m_clearButtonSize = 22;
    QMargins m_textMargins = QMargins(6, 0, 6, 0); // Default margins
    QPoint m_clearButtonOffset = QPoint(6, 0);
    QValidator* m_validator = nullptr;
};

} // namespace view::textfields

#endif // TEXTBOX_H
