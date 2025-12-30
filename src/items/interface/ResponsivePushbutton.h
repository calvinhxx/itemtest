#ifndef RESPONSIVEPUSHBUTTON_H
#define RESPONSIVEPUSHBUTTON_H

#include <QPushButton>
#include <QString>

/**
 * @brief ResponsivePushbutton - 仿 QML 风格的响应式按钮
 * 
 * 支持绑定 text 和 enabled 等属性到 ViewModel。
 */
class ResponsivePushbutton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
public:
    explicit ResponsivePushbutton(QWidget *parent = nullptr);
    explicit ResponsivePushbutton(const QString &text, QWidget *parent = nullptr);

public slots:
    void setText(const QString &text);
    void setEnabled(bool enabled);

signals:
    void textChanged(const QString &text);
    void enabledChanged(bool enabled);
};

#endif // RESPONSIVEPUSHBUTTON_H

