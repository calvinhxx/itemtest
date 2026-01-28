#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include "view/FluentElement.h"

namespace view::basicinput {

class Button : public QPushButton, public FluentElement {
    Q_OBJECT
public:
    explicit Button(const QString& text, QWidget* parent = nullptr);
    explicit Button(QWidget* parent = nullptr);

    void onThemeUpdated() override;
};

} // namespace view::basicinput

#endif // BUTTON_H
