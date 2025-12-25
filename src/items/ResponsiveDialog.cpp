#include "ResponsiveDialog.h"
#include <QMetaProperty>
#include <QDebug>

ResponsiveDialog::ResponsiveDialog(QWidget *parent) : QDialog(parent) {
}

void ResponsiveDialog::setWindowTitle(const QString &title) {
    if (windowTitle() == title) return;
    QDialog::setWindowTitle(title);
    emit titleChanged(title);
}

void ResponsiveDialog::setVisible(bool visible) {
    if (isVisible() == visible) return;
    QDialog::setVisible(visible);
    emit visibleChanged(visible);
}

void ResponsiveDialog::bindTitle(QObject *source, const char *propertyName) {
    if (!source) return;
    const QMetaObject *meta = source->metaObject();
    int propIndex = meta->indexOfProperty(propertyName);
    if (propIndex == -1) return;

    QMetaProperty prop = meta->property(propIndex);
    setWindowTitle(prop.read(source).toString());

    if (prop.hasNotifySignal()) {
        connect(source, prop.notifySignal(), this, 
                this->metaObject()->method(this->metaObject()->indexOfMethod("setWindowTitle(QString)")));
    }
}

void ResponsiveDialog::bindVisible(QObject *source, const char *propertyName) {
    if (!source) return;
    const QMetaObject *meta = source->metaObject();
    int propIndex = meta->indexOfProperty(propertyName);
    if (propIndex == -1) return;

    QMetaProperty prop = meta->property(propIndex);
    setVisible(prop.read(source).toBool());

    if (prop.hasNotifySignal()) {
        connect(source, prop.notifySignal(), this, 
                this->metaObject()->method(this->metaObject()->indexOfMethod("setVisible(bool)")));
    }
}
