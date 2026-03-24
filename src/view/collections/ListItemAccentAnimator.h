#ifndef LIST_ITEM_ACCENT_ANIMATOR_H
#define LIST_ITEM_ACCENT_ANIMATOR_H

#include <QObject>
#include <QHash>
#include <QPersistentModelIndex>

class QAbstractItemView;
class QVariantAnimation;

namespace view::collections {

/**
 * 为 ListView 中每个选中项独立驱动 accent 条 0→1 动画。
 * 由 delegate 创建并持有；delegate 在 paint() 中调用 progress(index) 获取当前进度。
 */
class ListItemAccentAnimator : public QObject {
    Q_OBJECT
public:
    explicit ListItemAccentAnimator(QAbstractItemView* view, QObject* parent = nullptr);

    /** 获取指定 index 的 accent 动画进度 (0~1)，无动画时返回 1.0 */
    qreal progress(const QModelIndex& index) const;

    /** 选中变化时由 delegate 或外部调用，为新选中的 index 启动动画 */
    void animateSelection(const QModelIndex& index);

    /** 清除已失效的动画缓存 */
    void cleanup();

private:
    QAbstractItemView* m_view = nullptr;
    QHash<QPersistentModelIndex, QVariantAnimation*> m_animations;
};

} // namespace view::collections

#endif // LIST_ITEM_ACCENT_ANIMATOR_H
