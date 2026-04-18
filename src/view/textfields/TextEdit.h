#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMargins>
#include <QWidget>
#include <QString>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"
#include "common/Typography.h"

class QTextEdit;
class QPainter;
class QPaintEvent;

namespace view::scrolling { class ScrollBar; }

namespace view::textfields {

/**
 * @brief TextEdit - WinUI 3 风格多行文本编辑组件
 *
 * 自适应行高：在 minVisibleLines ~ maxVisibleLines 范围内随内容动态调整高度，
 * 超过 maxVisibleLines 时显示自定义 Fluent 滚动条。
 *
 * 垂直居中设计：
 *   内部使用 QTextEdit（而非 QPlainTextEdit），因为 QTextDocumentLayout
 *   原生支持 QTextBlockFormat 的 topMargin/bottomMargin，以及 rootFrame margin。
 *   通过设置 rootFrame topMargin = (lineHeight-fontLh)/2 和每个 block 的
 *   bottomMargin = lineHeight-fontLh，实现每行文本和光标在 lineHeight 槽内垂直居中。
 *   Qt 原生处理光标定位、选区高亮、鼠标点击等，无需自定义 paintEvent。
 */
class TextEdit : public QWidget, public ::FluentElement, public ::view::QMLPlus {
    Q_OBJECT
    /** @brief 内容区域内边距；左右用于水平缩进，上下为垂直居中计算的最小值 */
    Q_PROPERTY(QMargins contentMargins READ contentMargins WRITE setContentMargins NOTIFY contentMarginsChanged)
    /** @brief 文本使用的主题字体 Token 名称 */
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    /** @brief 聚焦时底部高亮条宽度 (px) */
    Q_PROPERTY(int focusedBorderWidth READ focusedBorderWidth WRITE setFocusedBorderWidth NOTIFY focusedBorderWidthChanged)
    /** @brief 未聚焦时底部边框宽度 (px) */
    Q_PROPERTY(int unfocusedBorderWidth READ unfocusedBorderWidth WRITE setUnfocusedBorderWidth NOTIFY unfocusedBorderWidthChanged)
    /** @brief 每行视觉槽高度 (px)；height = clampedLines × lineHeight */
    Q_PROPERTY(int lineHeight READ lineHeight WRITE setLineHeight NOTIFY layoutMetricsChanged)
    /** @brief 最小可见行数 */
    Q_PROPERTY(int minVisibleLines READ minVisibleLines WRITE setMinVisibleLines NOTIFY layoutMetricsChanged)
    /** @brief 最大可见行数，超过后显示滚动条而不再增高 */
    Q_PROPERTY(int maxVisibleLines READ maxVisibleLines WRITE setMaxVisibleLines NOTIFY layoutMetricsChanged)

public:
    explicit TextEdit(QWidget* parent = nullptr);

    // 文本相关 API
    void setPlainText(const QString& text);
    QString toPlainText() const;
    void clear();

    void setPlaceholderText(const QString& text);
    QString placeholderText() const;

    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    ::view::scrolling::ScrollBar* verticalScrollBar() const;

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
    void textChanged();
    void cursorPositionChanged();
    void selectionChanged();
    void contentMarginsChanged();
    void fontRoleChanged();
    void focusedBorderWidthChanged();
    void unfocusedBorderWidthChanged();
    void layoutMetricsChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(FluentEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void applyThemeStyle();
    void paintFrame(QPainter& painter);
    void updateHeightForContent();

    /**
     * @brief 设置 rootFrame margin + block bottomMargin 实现垂直居中，
     *        并设置左右 viewport margins。
     */
    void applyBlockCenterFormat();

    QTextEdit*                    m_editor      = nullptr;
    ::view::scrolling::ScrollBar* m_vScrollBar  = nullptr;
    bool m_updatingFormat = false;
    bool m_scrollEnabled  = false;

    QMargins m_contentMargins   = QMargins(::Spacing::Padding::TextFieldHorizontal,
                                           ::Spacing::Padding::TextFieldVertical,
                                           ::Spacing::Padding::TextFieldHorizontal,
                                           ::Spacing::Padding::TextFieldVertical);
    QString  m_fontRole         = Typography::FontRole::Body;
    bool     m_isHovered        = false;
    bool     m_isFocused        = false;
    int      m_focusedBorderWidth   = ::Spacing::Border::Focused;
    int      m_unfocusedBorderWidth = ::Spacing::Border::Normal;
    int      m_lineHeight           = ::Spacing::ControlHeight::Standard;
    int      m_minVisibleLines      = 1;
    int      m_maxVisibleLines      = 4;
    QString  m_placeholderText;
};

} // namespace view::textfields

#endif // TEXTEDIT_H
