#ifndef LABEL_H
#define LABEL_H

#include <QLabel>
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

namespace view::textfields {

class Label : public QLabel, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    /** @brief 绑定的 Fluent 排版样式名称 (如 "Title", "Display", "Caption") */
    Q_PROPERTY(QString fluentTypography READ fluentTypography WRITE setFluentTypography NOTIFY typographyChanged)

public:
    explicit Label(const QString& text, QWidget* parent = nullptr);
    explicit Label(QWidget* parent = nullptr);

    QString fluentTypography() const { return m_styleName; }
    void setFluentTypography(const QString& styleName);

    void onThemeUpdated() override;

signals:
    void typographyChanged();

private:
    QString m_styleName = "Body";
};

} // namespace view::textfields

#endif // LABEL_H
