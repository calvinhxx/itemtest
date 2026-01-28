#ifndef ANCHORLAYOUT_H
#define ANCHORLAYOUT_H

#include <QLayout>
#include <QWidget>
#include <QMargins>
#include <QVector>
#include <QRect>

/**
 * @brief AnchorLayout - 一个仿 QML Anchors 的 QWidget 布局管理器。
 */
class AnchorLayout : public QLayout {
    Q_OBJECT
public:
    enum class Edge {
        None,
        Left,
        Right,
        Top,
        Bottom,
        HCenter,
        VCenter
    };

    struct Anchor {
        QWidget* target = nullptr;
        Edge edge = Edge::None;
        int offset = 0;

        Anchor() = default;
        Anchor(QWidget* t, Edge e, int o = 0) : target(t), edge(e), offset(o) {}
    };

    struct Anchors {
        Anchor left;
        Anchor right;
        Anchor top;
        Anchor bottom;
        Anchor horizontalCenter;
        Anchor verticalCenter;

        bool fill = false;
        QMargins fillMargins;

        Anchors() : fillMargins(0, 0, 0, 0) {}
    };

    explicit AnchorLayout(QWidget* parent = nullptr);
    ~AnchorLayout();

    void addAnchoredWidget(QWidget* w, const Anchors& anchors);

    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;

private:
    struct Item {
        QLayoutItem* item = nullptr;
        Anchors anchors;
        QRect geometry;
    };

    QVector<Item> m_items;
    int getWidgetIndex(QWidget* w) const;
    int getEdgeValue(QWidget* target, Edge edge, const QRect& parentRect) const;
};

#endif // ANCHORLAYOUT_H
