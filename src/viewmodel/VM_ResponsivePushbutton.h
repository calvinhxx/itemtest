#ifndef VM_RESPONSIVEPUSHBUTTON_H
#define VM_RESPONSIVEPUSHBUTTON_H

#include <QObject>
#include <QString>

/**
 * @brief VM_ResponsivePushbutton - ResponsivePushbutton 对应的视图模型。
 */
class VM_ResponsivePushbutton : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit VM_ResponsivePushbutton(QObject *parent = nullptr);

    QString text() const { return m_text; }
    void setText(const QString &t);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool e);

signals:
    void textChanged(const QString &t);
    void enabledChanged(bool e);

private:
    QString m_text = "按钮";
    bool m_enabled = true;
};

#endif // VM_RESPONSIVEPUSHBUTTON_H
