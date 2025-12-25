#ifndef VM_RESPONSIVELABEL_H
#define VM_RESPONSIVELABEL_H

#include <QObject>
#include <QString>

/**
 * @brief VM_ResponsiveLabel - ResponsiveLabel 对应的视图模型。
 * 采用 VM_ 前缀命名，明确其作为 ViewModel 的身份。
 */
class VM_ResponsiveLabel : public QObject {
    Q_OBJECT
    // 暴露给 View 绑定的属性
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit VM_ResponsiveLabel(QObject *parent = nullptr);

    QString text() const { return m_text; }
    void setText(const QString &t);

signals:
    void textChanged(const QString &t);

private:
    QString m_text = "就绪";
};

#endif // VM_RESPONSIVELABEL_H

