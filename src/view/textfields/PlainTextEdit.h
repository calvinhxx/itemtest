#ifndef PLAINTEXTEDIT_H
#define PLAINTEXTEDIT_H

#include <QMargins>
#include <QWidget>
#include <QString>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"
#include "common/Typography.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#else
class QEnterEvent;
#endif
class QPlainTextEdit;
class QPainter;
class QPaintEvent;

namespace view::scrolling { class ScrollBar; }

namespace view::textfields {

/**
 * @brief PlainTextEdit - WinUI 3 风格多行文本编辑组件
 *
 * 独立控件，自带 Fluent 外框与底部高亮；支持 contentMargins、主题样式。
 * 自适应行高：在 minVisibleLines ~ maxVisibleLines 范围内随内容动态调整高度，
 * 超过 maxVisibleLines 时显示自定义 Fluent 滚动条。
 */
class PlainTextEdit : public QWidget, public ::FluentElement, public ::view::QMLPlus {
    Q_OBJECT
    /** @brief 内容区域内边距，用于样式表中的 padding */
    Q_PROPERTY(QMargins contentMargins READ contentMargins WRITE setContentMargins NOTIFY contentMarginsChanged)
    /** @brief 文本使用的主题字体 Token 名称，参见 Typography::FontRole */
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    /** @brief 聚焦时底部高亮条宽度 (px)，样式与 LineEdit 保持一致 */
    Q_PROPERTY(int focusedBorderWidth READ focusedBorderWidth WRITE setFocusedBorderWidth NOTIFY focusedBorderWidthChanged)
    /** @brief 未聚焦时底部边框宽度 (px)，样式与 LineEdit 保持一致 */
    Q_PROPERTY(int unfocusedBorderWidth READ unfocusedBorderWidth WRITE setUnfocusedBorderWidth NOTIFY unfocusedBorderWidthChanged)
    /** @brief 单行高度 (px)，用于自适应高度计算 */
    Q_PROPERTY(int lineHeight READ lineHeight WRITE setLineHeight NOTIFY layoutMetricsChanged)
    /** @brief 最小可见行数，控件不会收缩到该行数以下 */
    Q_PROPERTY(int minVisibleLines READ minVisibleLines WRITE setMinVisibleLines NOTIFY layoutMetricsChanged)
    /** @brief 最大可见行数，超过后显示滚动条而不再增高 */
    Q_PROPERTY(int maxVisibleLines READ maxVisibleLines WRITE setMaxVisibleLines NOTIFY layoutMetricsChanged)

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

    /** @brief 返回 Fluent 自定义滚动条（精确类型） */
    ::view::scrolling::ScrollBar* verticalScrollBar() const;

    /** @brief 将焦点转发给内部编辑器（无参版本，覆盖 QWidget::setFocus()） */
    void setFocus();
    void setFocus(Qt::FocusReason reason);

    void onThemeUpdated() override;

    QMargins contentMargins() const { return m_contentMargins; }
    void setContentMargins(const QMargins& margins);

    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    int focusedBorderWidth() const { return m_focusedBorderWidth; }
    void setFocusedBorderWidth(int width);

    int unfocusedBorderWidth() const { return m_unfocusedBorderWidth; }
    void setUnfocusedBorderWidth(int width);

    int lineHeight() const { return m_lineHeight; }
    void setLineHeight(int height);

    int minVisibleLines() const { return m_minVisibleLines; }
    void setMinVisibleLines(int lines);

    int maxVisibleLines() const { return m_maxVisibleLines; }
    void setMaxVisibleLines(int lines);

signals:
    /** @brief 文本内容发生变化（转发自内部编辑器） */
    void textChanged();
    /** @brief 光标位置发生变化（转发自内部编辑器） */
    void cursorPositionChanged();
    /** @brief 选区发生变化（转发自内部编辑器） */
    void selectionChanged();
    void contentMarginsChanged();
    void fontRoleChanged();
    void focusedBorderWidthChanged();
    void unfocusedBorderWidthChanged();
    void layoutMetricsChanged();

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
    void updateHeightForContent();

    QPlainTextEdit*              m_editor     = nullptr;
    ::view::scrolling::ScrollBar* m_vScrollBar = nullptr;
    QMargins m_contentMargins   = QMargins(::Spacing::Padding::TextFieldHorizontal, 0,
                                           ::Spacing::Padding::TextFieldHorizontal, 0);
    QString  m_fontRole         = Typography::FontRole::Body;
    bool     m_isHovered        = false;
    bool     m_isFocused        = false;
    int      m_focusedBorderWidth   = ::Spacing::Border::Focused;
    int      m_unfocusedBorderWidth = ::Spacing::Border::Normal;
    int      m_lineHeight           = ::Spacing::XLarge; ///< 默认单行高度 32px
    int      m_minVisibleLines      = 1;
    int      m_maxVisibleLines      = 4;
    QString  m_placeholderText;
};

} // namespace view::textfields

#endif // PLAINTEXTEDIT_H
