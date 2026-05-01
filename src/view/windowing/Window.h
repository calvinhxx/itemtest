#ifndef FLUENTWINDOW_H
#define FLUENTWINDOW_H

#include <QPoint>
#include <QWidget>

#include "compatibility/WindowChromeCompat.h"
#include "view/FluentElement.h"
#include "view/QMLPlus.h"

class QShowEvent;

namespace view::windowing {

class TitleBar;

class Window : public QWidget, public FluentElement, public view::QMLPlus {
    Q_OBJECT
    Q_PROPERTY(QWidget* contentWidget READ contentWidget WRITE setContentWidget)

public:
    explicit Window(QWidget* parent = nullptr);
    ~Window() override = default;

    TitleBar* titleBar() const { return m_titleBar; }
    QWidget* contentHost() const { return m_contentHost; }

    QWidget* contentWidget() const { return m_contentWidget; }
    void setContentWidget(QWidget* widget);

    void onThemeUpdated() override;

public slots:
    void minimizeWindow();
    void toggleMaximizeRestore();
    void closeWindow();

signals:
    void minimizeRequested();
    void maximizeRequested();
    void restoreRequested();
    void closeRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType,
                     void* message,
                     compatibility::FluentNativeEventResult* result) override;

private slots:
    void updateChromeOptions();
    void handleTitleBarDragStarted(const QPoint& globalPos);
    void handleTitleBarDragMoved(const QPoint& globalPos);
    void handleTitleBarDragFinished();
    void handleTitleBarDoubleClicked(const QPoint& globalPos);

private:
    TitleBar* m_titleBar = nullptr;
    QWidget* m_contentHost = nullptr;
    QWidget* m_contentWidget = nullptr;
    compatibility::WindowChromeCompat m_chrome;
    bool m_fallbackDragging = false;
    QPoint m_fallbackDragOffset;
};

} // namespace view::windowing

#endif // FLUENTWINDOW_H
