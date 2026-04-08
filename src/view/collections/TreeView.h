#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>
#include <QPersistentModelIndex>
#include <QHash>
#include <QStandardItemModel>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QLabel;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QVariantAnimation;
class QWheelEvent;
class QTimer;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QEvent;
#endif

namespace view::scrolling { class ScrollBar; }

namespace view::collections {

/**
 * Fluent TreeView（仅视图层）。
 *
 * 不负责注入 QAbstractItemModel 或 QStyledItemDelegate；由业务 / 页面层或测试组装。
 * 提供：主题调色板与字体、Fluent 滚动条、WinUI 风格选中/展开、视口 hover。
 *
 * WinUI 3 / Figma 视觉对齐：
 *   - 容器 1px ControlStrokeColor 边框 + CornerRadius::Control(4px) 圆角
 *   - 可选 headerText 显示在树上方
 *   - 空列表占位符文本
 *   - 每级缩进 16px
 *   - 展开/折叠使用 Segoe Fluent Icons 中号 Chevron
 *   - 选中项左侧 3px accent 指示条
 */
class TreeView : public QTreeView, public FluentElement, public view::QMLPlus {
    Q_OBJECT

public:
    enum class TreeSelectionMode {
        None,
        Single,
        Multiple,
        Extended
    };
    Q_ENUM(TreeSelectionMode)

    Q_PROPERTY(TreeSelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    Q_PROPERTY(bool viewportHovered READ viewportHovered NOTIFY viewportHoveredChanged)

    /** 容器外边框是否可见 */
    Q_PROPERTY(bool borderVisible READ borderVisible WRITE setBorderVisible NOTIFY borderVisibleChanged)
    /** 容器背景是否可见 */
    Q_PROPERTY(bool backgroundVisible READ backgroundVisible WRITE setBackgroundVisible NOTIFY backgroundVisibleChanged)
    /** 树上方标题文本 */
    Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText NOTIFY headerTextChanged)
    /** 树为空时的占位提示文本 */
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)

    /** 是否允许拖拽换位（文件管理器风格：同/跨父节点移动 & 拖放至文件夹） */
    Q_PROPERTY(bool canReorderItems READ canReorderItems WRITE setCanReorderItems NOTIFY canReorderItemsChanged)

    explicit TreeView(QWidget* parent = nullptr);
    ~TreeView() override = default;

    // --- Selection ---
    TreeSelectionMode selectionMode() const { return m_selectionMode; }
    void setSelectionMode(TreeSelectionMode mode);

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

    // --- Drag reorder ---
    bool canReorderItems() const { return m_canReorderItems; }
    void setCanReorderItems(bool enabled);

    // --- Tree API ---
    void expandAll();
    void collapseAll();
    void toggleExpanded(const QModelIndex& index);

    // --- Selection API ---
    QModelIndex selectedItem() const;
    QModelIndexList selectedItems() const;
    void setSelectedItem(const QModelIndex& index);

    ::view::scrolling::ScrollBar* verticalFluentScrollBar() const;
    ::view::scrolling::ScrollBar* horizontalFluentScrollBar() const;

    void refreshFluentScrollChrome();

signals:
    void selectionModeChanged();
    void fontRoleChanged();
    void viewportHoveredChanged();
    void borderVisibleChanged();
    void backgroundVisibleChanged();
    void headerTextChanged();
    void placeholderTextChanged();
    void itemClicked(const QModelIndex& index);
    void canReorderItemsChanged();
    void itemReordered(const QModelIndex& srcParent, int srcRow,
                       const QModelIndex& dstParent, int dstRow);

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
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    int verticalOffset() const override;
    void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

    void onThemeUpdated() override;

private:
    void applyThemeStyle();
    void syncFluentScrollBar();
    void syncFluentHScrollBar();
    void layoutHeader();
    void setViewportHovered(bool hovered);
    void updateViewportMargins();
    void startBounceBack();

    // Drag helpers — file-manager style
    void computeDropTarget(const QPoint& pos);
    bool isDescendantOf(const QModelIndex& candidate, const QModelIndex& ancestor) const;
    QPixmap renderRowPixmap(const QModelIndex& index) const;

    TreeSelectionMode m_selectionMode = TreeSelectionMode::Single;
    QString m_fontRole;

    // --- Container visuals ---
    bool m_borderVisible = true;
    bool m_backgroundVisible = true;
    QString m_headerText;
    QString m_placeholderText;
    QLabel* m_headerLabel = nullptr;

    ::view::scrolling::ScrollBar* m_vScrollBar = nullptr;
    ::view::scrolling::ScrollBar* m_hScrollBar = nullptr;
    bool m_viewportHovered = false;

    // --- Overscroll bounce ---
    qreal m_overscrollY = 0.0;
    QVariantAnimation* m_bounceAnim = nullptr;
    QTimer* m_bounceTimer = nullptr;

    // --- Drag reorder (file-manager style) ---
    enum class DropMode { None, Between, OnItem };
    bool m_canReorderItems = false;
    bool m_isDragging = false;
    QPersistentModelIndex m_dragSourceIndex;
    QPoint m_dragStartPos;
    QPoint m_dragCurrentPos;
    QPixmap m_dragPixmap;
    DropMode m_dropMode = DropMode::None;
    QPersistentModelIndex m_dropTargetParent;  // parent for Between, unused for OnItem
    int m_dropTargetRow = -1;                  // row index for Between, -1 for OnItem
    QPersistentModelIndex m_dropOnIndex;       // target item for OnItem (re-parent)

    // --- Expand animation ---
    QVariantAnimation* m_expandRevealAnim = nullptr;
    QPersistentModelIndex m_animParent;
    bool m_animEnabled = true;
    bool m_animExpanding = true;   // true=expand, false=collapse

    // Chevron rotation progress per-index: 0.0=collapsed(right), 1.0=expanded(down)
    QHash<QPersistentModelIndex, QVariantAnimation*> m_chevronAnims;

public:
    /** Query chevron rotation progress for delegate painting. 0=right, 1=down. */
    qreal chevronRotation(const QModelIndex& index) const;
};

using TreeSelectionMode = TreeView::TreeSelectionMode;

} // namespace view::collections

#endif // TREEVIEW_H
