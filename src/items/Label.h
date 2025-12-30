#ifndef LABEL_H
#define LABEL_H

#include <QLabel>
#include <QString>

/**
 * @brief Label - 响应式文本组件
 */
class Label : public QLabel {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit Label(QWidget *parent = nullptr);
    explicit Label(const QString &text, QWidget *parent = nullptr);

public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);
};

#endif // LABEL_H
