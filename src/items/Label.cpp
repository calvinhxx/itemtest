#include "Label.h"

Label::Label(QWidget *parent) : QLabel(parent) {
}

Label::Label(const QString &text, QWidget *parent) : QLabel(text, parent) {
}

void Label::setText(const QString &text) {
    if (QLabel::text() != text) {
        QLabel::setText(text);
        emit textChanged(text);
        updateGeometry();
    }
}

