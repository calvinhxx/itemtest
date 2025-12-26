#include "ResponsiveLabel.h"
#include <QMetaProperty>
#include <QDebug>

ResponsiveLabel::ResponsiveLabel(QWidget *parent) : QLabel(parent) {}

ResponsiveLabel::ResponsiveLabel(const QString &text, QWidget *parent) : QLabel(text, parent) {}

void ResponsiveLabel::setText(const QString &text) {
    if (this->text() == text) return;
    QLabel::setText(text);
    emit textChanged(text);
}

void ResponsiveLabel::bind(QObject *source, const char *propertyName) {
    if (!source) return;

    const QMetaObject *meta = source->metaObject();
    int propIndex = meta->indexOfProperty(propertyName);
    if (propIndex == -1) {
        qWarning() << "ResponsiveLabel: Property" << propertyName << "not found on" << source;
        return;
    }

    QMetaProperty prop = meta->property(propIndex);
    
    // 1. 初始化同步
    setText(prop.read(source).toString());

    // 2. 建立自动同步 (基于元对象系统的 NOTIFY 信号)
    if (prop.hasNotifySignal()) {
        QMetaMethod notifySignal = prop.notifySignal();
        // 找到我们自己的 setText(QString) 槽函数
        QMetaMethod updateSlot = this->metaObject()->method(
            this->metaObject()->indexOfMethod("setText(QString)")
        );
        
        // 建立动态连接
        connect(source, notifySignal, this, updateSlot);
    } else {
        qWarning() << "ResponsiveLabel: Property" << propertyName << "has no NOTIFY signal. Auto-update will not work.";
    }
}

