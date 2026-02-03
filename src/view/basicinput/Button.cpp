#include "Button.h"

namespace view::basicinput {

Button::Button(const QString& text, QWidget* parent) : QPushButton(text, parent) {
    onThemeUpdated();
}

Button::Button(QWidget* parent) : QPushButton(parent) {
    onThemeUpdated();
}

void Button::onThemeUpdated() {
    const auto& colors = themeColors();
    const auto& radius = themeRadius();
    const auto& spacing = themeSpacing();

    setFont(themeFont("BodyStrong").toQFont());

    setStyleSheet(QString(
            "QPushButton {"
        "  background-color: %1; color: %2; border: 1px solid %3; border-radius: %4px;"
        "  padding: %5px %6px;"
            "}"
        "QPushButton:hover { background-color: %7; }"
        "QPushButton:pressed { background-color: %8; }"
        "QPushButton:disabled { background-color: %9; color: %10; border: 1px solid %11; }"
    ).arg(colors.accentDefault.name(), colors.textOnAccent.name(), colors.strokeDefault.name())
     .arg(radius.small)
     .arg(spacing.padding.controlV).arg(spacing.padding.controlH)
     .arg(colors.accentSecondary.name(), colors.accentTertiary.name())
     .arg(colors.controlDisabled.name(), colors.textDisabled.name(), colors.strokeDivider.name()));
}

} // namespace view::basicinput
