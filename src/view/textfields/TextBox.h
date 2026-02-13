#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "view/textfields/TextBlock.h"

namespace view::textfields {

/**
 * @brief TextBox - WinUI 3 风格的文本输入框
 *
 * 支持单行/多行模式，包含Header（标题）、Placeholder（占位符）等特性。
 * 视觉样式（边框、背景、下划线）完全自绘或通过 QStyle 实现。
 */
class TextBox : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(QString header READ header WRITE setHeader NOTIFY headerChanged)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(bool multiLine READ isMultiLine WRITE setMultiLine NOTIFY multiLineChanged)

public:
    explicit TextBox(QWidget* parent = nullptr);

    // 文本属性
    QString text() const;
    void setText(const QString& text);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    // Header 属性
    QString header() const;
    void setHeader(const QString& header);

    // 状态属性
    bool isReadOnly() const;
    void setReadOnly(bool readOnly);

    bool isMultiLine() const { return m_isMultiLine; }
    void setMultiLine(bool multiLine);

    void onThemeUpdated() override;

signals:
    void textChanged(const QString& text);
    void placeholderTextChanged();
    void headerChanged();
    void readOnlyChanged();
    void multiLineChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateLayout();
    void updateStyle();
    
    // 内部组件
    TextBlock* m_headerLabel = nullptr;
    QLineEdit* m_lineEdit = nullptr;
    QPlainTextEdit* m_textEdit = nullptr;

    // 状态
    bool m_isMultiLine = false;
    bool m_isFocused = false;
    bool m_isHovered = false;

    // 缓存颜色
    QColor m_bgColor;
    QColor m_borderColor;
    QColor m_borderActiveColor;
    QColor m_bottomBorderColor; // Rest 状态底部边框
};

} // namespace view::textfields

#endif // TEXTBOX_H
