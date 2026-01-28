#include "Label.h"

namespace view::textfields {

Label::Label(const QString& text, QWidget* parent) : QLabel(text, parent) {
    onThemeUpdated();
}

Label::Label(QWidget* parent) : QLabel(parent) {
    onThemeUpdated();
}

void Label::onThemeUpdated() {
    setFont(themeFont("Body").toQFont());
    setStyleSheet(QString("color: %1;").arg(themeColors().textPrimary.name()));
}

} // namespace view::textfields
