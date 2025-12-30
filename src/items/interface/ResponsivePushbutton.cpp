#include "ResponsivePushbutton.h"

ResponsivePushbutton::ResponsivePushbutton(QWidget *parent) : QPushButton(parent) {}

ResponsivePushbutton::ResponsivePushbutton(const QString &text, QWidget *parent) : QPushButton(text, parent) {}

void ResponsivePushbutton::setText(const QString &text) {
    if (this->text() == text) return;
    QPushButton::setText(text);
    emit textChanged(text);
}

void ResponsivePushbutton::setEnabled(bool enabled) {
    if (this->isEnabled() == enabled) return;
    QPushButton::setEnabled(enabled);
    emit enabledChanged(enabled);
}
