#include "ResponsiveLabel.h"

ResponsiveLabel::ResponsiveLabel(QWidget *parent) : QLabel(parent) {}

ResponsiveLabel::ResponsiveLabel(const QString &text, QWidget *parent) : QLabel(text, parent) {}

void ResponsiveLabel::setText(const QString &text) {
    if (this->text() == text) return;
    QLabel::setText(text);
    this->updateGeometry(); // 确保布局感知大小变化
    emit textChanged(text);
}
