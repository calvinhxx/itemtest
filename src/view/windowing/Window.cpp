#include "Window.h"

#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "TitleBar.h"
#include "design/Breakpoints.h"

namespace view::windowing {

Window::Window(QWidget* parent)
    : QWidget(parent),
      m_chrome(this) {
    m_chrome.applyPlatformWindowFlags();

    setAutoFillBackground(false);
    setMinimumSize(Breakpoints::MinWindowWidth, Breakpoints::MinWindowHeight);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_titleBar = new TitleBar(this);
    m_titleBar->setSystemReservedLeadingWidth(m_chrome.nativeTitleBarLeadingInset());
    m_titleBar->setVisible(m_chrome.usesCustomWindowChrome() || m_chrome.prefersNativeMacControls());
    rootLayout->addWidget(m_titleBar);

    m_contentHost = new QWidget(this);
    m_contentHost->setObjectName(QStringLiteral("fluentWindowContentHost"));
    m_contentHost->setAutoFillBackground(false);
    auto* contentLayout = new QVBoxLayout(m_contentHost);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    rootLayout->addWidget(m_contentHost, 1);

    connect(m_titleBar, &TitleBar::chromeGeometryChanged, this, &Window::updateChromeOptions);
    connect(m_titleBar, &TitleBar::dragStarted, this, &Window::handleTitleBarDragStarted);
    connect(m_titleBar, &TitleBar::dragMoved, this, &Window::handleTitleBarDragMoved);
    connect(m_titleBar, &TitleBar::dragFinished, this, &Window::handleTitleBarDragFinished);
    connect(m_titleBar, &TitleBar::doubleClicked, this, &Window::handleTitleBarDoubleClicked);

    onThemeUpdated();
    updateChromeOptions();
}

void Window::setContentWidget(QWidget* widget) {
    if (m_contentWidget == widget)
        return;

    auto* contentLayout = qobject_cast<QVBoxLayout*>(m_contentHost->layout());
    if (m_contentWidget) {
        contentLayout->removeWidget(m_contentWidget);
        m_contentWidget->setParent(nullptr);
    }

    m_contentWidget = widget;
    if (widget) {
        widget->setParent(m_contentHost);
        contentLayout->addWidget(widget);
    }
}

void Window::onThemeUpdated() {
    if (m_titleBar)
        m_titleBar->onThemeUpdated();
    update();
}

void Window::minimizeWindow() {
    emit minimizeRequested();
    showMinimized();
}

void Window::toggleMaximizeRestore() {
    if (isMaximized()) {
        emit restoreRequested();
        showNormal();
    } else {
        emit maximizeRequested();
        showMaximized();
    }
    updateChromeOptions();
}

void Window::closeWindow() {
    emit closeRequested();
    close();
}

void Window::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    const auto& colors = themeColors();
    painter.fillRect(rect(), colors.bgCanvas);

    if (m_chrome.usesCustomWindowChrome()) {
        painter.setPen(colors.strokeDefault);
        painter.drawRect(rect().adjusted(0, 0, -1, -1));
    }
}

void Window::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateChromeOptions();
}

void Window::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    m_chrome.applyPlatformWindowFlags();
    m_titleBar->setSystemReservedLeadingWidth(m_chrome.nativeTitleBarLeadingInset());
    m_titleBar->setVisible(m_chrome.usesCustomWindowChrome() || m_chrome.prefersNativeMacControls());
    updateChromeOptions();
}

void Window::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);

    if (event->type() == QEvent::WindowStateChange) {
        updateChromeOptions();
    }
}

bool Window::nativeEvent(const QByteArray& eventType,
                         void* message,
                         compatibility::FluentNativeEventResult* result) {
    return m_chrome.handleNativeEvent(eventType, message, result);
}

void Window::updateChromeOptions() {
    if (!m_titleBar)
        return;

    compatibility::WindowChromeOptions options = m_chrome.options();
    options.titleBarRect = m_titleBar->isVisible() ? m_titleBar->geometry() : QRect();
    options.dragExclusionRects.clear();
    if (m_titleBar->isVisible()) {
        for (const QRect& rect : m_titleBar->dragExclusionRects()) {
            options.dragExclusionRects << QRect(m_titleBar->mapTo(this, rect.topLeft()), rect.size());
        }
    }
    options.resizeBorderWidth = 8;
    m_chrome.configure(options);
}

void Window::handleTitleBarDragStarted(const QPoint& globalPos) {
    updateChromeOptions();
    m_fallbackDragging = false;
    if (m_chrome.beginSystemMove(globalPos))
        return;

    m_fallbackDragging = true;
    m_fallbackDragOffset = globalPos - frameGeometry().topLeft();
}

void Window::handleTitleBarDragMoved(const QPoint& globalPos) {
    if (!m_fallbackDragging)
        return;

    move(globalPos - m_fallbackDragOffset);
}

void Window::handleTitleBarDragFinished() {
    m_fallbackDragging = false;
}

void Window::handleTitleBarDoubleClicked(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    m_fallbackDragging = false;

    if (m_chrome.performTitleBarDoubleClick()) {
        updateChromeOptions();
        return;
    }

    toggleMaximizeRestore();
}

} // namespace view::windowing
