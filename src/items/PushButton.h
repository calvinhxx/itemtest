#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QPushButton>
#include <QString>

/**
 * @brief PushButton - 响应式按钮组件
 */
class PushButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
public:
    explicit PushButton(QWidget *parent = nullptr);
    explicit PushButton(const QString &text, QWidget *parent = nullptr);

public slots:
    void setText(const QString &text);
    void setEnabled(bool e);

signals:
    void textChanged(const QString &text);
    void enabledChanged(bool e);
};

#endif // PUSHBUTTON_H
