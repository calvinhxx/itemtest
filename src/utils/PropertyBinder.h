#ifndef PROPERTYBINDER_H
#define PROPERTYBINDER_H

#include <QObject>
#include <QMetaProperty>
#include <QVariant>
#include <QPointer>

// 前向声明
class PropertyLink;

/**
 * @brief PropertyBinder - 简约通用的属性绑定器 (Qt 5/6 兼容)
 * 
 * 使用方式：
 *   PropertyBinder::bind(viewModel, "text", label, "text");
 *   PropertyBinder::bind(viewModel, "visible", dialog, "visible", PropertyBinder::TwoWay);
 * 
 * 原理：
 *   1. 使用 QMetaProperty 读取/写入属性值
 *   2. 通过字符串形式的信号槽连接实现自动同步
 *   3. 使用 PropertyLink 类作为中间对象，提供显式槽函数，确保 Qt 5/6 兼容
 */
class PropertyBinder {
public:
    enum Direction { OneWay, TwoWay };

    /**
     * @brief 绑定两个对象的属性
     * @param source 源对象 (通常是 ViewModel)
     * @param sPropName 源属性名
     * @param target 目标对象 (通常是 Widget)
     * @param tPropName 目标属性名
     * @param dir 绑定方向，默认单向
     */
    static void bind(QObject* source, const char* sPropName,
                    QObject* target, const char* tPropName,
                    Direction dir = OneWay);
};

/**
 * @brief PropertyLink - 内部辅助类，用于属性同步
 * 使用显式槽函数确保 Qt 5/6 兼容性
 * 注意：必须放在 PropertyBinder 外部，因为 MOC 不支持嵌套类的 Q_OBJECT
 */
class PropertyLink : public QObject {
    Q_OBJECT
public:
    PropertyLink(QObject* from, const QMetaProperty& fromProp,
                QObject* to, const QMetaProperty& toProp,
                QObject* parent)
        : QObject(parent)
        , m_from(from)
        , m_fromProp(fromProp)
        , m_to(to)
        , m_toProp(toProp) {
    }

public slots:
    void syncToTarget() {
        if (!m_from || !m_to) return;
        QVariant val = m_fromProp.read(m_from);
        if (val.isValid() && m_toProp.read(m_to) != val) {
            m_toProp.write(m_to, val);
        }
    }

private:
    QPointer<QObject> m_from;
    QMetaProperty m_fromProp;
    QPointer<QObject> m_to;
    QMetaProperty m_toProp;
};

#endif // PROPERTYBINDER_H
