#ifndef RESPONSIVEDIALOG_H
#define RESPONSIVEDIALOG_H

#include <QDialog>
#include <QString>

/**
 * @brief ResponsiveDialog - 一个高度抽象的响应式对话框基类。
 */
class ResponsiveDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
public:
    explicit ResponsiveDialog(QWidget *parent = nullptr);

public slots:
    void setWindowTitle(const QString &title);
    void setVisible(bool visible) override;

signals:
    void windowTitleChanged(const QString &title);
    void visibleChanged(bool visible);
};

#endif // RESPONSIVEDIALOG_H
