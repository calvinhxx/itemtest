#ifndef ANCHORLAYOUT_H
#define ANCHORLAYOUT_H

#include <QLayout>
#include <QWidget>
#include <QMargins>
#include <QVector>
#include <QRect>

/**
 * @brief AnchorLayout - 一个简约的、仿 QML Anchors 的 QWidget 布局管理器。
 * 
 * 它允许通过设置锚点（anchors）来确定子控件的位置和大小。
 */
class AnchorLayout : public QLayout {
    Q_OBJECT
public:
    struct Anchors {
        // 锚定目标（nullptr 表示父控件）
        QWidget* leftTo = nullptr;   int leftOffset = 0;
        QWidget* rightTo = nullptr;  int rightOffset = 0;
        QWidget* topTo = nullptr;    int topOffset = 0;
        QWidget* bottomTo = nullptr; int bottomOffset = 0;

        bool fill = false;           // 是否填充父控件
        QMargins fillMargins;        // fill 为 true 时的外边距

        bool horizontalCenter = false; // 水平居中
        int horizontalCenterOffset = 0; // 水平居中偏移

        bool verticalCenter = false;   // 垂直居中
        int verticalCenterOffset = 0;   // 垂直居中偏移

        Anchors() : fillMargins(0, 0, 0, 0) {}
    };

    explicit AnchorLayout(QWidget* parent = nullptr);
    ~AnchorLayout();

    // 添加一个带锚点配置的 widget
    void addAnchoredWidget(QWidget* w, const Anchors& anchors);

    // QLayout 接口实现
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
};

#endif // ANCHORLAYOUT_H
