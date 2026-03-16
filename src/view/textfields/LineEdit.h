#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QMargins>
#include <QPoint>
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
class QPainter;
class QPaintEvent;
class QResizeEvent;

namespace view::basicinput { class Button; }

namespace view::textfields {

/**
 * @brief LineEdit - WinUI 3 风格单行文本编辑组件
 *
 * 独立控件，自带 Fluent 外框与底部高亮；支持 contentMargins、主题样式。
 */
class LineEdit : public QLineEdit, public ::FluentElement, public ::view::QMLPlus {
    Q_OBJECT
    /** @brief 内容区域内边距，用于样式表中的 padding */
    Q_PROPERTY(QMargins contentMargins READ contentMargins WRITE setContentMargins NOTIFY contentMarginsChanged)
    /** @brief 文本使用的主题字体 Token 名称，参见 Typography::FontRole */
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    /** @brief 是否启用清除按钮（右侧 X） */
    Q_PROPERTY(bool clearButtonEnabled READ isClearButtonEnabled WRITE setClearButtonEnabled NOTIFY clearButtonEnabledChanged)
    /** @brief 清除按钮大小 (px) */
    Q_PROPERTY(int clearButtonSize READ clearButtonSize WRITE setClearButtonSize NOTIFY clearButtonSizeChanged)
    /** @brief 清除按钮偏移 (x=距右边缘, y=相对垂直中心的偏移) */
    Q_PROPERTY(QPoint clearButtonOffset READ clearButtonOffset WRITE setClearButtonOffset NOTIFY clearButtonOffsetChanged)
    /** @brief 聚焦时底部高亮条宽度 (px) */
    Q_PROPERTY(int focusedBorderWidth READ focusedBorderWidth WRITE setFocusedBorderWidth NOTIFY focusedBorderWidthChanged)
    /** @brief 未聚焦时底部边框宽度 (px) */
    Q_PROPERTY(int unfocusedBorderWidth READ unfocusedBorderWidth WRITE setUnfocusedBorderWidth NOTIFY unfocusedBorderWidthChanged)

public:
    explicit LineEdit(QWidget* parent = nullptr);

    void onThemeUpdated() override;

    QMargins contentMargins() const { return m_contentMargins; }
    void setContentMargins(const QMargins& margins);

    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    bool isClearButtonEnabled() const { return m_clearButtonEnabled; }
    void setClearButtonEnabled(bool enabled);

    int clearButtonSize() const { return m_clearButtonSize; }
    void setClearButtonSize(int size);

    QPoint clearButtonOffset() const { return m_clearButtonOffset; }
    void setClearButtonOffset(const QPoint& offset);

    int focusedBorderWidth() const { return m_focusedBorderWidth; }
    void setFocusedBorderWidth(int width);

    int unfocusedBorderWidth() const { return m_unfocusedBorderWidth; }
    void setUnfocusedBorderWidth(int width);

signals:
    void contentMarginsChanged();
    void fontRoleChanged();
    void clearButtonEnabledChanged();
    void clearButtonSizeChanged();
    void clearButtonOffsetChanged();
    void focusedBorderWidthChanged();
    void unfocusedBorderWidthChanged();

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
    void updateClearButtonVisibility();
    void updateClearButtonGeometry();

    QMargins m_contentMargins  = QMargins(::Spacing::Padding::TextFieldHorizontal,
                                          ::Spacing::Padding::TextFieldVertical,
                                          ::Spacing::Padding::TextFieldHorizontal,
                                          ::Spacing::Padding::TextFieldVertical);
    QString  m_fontRole        = Typography::FontRole::Body;
    ::view::basicinput::Button* m_clearButton = nullptr;
    bool     m_clearButtonEnabled = true;
    int      m_clearButtonSize    = 22; ///< 清除按钮边长（px）
    QPoint   m_clearButtonOffset  = QPoint(::Spacing::XSmall, 0);
    int      m_focusedBorderWidth   = ::Spacing::Border::Focused;
    int      m_unfocusedBorderWidth = ::Spacing::Border::Normal;
    bool     m_isHovered = false;
    bool     m_isFocused = false;
};

} // namespace view::textfields

#endif // LINEEDIT_H
