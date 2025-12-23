#ifndef ANCHORLAYOUT_H
#define ANCHORLAYOUT_H

#include <QLayout>
#include <QLayoutItem>
#include <QPointer>
#include <QVector>
#include <QSize>
#include <QMargins>

struct AnchorSpec {
    enum Edge { LeftEdge = 0, RightEdge = 1, TopEdge = 2, BottomEdge = 3 };

    // horizontal anchors
    QPointer<QWidget> leftToWidget;        // if set, this widget's left is anchored to target edge
    Edge leftToWidgetEdge = RightEdge;     // default: left -> target.right (原有语义)
    bool leftToParent = false;
    Edge leftToParentEdge = LeftEdge;
    int leftOffset = 0;

    QPointer<QWidget> rightToWidget;
    Edge rightToWidgetEdge = LeftEdge;     // default: right -> target.left
    bool rightToParent = false;
    Edge rightToParentEdge = RightEdge;
    int rightOffset = 0;

    bool horizontalCenter = false;
    QPointer<QWidget> hCenterToWidget;     // if set, center relative to this widget; else parent
    int hCenterOffset = 0;

    // vertical anchors
    QPointer<QWidget> topToWidget;
    Edge topToWidgetEdge = BottomEdge;     // top -> target.bottom (default, place below)
    bool topToParent = false;
    Edge topToParentEdge = TopEdge;
    int topOffset = 0;

    QPointer<QWidget> bottomToWidget;
    Edge bottomToWidgetEdge = TopEdge;     // bottom -> target.top (default, place above)
    bool bottomToParent = false;
    Edge bottomToParentEdge = BottomEdge;
    int bottomOffset = 0;

    bool verticalCenter = false;
    QPointer<QWidget> vCenterToWidget;
    int vCenterOffset = 0;

    // fill shortcut (anchors.fill(parent))
    bool fillParent = false;
    QMargins fillMargins = QMargins();

    // preferred size (optional)
    QSize preferredSize = QSize();
};

class AnchorLayout : public QLayout
{
public:
    explicit AnchorLayout(QWidget *parent = nullptr);
    ~AnchorLayout() override;

    void addAnchoredWidget(QWidget *w, const AnchorSpec &spec);

    // QLayout overrides
    void addItem(QLayoutItem *item) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    void setGeometry(const QRect &rect) override;

private:
    struct Item { QLayoutItem *layoutItem = nullptr; AnchorSpec spec; QWidget *widget = nullptr; };
    QVector<Item> m_items;

    QRect parentContentsRect() const;
    int widgetEdgePos(QWidget *w, AnchorSpec::Edge edge) const;
    QSize itemSizeHint(const Item &it) const;
};

#endif // ANCHORLAYOUT_H
