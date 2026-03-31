#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QLabel;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QTimer;
class QVariantAnimation;
class QWheelEvent;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QEvent;
#endif

namespace view::scrolling { class ScrollBar; }

namespace view::collections {

/**
 * Fluent 列表视图（仅视图层）。
 *
 * 不负责注入 QAbstractItemModel 或 QStyledItemDelegate；由业务 / 页面层或测试组装。
 * 提供：主题调色板与字体、Fluent 纵向滚动条、WinUI 风格选择模式映射、视口 hover。
 *
 * WinUI 3 / Figma 视觉对齐：
 *   - 容器 1px ControlStrokeColor 边框 + CornerRadius::Control(4px) 圆角
 *   - 可选 headerText 显示在列表上方
 *   - 空列表占位符文本
 *   - viewport 内容裁剪至圆角区域
 *
 * 默认不允许行内编辑（editTriggers 为 NoEditTriggers）；需要可编辑列表时请自行 setEditTriggers。
 */
class ListView : public QListView, public FluentElement, public view::QMLPlus {
    Q_OBJECT

public:
    enum class ListSelectionMode {
        None,
        Single,
        Multiple,
        Extended
    };
    Q_ENUM(ListSelectionMode)

    Q_PROPERTY(ListSelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    Q_PROPERTY(bool viewportHovered READ viewportHovered NOTIFY viewportHoveredChanged)

    /** 容器外边框是否可见（WinUI ListView 默认带 1px ControlStroke 边框） */
    Q_PROPERTY(bool borderVisible READ borderVisible WRITE setBorderVisible NOTIFY borderVisibleChanged)
    /** 容器背景是否可见（嵌入 Dialog/Flyout 等自绘容器时设为 false） */
    Q_PROPERTY(bool backgroundVisible READ backgroundVisible WRITE setBackgroundVisible NOTIFY backgroundVisibleChanged)
    /** 列表上方标题文本（对应 WinUI ListView.Header） */
    Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText NOTIFY headerTextChanged)
    /** 列表为空时的占位提示文本 */
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)

    explicit ListView(QWidget* parent = nullptr);
    ~ListView() override = default;

    // --- Selection ---
    ListSelectionMode selectionMode() const { return m_selectionMode; }
    void setSelectionMode(ListSelectionMode mode);

    // --- Appearance ---
    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    bool borderVisible() const { return m_borderVisible; }
    void setBorderVisible(bool visible);

    bool backgroundVisible() const { return m_backgroundVisible; }
    void setBackgroundVisible(bool visible);

    QString headerText() const { return m_headerText; }
    void setHeaderText(const QString& text);

    QString placeholderText() const { return m_placeholderText; }
    void setPlaceholderText(const QString& text);

    bool viewportHovered() const { return m_viewportHovered; }

    // --- Selection API ---
    int selectedIndex() const;
    QList<int> selectedRows() const;
    void setSelectedIndex(int index);

    ::view::scrolling::ScrollBar* verticalFluentScrollBar() const;

    /**
     * 隐藏 QAbstractScrollArea 内置滚动条并刷新 Fluent 纵向条（QComboBox 弹层等场景下
     * 平台/样式可能把系统滚动条重新显示出来，需在 show 后再次压制）。
     */
    void refreshFluentScrollChrome();

signals:
    void selectionModeChanged();
    void fontRoleChanged();
    void viewportHoveredChanged();
    void borderVisibleChanged();
    void backgroundVisibleChanged();
    void headerTextChanged();
    void placeholderTextChanged();
    void itemClicked(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    int verticalOffset() const override;

    void onThemeUpdated() override;

private:
    void applyThemeStyle();
    /** 压制原生条 + 同步 range/pageStep + 定位 + 显隐 Fluent 纵向滚动条 */
    void syncFluentScrollBar();
    void layoutHeader();
    void setViewportHovered(bool hovered);
    void updateViewportMargins();
    void startBounceBack();

    ListSelectionMode m_selectionMode = ListSelectionMode::Single;
    QString m_fontRole;

    // --- Container visuals ---
    bool m_borderVisible = true;
    bool m_backgroundVisible = true;
    QString m_headerText;
    QString m_placeholderText;
    QLabel* m_headerLabel = nullptr;

    ::view::scrolling::ScrollBar* m_vScrollBar = nullptr;
    bool m_viewportHovered = false;

    // --- Overscroll bounce ---
    qreal m_overscrollY = 0.0;
    QVariantAnimation* m_bounceAnim = nullptr;
    QTimer* m_bounceTimer = nullptr;
};

using ListSelectionMode = ListView::ListSelectionMode;

} // namespace view::collections

#endif // LISTVIEW_H
