#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QItemSelection;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QVariantAnimation;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QEvent;
#endif

namespace view::scrolling { class ScrollBar; }

namespace view::collections {

/**
 * Fluent 列表视图（仅视图层）。
 *
 * 不负责注入 QAbstractItemModel 或 QStyledItemDelegate；由业务 / 页面层或测试组装。
 * 提供：主题调色板与字体、Fluent 纵向滚动条、WinUI 风格选择模式映射、视口 hover、
 * 选中项左侧强调条动画进度（供 delegate 读取）；时长可通过属性配置。
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
    /**
     * 选中变化时 0→1 的强调条动画进度，供 item delegate 绘制左侧 accent 时使用。
     * 无选中或未启动动画时为 1。
     */
    Q_PROPERTY(qreal selectionAccentProgress READ selectionAccentProgress
                   NOTIFY selectionAccentProgressChanged)

    explicit ListView(QWidget* parent = nullptr);
    ~ListView() override = default;

    ListSelectionMode selectionMode() const { return m_selectionMode; }
    void setSelectionMode(ListSelectionMode mode);

    QString fontRole() const { return m_fontRole; }
    void setFontRole(const QString& role);

    bool viewportHovered() const { return m_viewportHovered; }

    qreal selectionAccentProgress() const { return m_selectionAccentProgress; }

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
    void selectionAccentProgressChanged();
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

    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;

    void onThemeUpdated() override;

private:
    void applyThemeStyle();
    void layoutScrollBar();
    /** 始终隐藏 Qt 内置 QScrollBar，仅使用 m_vScrollBar 自绘条 */
    void suppressNativeScrollBars();
    void setViewportHovered(bool hovered);
    void restartSelectionAccentAnimation();

    ListSelectionMode m_selectionMode = ListSelectionMode::Single;
    QString m_fontRole;

    ::view::scrolling::ScrollBar* m_vScrollBar = nullptr;
    bool m_viewportHovered = false;

    QVariantAnimation* m_accentAnim = nullptr;
    qreal m_selectionAccentProgress = 1.0;
};

using ListSelectionMode = ListView::ListSelectionMode;

} // namespace view::collections

#endif // LISTVIEW_H
