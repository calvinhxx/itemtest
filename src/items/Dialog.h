#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>

/**
 * @brief Dialog - 响应式对话框基础类
 */
class Dialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
public:
    explicit Dialog(QWidget *parent = nullptr);

public slots:
    void setWindowTitle(const QString &title);
    void setVisible(bool v) override;

signals:
    void windowTitleChanged(const QString &title);
    void visibleChanged(bool v);
};

#endif // DIALOG_H
