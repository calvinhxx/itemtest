#include "PropertyBinder.h"

void PropertyBinder::bind(QObject* source, const char* sPropName,
                         QObject* target, const char* tPropName,
                         Direction dir) {
    if (!source || !target) return;

    // 获取属性元信息
    int sIdx = source->metaObject()->indexOfProperty(sPropName);
    int tIdx = target->metaObject()->indexOfProperty(tPropName);
    if (sIdx == -1 || tIdx == -1) return;

    QMetaProperty sProp = source->metaObject()->property(sIdx);
    QMetaProperty tProp = target->metaObject()->property(tIdx);

    // 1. 初始同步
    QVariant val = sProp.read(source);
    if (val.isValid()) {
        tProp.write(target, val);
    }

    // 2. Source -> Target 同步
    if (sProp.hasNotifySignal()) {
        PropertyLink* link = new PropertyLink(source, sProp, target, tProp, source);
        QByteArray signal = "2" + sProp.notifySignal().methodSignature();
        QObject::connect(source, signal.constData(), link, SLOT(syncToTarget()));
    }

    // 3. Target -> Source 同步 (双向绑定)
    if (dir == TwoWay && tProp.hasNotifySignal()) {
        PropertyLink* link = new PropertyLink(target, tProp, source, sProp, target);
        QByteArray signal = "2" + tProp.notifySignal().methodSignature();
        QObject::connect(target, signal.constData(), link, SLOT(syncToTarget()));
    }
}
