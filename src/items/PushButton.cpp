#include "PushButton.h"

PushButton::PushButton(QWidget *parent) : QPushButton(parent) {
}

PushButton::PushButton(const QString &text, QWidget *parent) : QPushButton(text, parent) {
}

void PushButton::setText(const QString &text) {
    if (QPushButton::text() != text) {
        QPushButton::setText(text);
        emit textChanged(text);
    }
}

void PushButton::setEnabled(bool e) {
    if (QPushButton::isEnabled() != e) {
        QPushButton::setEnabled(e);
        emit enabledChanged(e);
    }
}

