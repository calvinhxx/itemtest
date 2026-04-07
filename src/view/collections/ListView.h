#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>
#include <QHash>
#include <functional>

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
class QPropertyAnimation;
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
    Q_PROPERTY(Flow flow READ flow WRITE setFlow NOTIFY flowChanged)

    /** 容器外边框是否可见（WinUI ListView 默认带 1px ControlStroke 边框） */
    Q_PROPERTY(bool borderVisible READ borderVisible WRITE setBorderVisible NOTIFY borderVisibleChanged)
    /** 容器背景是否可见（嵌入 Dialog/Flyout 等自绘容器时设为 false） */
    Q_PROPERTY(bool backgroundVisible READ backgroundVisible WRITE setBackgroundVisible NOTIFY backgroundVisibleChanged)
    /** 列表上方的自定义 header 控件（类似 QML ListView.header） */
    Q_PROPERTY(QWidget* header READ header WRITE setHeader NOTIFY headerChanged)
    /** 列表下方的自定义 footer 控件（类似 QML ListView.footer） */
    Q_PROPERTY(QWidget* footer READ footer WRITE setFooter NOTIFY footerChanged)
    /** 列表上方标题文本（便捷接口 — 内部创建 QLabel 设为 header） */
    Q_PROPERTY(QString headerText READ headerText WRITE setHeaderText NOTIFY headerTextChanged)
    /** 列表下方尾部文本（便捷接口 — 内部创建 QLabel 设为 footer） */
    Q_PROPERTY(QString footerText READ footerText WRITE setFooterText NOTIFY footerTextChanged)
    /** 列表为空时的占位提示文本 */
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    /** 是否允许拖拽换位（对应 WinUI CanReorderItems + CanDragItems + AllowDrop） */
    Q_PROPERTY(bool canReorderItems READ canReorderItems WRITE setCanReorderItems NOTIFY canReorderItemsChanged)
    /** Section 分组回调：给定行号返回分组标题；标题相同的连续行归入同一 section */
    Q_PROPERTY(bool sectionEnabled READ sectionEnabled WRITE setSectionEnabled NOTIFY sectionEnabledChanged)

    explicit ListView(QWidget* parent = nullptr);
    ~ListView() override = default;

    // --- Flow ---
    using Flow = QListView::Flow;  // TopToBottom or LeftToRight
    Flow flow() const;
    void setFlow(Flow flow);

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

    // --- Header / Footer (custom widget API) ---
    QWidget* header() const { return m_header; }
    void setHeader(QWidget* widget);

    QWidget* footer() const { return m_footer; }
    void setFooter(QWidget* widget);

    // --- Header / Footer (convenience text API) ---
    QString headerText() const { return m_headerText; }
    void setHeaderText(const QString& text);

    QString footerText() const { return m_footerText; }
    void setFooterText(const QString& text);

    QString placeholderText() const { return m_placeholderText; }
    void setPlaceholderText(const QString& text);

    bool viewportHovered() const { return m_viewportHovered; }

    // --- Drag reorder ---
    bool canReorderItems() const { return m_canReorderItems; }
    void setCanReorderItems(bool enabled);

    // --- Section ---
    using SectionKeyFunc = std::function<QString(int row)>;
    bool sectionEnabled() const { return m_sectionEnabled; }
    void setSectionEnabled(bool enabled);
    /** 设置分组 key 回调。相邻行返回相同 key 的归为一组，key 作为 section header 绘制。 */
    void setSectionKeyFunction(SectionKeyFunc func);

    // --- Selection API ---
    int selectedIndex() const;
    QList<int> selectedRows() const;
    void setSelectedIndex(int index);

    ::view::scrolling::ScrollBar* verticalFluentScrollBar() const;
    ::view::scrolling::ScrollBar* horizontalFluentScrollBar() const;

    /**
     * 隐藏 QAbstractScrollArea 内置滚动条并刷新 Fluent 纵向条（QComboBox 弹层等场景下
     * 平台/样式可能把系统滚动条重新显示出来，需在 show 后再次压制）。
     */
    void refreshFluentScrollChrome();

signals:
    void selectionModeChanged();
    void fontRoleChanged();
    void viewportHoveredChanged();
    void flowChanged();
    void borderVisibleChanged();
    void backgroundVisibleChanged();
    void headerChanged();
    void footerChanged();
    void headerTextChanged();
    void footerTextChanged();
    void placeholderTextChanged();
    void canReorderItemsChanged();
    void sectionEnabledChanged();
    void itemClicked(int index);
    void itemReordered(int fromRow, int toRow);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

    // Drag-reorder events
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    int verticalOffset() const override;
    int horizontalOffset() const override;
    QRect visualRect(const QModelIndex& index) const override;

    void onThemeUpdated() override;

    friend class SectionProxyDelegate;

private:
    void applyThemeStyle();
    /** 压制原生条 + 同步 range/pageStep + 定位 + 显隐 Fluent 纵向滚动条 */
    void syncFluentScrollBar();
    void syncFluentHScrollBar();
    void layoutHeader();
    void layoutFooter();
    void setViewportHovered(bool hovered);
    void updateViewportMargins();
    void startBounceBack();
    void paintSectionHeaders(QPainter& p);
    void installSectionProxy();
    bool isPointInSectionHeader(const QPoint& viewportPos) const;
    int dropIndicatorRow(const QPoint& pos) const;
    void updateDragDisplacement();
    void clearDragAnimations();

    ListSelectionMode m_selectionMode = ListSelectionMode::Single;
    QString m_fontRole;

    // --- Container visuals ---
    bool m_borderVisible = true;
    bool m_backgroundVisible = true;
    QString m_headerText;
    QString m_footerText;
    QString m_placeholderText;
    QWidget* m_header = nullptr;
    QWidget* m_footer = nullptr;
    bool m_ownsHeader = false;   // 内部通过 setHeaderText 创建的 QLabel
    bool m_ownsFooter = false;   // 内部通过 setFooterText 创建的 QLabel

    ::view::scrolling::ScrollBar* m_vScrollBar = nullptr;
    ::view::scrolling::ScrollBar* m_hScrollBar = nullptr;
    bool m_viewportHovered = false;

    // --- Drag reorder ---
    bool m_canReorderItems = false;
    int  m_dragSourceRow = -1;
    int  m_dropTargetRow = -1;      // 拖拽指示线位置
    QHash<int, qreal>              m_dragOffsets;  // row → 当前 Y 位移 px
    QHash<int, QVariantAnimation*> m_dragAnims;    // row → 位移动画
    mutable bool m_paintingWithOffsets = false;

    // --- Section ---
    bool m_sectionEnabled = false;
    SectionKeyFunc m_sectionKeyFunc;
    QAbstractItemDelegate* m_sectionProxy = nullptr;
    QAbstractItemDelegate* m_userDelegate = nullptr;

    // --- Overscroll bounce ---
    qreal m_overscrollY = 0.0;
    qreal m_overscrollX = 0.0;
    QVariantAnimation* m_bounceAnim = nullptr;
    QTimer* m_bounceTimer = nullptr;
};

using ListSelectionMode = ListView::ListSelectionMode;

} // namespace view::collections

#endif // LISTVIEW_H
