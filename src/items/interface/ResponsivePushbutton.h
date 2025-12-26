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
public:
    explicit ResponsivePushbutton(QWidget *parent = nullptr);
    explicit ResponsivePushbutton(const QString &text, QWidget *parent = nullptr);

    /**
     * @brief 绑定文本属性
     */
    void bindText(QObject *source, const char *propertyName);

    /**
     * @brief 绑定启用状态属性
     */
    void bindEnabled(QObject *source, const char *propertyName);

public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);
};

#endif // RESPONSIVEPUSHBUTTON_H

