#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include <QObject>
#include <QString>

/**
 * @brief ViewModel - 通用视图模型
 * 
 * 包含所有常用属性，配合 PropertyBinder 使用，简化 UT 编写。
 * 所有测试可以使用同一个 ViewModel 实例，通过 PropertyBinder 绑定到不同的 Widget 属性。
 * 
 * 使用示例：
 *   ViewModel* vm = new ViewModel(parent);
 *   PropertyBinder::bind(vm, "text", label, "text");
 *   PropertyBinder::bind(vm, "enabled", button, "enabled");
 *   PropertyBinder::bind(vm, "title", dialog, "windowTitle");
 *   PropertyBinder::bind(vm, "visible", dialog, "visible", PropertyBinder::TwoWay);
 */
class ViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    explicit ViewModel(QObject *parent = nullptr);

    // Text 属性
    QString text() const { return m_text; }
    void setText(const QString &t);

    // Enabled 属性
    bool enabled() const { return m_enabled; }
    void setEnabled(bool e);

    // Title 属性
    QString title() const { return m_title; }
    void setTitle(const QString &t);

    // Visible 属性
    bool visible() const { return m_visible; }
    void setVisible(bool v);

signals:
    void textChanged(const QString &t);
    void enabledChanged(bool e);
    void titleChanged(const QString &t);
    void visibleChanged(bool v);

private:
    QString m_text;
    bool m_enabled = true;
    QString m_title;
    bool m_visible = false;
};

#endif // VIEWMODEL_H

