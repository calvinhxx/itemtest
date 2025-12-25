#ifndef VM_RESPONSIVEDIALOG_H
#define VM_RESPONSIVEDIALOG_H

#include <QObject>
#include <QString>

/**
 * @brief VM_ResponsiveDialog - 仅包含对话框控制逻辑的视图模型
 */
class VM_ResponsiveDialog : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    explicit VM_ResponsiveDialog(QObject *parent = nullptr);

    QString title() const { return m_title; }
    void setTitle(const QString &t);

    bool visible() const { return m_visible; }
    void setVisible(bool v);

signals:
    void titleChanged(const QString &t);
    void visibleChanged(bool v);

private:
    QString m_title = "响应式对话框";
    bool m_visible = false;
};

#endif // VM_RESPONSIVEDIALOG_H
