#ifndef RESPONSIVELABEL_H
#define RESPONSIVELABEL_H

#include <QLabel>
#include <QString>
#include <QPointer>

/**
 * @brief ResponsiveLabel - 一个仿 QML 风格的响应式 Label 组件。
 * 
 * 它允许通过 bind() 方法直接将 Label 的 text 属性绑定到另一个 QObject 的属性上。
 * 当被绑定的属性发生变化（发出 NOTIFY 信号）时，Label 会自动更新显示内容。
 */
class ResponsiveLabel : public QLabel {
    Q_OBJECT
public:
    explicit ResponsiveLabel(QWidget *parent = nullptr);
    explicit ResponsiveLabel(const QString &text, QWidget *parent = nullptr);

    /**
     * @brief 绑定文本到指定对象的属性
     * @param source 数据源对象 (通常是 ViewModel)
     * @param propertyName 属性名称 (字符串)
     */
    void bind(QObject *source, const char *propertyName);

public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);
};

#endif // RESPONSIVELABEL_H

