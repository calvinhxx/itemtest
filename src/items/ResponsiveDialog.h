#ifndef RESPONSIVEDIALOG_H
#define RESPONSIVEDIALOG_H

#include <QDialog>
#include <QString>

/**
 * @brief ResponsiveDialog - 一个高度抽象的响应式对话框基类。
 * 
 * 它仅处理对话框的核心行为（标题、可见性、模态等）的响应式绑定。
 * 具体的内容由使用者通过布局或 setContent() 自定义。
 */
class ResponsiveDialog : public QDialog {
    Q_OBJECT
public:
    explicit ResponsiveDialog(QWidget *parent = nullptr);

    /**
     * @brief 绑定对话框标题
     */
    void bindTitle(QObject *source, const char *propertyName);

    /**
     * @brief 绑定对话框的显示/隐藏状态
     */
    void bindVisible(QObject *source, const char *propertyName);

public slots:
    void setWindowTitle(const QString &title);
    void setVisible(bool visible) override;

signals:
    void titleChanged(const QString &title);
    void visibleChanged(bool visible);
};

#endif // RESPONSIVEDIALOG_H
