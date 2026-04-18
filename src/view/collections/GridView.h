#ifndef GRIDVIEW_H
#define GRIDVIEW_H

#include <QListView>

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QLabel;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QTimer;
class QVariantAnimation;
class QWheelEvent;

namespace view::scrolling { class ScrollBar; }

namespace view::collections {

/**
 * Fluent 网格视图（仅视图层）。
 *
 * 基于 QListView IconMode + Wrapping 实现 WinUI 3 GridView 语义。
 * 不负责注入 QAbstractItemModel 或 QStyledItemDelegate；由业务 / 页面层或测试组装。
 *
 * 提供：主题调色板与字体、Fluent 纵向滚动条、WinUI 风格选择模式映射、
 *       可配置的 cellSize / spacing / maxColumns、视口 hover。
 *
 * WinUI 3 / Figma 视觉对齐：
 *   - 容器 1px ControlStrokeColor 边框 + CornerRadius::Control(4px) 圆角
 *   - 可选 headerText 显示在网格上方
 *   - 空网格占位符文本
 *   - 每个 cell 112×112 默认尺寸，4px 间距
 */
class GridView : public QListView, public FluentElement, public view::QMLPlus {
    Q_OBJECT

public:
    enum class GridSelectionMode {
        None,
        Single,
        Multiple,
        Extended
    };
    Q_ENUM(GridSelectionMode)

    Q_PROPERTY(GridSelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
    Q_PROPERTY(QString fontRole READ fontRole WRITE setFontRole NOTIFY fontRoleChanged)
    Q_PROPERTY(bool viewportHovered READ viewportHovered NOTIFY viewportHoveredChanged)

    /** 是否允许拖拽换位（对应 WinUI CanReorderItems + CanDragItems + AllowDrop） */
    Q_PROPERTY(bool canReorderItems READ canReorderItems WRITE setCanReorderItems NOTIFY canReorderItemsChanged)

    /** 容器外边框是否可见 */
    Q_PROPERTY(bool borderVisible READ borderVisible WRITE setBorderVisible NOTIFY borderVisibleChanged)
    /** 网格上方标题文本（对应 WinUI GridView.Header） */
    Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText NOTIFY headerTextChanged)
    /** 网格为空时的占位提示文本 */
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)

    /** 单元格尺寸 (默认 112×112，对齐 Figma GridView Item 设计稿) */
    Q_PROPERTY(QSize cellSize READ cellSize WRITE setCellSize NOTIFY cellSizeChanged)
    /** 单元格水平间距 */
    Q_PROPERTY(int horizontalSpacing READ horizontalSpacing WRITE setHorizontalSpacing NOTIFY horizontalSpacingChanged)
    /** 单元格垂直间距 */
    Q_PROPERTY(int verticalSpacing READ verticalSpacing WRITE setVerticalSpacing NOTIFY verticalSpacingChanged)
    /** 最大列数 (0 = 不限制，自动根据容器宽度计算) */
    Q_PROPERTY(int maxColumns READ maxColumns WRITE setMaxColumns NOTIFY maxColumnsChanged)

    explicit GridView(QWidget* parent = nullptr);
    ~GridView() override = default;

    // --- Selection ---
    GridSelectionMode selectionMode() const { return m_selectionMode; }
    void setSelectionMode(GridSelectionMode mode);

    // --- Appearance ---
    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    bool borderVisible() const { return m_borderVisible; }
    void setBorderVisible(bool visible);

    QString headerText() const { return m_headerText; }
    void setHeaderText(const QString& text);

    QString placeholderText() const { return m_placeholderText; }
    void setPlaceholderText(const QString& text);

    bool viewportHovered() const { return m_viewportHovered; }

    // --- Drag reorder ---
    bool canReorderItems() const { return m_canReorderItems; }
    void setCanReorderItems(bool enabled);

    // --- Grid layout ---
    QSize cellSize() const { return m_cellSize; }
    void setCellSize(const QSize& size);

    int horizontalSpacing() const { return m_hSpacing; }
    void setHorizontalSpacing(int spacing);

    int verticalSpacing() const { return m_vSpacing; }
    void setVerticalSpacing(int spacing);

    int maxColumns() const { return m_maxColumns; }
    void setMaxColumns(int maxCols);

    // --- Selection API ---
    int selectedIndex() const;
    QList<int> selectedRows() const;
    void setSelectedIndex(int index);

    ::view::scrolling::ScrollBar* verticalFluentScrollBar() const;
    void refreshFluentScrollChrome();
    QRect visualRect(const QModelIndex& index) const override;

signals:
    void selectionModeChanged();
    void fontRoleChanged();
    void viewportHoveredChanged();
    void borderVisibleChanged();
    void headerTextChanged();
    void placeholderTextChanged();
    void cellSizeChanged();
    void horizontalSpacingChanged();
    void verticalSpacingChanged();
    void maxColumnsChanged();
    void canReorderItemsChanged();
    void itemReordered(int fromIndex, int toIndex);
    void itemClicked(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

    void enterEvent(FluentEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    int verticalOffset() const override;

    // Drag-reorder (custom mouse handling)
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void onThemeUpdated() override;

private:
    void applyThemeStyle();
    void syncFluentScrollBar();
    void layoutHeader();
    void setViewportHovered(bool hovered);
    void updateViewportMargins();
    void updateGridSize();
    void startBounceBack();
    int dropIndicatorIndex(const QPoint& pos) const;
    void updateDragDisplacement();
    void clearDragAnimations();
    QPixmap renderItemPixmap(int row) const;
    QPixmap renderDragPixmap() const;

    GridSelectionMode m_selectionMode = GridSelectionMode::Single;
    QString m_fontRole;

    // --- Container visuals ---
    bool m_borderVisible = true;
    QString m_headerText;
    QString m_placeholderText;
    QLabel* m_headerLabel = nullptr;

    // --- Grid layout ---
    QSize m_cellSize{112, 112};
    int m_hSpacing = 4;
    int m_vSpacing = 4;
    int m_maxColumns = 0;

    ::view::scrolling::ScrollBar* m_vScrollBar = nullptr;
    bool m_viewportHovered = false;

    // --- Drag reorder ---
    bool m_canReorderItems = false;
    bool m_isDragging = false;
    bool m_dragPressIntercepted = false;
    bool m_pressedOnBlank = false;
    QPoint m_dragStartPos;
    QPoint m_dragCurrentPos;
    QPixmap m_dragPixmap;
    int  m_dragSourceIndex = -1;
    QList<int> m_dragSourceIndices;
    int  m_dropTargetIndex = -1;
    QHash<int, QPointF>            m_dragOffsets;
    QHash<int, QVariantAnimation*> m_dragAnims;
    mutable bool m_paintingWithOffsets = false;

    // --- Overscroll bounce ---
    qreal m_overscrollY = 0.0;
    QVariantAnimation* m_bounceAnim = nullptr;
    QTimer* m_bounceTimer = nullptr;
};

using GridSelectionMode = GridView::GridSelectionMode;

} // namespace view::collections

#endif // GRIDVIEW_H
