#ifndef LABEL_H
#define LABEL_H

#include <QLabel>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::textfields {

class Label : public QLabel, public FluentElement, public view::QMLPlus {
    Q_OBJECT
public:
    explicit Label(const QString& text, QWidget* parent = nullptr);
    explicit Label(QWidget* parent = nullptr);

    void onThemeUpdated() override;
};

} // namespace view::textfields

#endif // LABEL_H
