#ifndef RESPONSIVELABEL_H
#define RESPONSIVELABEL_H

#include <QLabel>
#include <QString>

/**
 * @brief ResponsiveLabel - 一个仿 QML 风格的响应式 Label 组件。
 */
class ResponsiveLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit ResponsiveLabel(QWidget *parent = nullptr);
    explicit ResponsiveLabel(const QString &text, QWidget *parent = nullptr);

public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);
};

#endif // RESPONSIVELABEL_H
