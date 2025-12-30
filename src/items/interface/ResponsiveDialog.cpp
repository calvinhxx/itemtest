#include "ResponsiveDialog.h"

ResponsiveDialog::ResponsiveDialog(QWidget *parent) : QDialog(parent) {
    setMinimumSize(200, 100);
}

void ResponsiveDialog::setWindowTitle(const QString &title) {
    if (QDialog::windowTitle() == title) return;
    QDialog::setWindowTitle(title);
    emit windowTitleChanged(title);
}

void ResponsiveDialog::setVisible(bool visible) {
    if (isVisible() == visible) return;
    QDialog::setVisible(visible);
    emit visibleChanged(visible);
}
