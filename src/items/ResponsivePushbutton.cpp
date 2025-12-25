#include "ResponsivePushbutton.h"
#include <QMetaProperty>
#include <QDebug>

ResponsivePushbutton::ResponsivePushbutton(QWidget *parent) : QPushButton(parent) {}

ResponsivePushbutton::ResponsivePushbutton(const QString &text, QWidget *parent) : QPushButton(text, parent) {}

void ResponsivePushbutton::setText(const QString &text) {
    if (this->text() == text) return;
    QPushButton::setText(text);
    emit textChanged(text);
}

void ResponsivePushbutton::bindText(QObject *source, const char *propertyName) {
    if (!source) return;
    const QMetaObject *meta = source->metaObject();
    int propIndex = meta->indexOfProperty(propertyName);
    if (propIndex == -1) return;

    QMetaProperty prop = meta->property(propIndex);
    setText(prop.read(source).toString());

    if (prop.hasNotifySignal()) {
        QMetaMethod notifySignal = prop.notifySignal();
        QMetaMethod updateSlot = this->metaObject()->method(
            this->metaObject()->indexOfMethod("setText(QString)")
        );
        connect(source, notifySignal, this, updateSlot);
    }
}

void ResponsivePushbutton::bindEnabled(QObject *source, const char *propertyName) {
    if (!source) return;
    const QMetaObject *meta = source->metaObject();
    int propIndex = meta->indexOfProperty(propertyName);
    if (propIndex == -1) return;

    QMetaProperty prop = meta->property(propIndex);
    setEnabled(prop.read(source).toBool());

    if (prop.hasNotifySignal()) {
        QMetaMethod notifySignal = prop.notifySignal();
        QMetaMethod updateSlot = this->metaObject()->method(
            this->metaObject()->indexOfMethod("setEnabled(bool)")
        );
        connect(source, notifySignal, this, updateSlot);
    }
}

