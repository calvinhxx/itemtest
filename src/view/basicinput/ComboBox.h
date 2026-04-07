#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <QHash>
#include <QPoint>
#include <QPersistentModelIndex>
#include <QStyledItemDelegate>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "view/dialogs_flyouts/Dialog.h"
#include "common/Typography.h"
#include "common/Spacing.h"

class QPropertyAnimation;

class QVariantAnimation;

namespace view::collections {
class ListView;
}
namespace view::textfields { class LineEdit; }

namespace view::basicinput {

// ─── ComboBox 弹层代理 ──────────────────────────────────────────────────────

class ComboBoxItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ComboBoxItemDelegate(FluentElement* themeHost, QAbstractItemView* view,
                                  QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

private:
    qreal accentProgress(const QModelIndex& index) const;
    void animateAccent(const QModelIndex& index);

    FluentElement* m_themeHost = nullptr;
    QAbstractItemView* m_view = nullptr;
    QHash<QPersistentModelIndex, QVariantAnimation*> m_accentAnims;
};

/**
 * @brief ComboBox - WinUI 3 风格组合框
 *
 * Figma MCP 设计规范：
 *   - 控件：4px 外圆角，3px 内圆角，bg rgba(255,255,255,0.7)，1px border rgba(0,0,0,0.06)
 *   - 内边距：px=11 py=4，文本 14px Body，色 rgba(0,0,0,0.9)
 *   - Chevron：Segoe Fluent Icons 12px，色 rgba(0,0,0,0.61)
 *   - 弹层：8px 圆角，阴影 0 8px 16px rgba(0,0,0,0.14)
 *   - 列表项：40px 高，文本左缩进 16px
 *   - 选中指示器：3px 宽 16px 高，accent 色，圆角药丸形
 *
 * 状态驱动：Rest → Hover → Pressed → Disabled → Open
 *
 * 可编辑模式（setEditable(true)）：
 *   - 内嵌 LineEdit 允许用户输入文本
 *   - 边框 accent 底边条（WinUI 3 focused style）
 *   - 下拉弹层位于控件下方
 */
class ComboBox : public QComboBox, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    Q_PROPERTY(int contentPaddingH READ contentPaddingH WRITE setContentPaddingH NOTIFY layoutChanged)
    Q_PROPERTY(int contentPaddingV READ contentPaddingV WRITE setContentPaddingV NOTIFY layoutChanged)
    Q_PROPERTY(QString chevronGlyph READ chevronGlyph WRITE setChevronGlyph NOTIFY chevronChanged)
    Q_PROPERTY(int chevronSize READ chevronSize WRITE setChevronSize NOTIFY chevronChanged)
    Q_PROPERTY(QPoint chevronOffset READ chevronOffset WRITE setChevronOffset NOTIFY chevronChanged)
    Q_PROPERTY(int popupOffset READ popupOffset WRITE setPopupOffset NOTIFY layoutChanged)
    Q_PROPERTY(qreal pressProgress READ pressProgress WRITE setPressProgress)

public:
    explicit ComboBox(QWidget* parent = nullptr);
    ~ComboBox() override;

    // --- Appearance ---
    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    int contentPaddingH() const { return m_contentPaddingH; }
    void setContentPaddingH(int px);

    int contentPaddingV() const { return m_contentPaddingV; }
    void setContentPaddingV(int px);

    QString chevronGlyph() const { return m_chevronGlyph; }
    void setChevronGlyph(const QString& glyph);

    int chevronSize() const { return m_chevronSize; }
    void setChevronSize(int size);

    QPoint chevronOffset() const { return m_chevronOffset; }
    void setChevronOffset(const QPoint& offset);

    int popupOffset() const { return m_popupOffset; }
    void setPopupOffset(int offset);

    qreal pressProgress() const { return m_pressProgress; }
    void setPressProgress(qreal p);

    // --- Editable ---
    void setEditable(bool editable);

    // --- QComboBox overrides ---
    void showPopup() override;
    void hidePopup() override;

    QSize sizeHint() const override;

signals:
    void fontRoleChanged();
    void layoutChanged();
    void chevronChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    void onThemeUpdated() override;

private:
    friend class ComboBoxPopup;
    void initAnimation();
    void onPopupHidden();
    void layoutLineEdit();
    void applyLineEditStyle();
    void validateLineEditText();

    // --- Configurable design tokens ---
    QString m_fontRole       = Typography::FontRole::Body;
    int     m_contentPaddingH = ::Spacing::Padding::ComboBoxHorizontal;
    int     m_contentPaddingV = ::Spacing::Padding::ComboBoxVertical;
    QString m_chevronGlyph   = Typography::Icons::ChevronDownMed;
    int     m_chevronSize    = 12;
    QPoint  m_chevronOffset  {::Spacing::Padding::ComboBoxHorizontal, 0};
    int     m_popupOffset    = ::Spacing::XSmall; // 4px gap between combo and popup

    // --- State ---
    bool  m_hovered  = false;
    bool  m_pressed  = false;
    bool  m_chevronHovered = false;
    bool  m_popupVisible = false;
    qreal m_pressProgress = 0.0;

    QPropertyAnimation* m_pressAnimation = nullptr;

    // --- Editable ---
    view::textfields::LineEdit* m_lineEdit = nullptr;

    // --- Popup ---
    class ComboBoxPopup;
    ComboBoxPopup* m_popup = nullptr;
};

// ─── ComboBox 弹层窗口 ──────────────────────────────────────────────────────

class ComboBox::ComboBoxPopup : public view::dialogs_flyouts::Dialog {
public:
    explicit ComboBoxPopup(ComboBox* comboBox);

    void showForComboBox();
    void onThemeUpdated() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    ComboBox* m_comboBox;
    view::collections::ListView* m_listView;
    ComboBoxItemDelegate* m_delegate;
};

} // namespace view::basicinput

#endif // COMBOBOX_H
