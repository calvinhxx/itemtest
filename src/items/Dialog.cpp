#include "Dialog.h"

Dialog::Dialog(QWidget *parent) : QDialog(parent) {
}

void Dialog::setWindowTitle(const QString &title) {
    if (QDialog::windowTitle() != title) {
        QDialog::setWindowTitle(title);
        emit windowTitleChanged(title);
    }
}

void Dialog::setVisible(bool v) {
    if (isVisible() != v) {
        QDialog::setVisible(v);
        emit visibleChanged(v);
    }
}

