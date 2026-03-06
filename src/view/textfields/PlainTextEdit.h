#ifndef PLAINTEXTEDIT_H
#define PLAINTEXTEDIT_H

#include <QMargins>
#include <QWidget>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#else
class QEnterEvent;
#endif
class QPlainTextEdit;
class QScrollBar;
class QPainter;
class QPaintEvent;

namespace view::textfields {

/**
 * @brief PlainTextEdit - WinUI 3 风格多行文本编辑组件
 *
 * 独立控件，自带 Fluent 外框与底部高亮；支持 contentMargins、主题样式。
 */
class PlainTextEdit : public QWidget, public ::FluentElement, public ::view::QMLPlus {
    Q_OBJECT
    /** @brief 内容区域内边距，用于样式表中的 padding */
    Q_PROPERTY(QMargins contentMargins READ contentMargins WRITE setContentMargins NOTIFY contentMarginsChanged)
    /** @brief 聚焦时底部高亮条宽度 (px)，样式与 LineEdit 保持一致 */
    Q_PROPERTY(int focusedBorderWidth READ focusedBorderWidth WRITE setFocusedBorderWidth NOTIFY borderWidthChanged)
    /** @brief 未聚焦时底部边框宽度 (px)，样式与 LineEdit 保持一致 */
    Q_PROPERTY(int unfocusedBorderWidth READ unfocusedBorderWidth WRITE setUnfocusedBorderWidth NOTIFY borderWidthChanged)

public:
    explicit PlainTextEdit(QWidget* parent = nullptr);

    // 文本相关 API（转发给内部 QPlainTextEdit）
    void setPlainText(const QString& text);
    QString toPlainText() const;
    void clear();

    void setPlaceholderText(const QString& text);
    QString placeholderText() const;

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    QScrollBar* verticalScrollBar() const;

    void setFocus(Qt::FocusReason reason);

    void onThemeUpdated() override;

    QMargins contentMargins() const { return m_contentMargins; }
    void setContentMargins(const QMargins& margins);

    int focusedBorderWidth() const { return m_focusedBorderWidth; }
    void setFocusedBorderWidth(int width);

    int unfocusedBorderWidth() const { return m_unfocusedBorderWidth; }
    void setUnfocusedBorderWidth(int width);

signals:
    /** @brief 文本内容发生变化（转发自内部编辑器） */
    void textChanged();
    void contentMarginsChanged();
    void borderWidthChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void applyThemeStyle();
    void paintFrame(QPainter& painter);

    QPlainTextEdit* m_editor = nullptr;
    QMargins m_contentMargins = QMargins(6, 0, 6, 0);
    bool m_isHovered = false;
    bool m_isFocused = false;
    int m_focusedBorderWidth = 2;
    int m_unfocusedBorderWidth = 1;
};

} // namespace view::textfields

#endif // PLAINTEXTEDIT_H
