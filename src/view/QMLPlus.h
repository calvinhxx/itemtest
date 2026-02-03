#ifndef QMLPLUS_H
#define QMLPLUS_H

#include <QWidget>
#include <QLayout>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QVector>
#include <QPointer>
#include <QMargins>
#include <QMetaProperty>

namespace view {

// =============================================================================
// 1. Anchors 核心定义
// =============================================================================

class AnchorLayout : public QLayout {
    Q_OBJECT
public:
    enum class Edge { None, Left, Right, Top, Bottom, HCenter, VCenter };

    struct Anchor {
        QWidget* target = nullptr;
        Edge edge = Edge::None;
        int offset = 0;
        Anchor() = default;
        Anchor(QWidget* t, Edge e, int o = 0) : target(t), edge(e), offset(o) {}
    };

    struct Anchors {
        Anchor left, right, top, bottom, horizontalCenter, verticalCenter;
        bool fill = false;
        QMargins fillMargins;
        Anchors() : fillMargins(0, 0, 0, 0) {}
    };

    explicit AnchorLayout(QWidget* parent = nullptr);
    ~AnchorLayout();

    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;

    void addAnchoredWidget(QWidget* w, const Anchors& anchors);

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

// =============================================================================
// 2. Property Binding 核心定义
// =============================================================================

class PropertyLink : public QObject {
    Q_OBJECT
public:
    PropertyLink(QObject* from, const QMetaProperty& fromProp, QObject* to, const QMetaProperty& toProp, QObject* parent);
public slots:
    void syncToTarget();
private:
    QPointer<QObject> m_from;
    QMetaProperty m_fromProp;
    QPointer<QObject> m_to;
    QMetaProperty m_toProp;
};

class PropertyBinder {
public:
    enum Direction { OneWay, TwoWay };
    static void bind(QObject* source, const char* sPropName, QObject* target, const char* tPropName, Direction dir = OneWay);
};

// =============================================================================
// 3. States 核心定义
// =============================================================================

struct PropertyChange {
    QPointer<QObject> target;
    QByteArray propertyName;
    QVariant value;
};

struct QMLState {
    QString name;
    QVector<PropertyChange> changes;
};

// =============================================================================
// 4. QMLPlus Mixin (自动发现宿主模式)
// =============================================================================

class QMLPlus {
public:
    QMLPlus();
    virtual ~QMLPlus();

    // --- 核心能力 ---
    AnchorLayout::Anchors* anchors();
    
    void setState(const QString& name);
    QString state() const { return m_currentState; }
    void addState(const QMLState& state);

    /**
     * @brief 属性绑定接口，自动将宿主 QWidget 作为 Target
     */
    void bind(const char* targetProp, QObject* source, const char* sourceProp, PropertyBinder::Direction dir = PropertyBinder::OneWay);

protected:
    void applyState(const QString& name);

private:
    AnchorLayout::Anchors* m_anchors = nullptr;
    QString m_currentState;
    QMap<QString, QMLState> m_states;
    QMap<QObject*, QMap<QByteArray, QVariant>> m_defaultValues;
};

} // namespace view

#endif // QMLPLUS_H
