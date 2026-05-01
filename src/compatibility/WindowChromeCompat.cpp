#include "WindowChromeCompat.h"

#include <QWindow>

namespace compatibility {

namespace detail {
void applyPlatformWindowFlags(QWidget* window, const WindowChromeOptions& options);
bool handlePlatformNativeEvent(QWidget* window,
                               const WindowChromeOptions& options,
                               const QByteArray& eventType,
                               void* message,
                               FluentNativeEventResult* result);
bool beginPlatformSystemMove(QWidget* window, const QPoint& globalPos);
bool beginPlatformSystemResize(QWidget* window, Qt::Edges edges, const QPoint& globalPos);
bool performPlatformTitleBarDoubleClick(QWidget* window, const WindowChromeOptions& options);
void syncPlatformTitleBarGeometry(QWidget* window, const WindowChromeOptions& options);
}

WindowChromeCompat::WindowChromeCompat(QWidget* window)
    : m_window(window) {
    m_options.useCustomWindowChrome = platformPrefersCustomWindowChrome();
    m_options.preferNativeMacControls = (currentPlatform() == Platform::MacOS);
}

void WindowChromeCompat::configure(const WindowChromeOptions& options) {
    m_options = options;
    detail::syncPlatformTitleBarGeometry(m_window, m_options);
}

void WindowChromeCompat::applyPlatformWindowFlags() {
    detail::applyPlatformWindowFlags(m_window, m_options);
}

bool WindowChromeCompat::handleNativeEvent(const QByteArray& eventType,
                                           void* message,
                                           FluentNativeEventResult* result) {
    return detail::handlePlatformNativeEvent(m_window, m_options, eventType, message, result);
}

bool WindowChromeCompat::beginSystemMove(const QPoint& globalPos) {
    if (detail::beginPlatformSystemMove(m_window, globalPos))
        return true;

    if (!m_window || !m_window->windowHandle())
        return false;

    return m_window->windowHandle()->startSystemMove();
}

bool WindowChromeCompat::beginSystemResize(Qt::Edges edges, const QPoint& globalPos) {
    if (detail::beginPlatformSystemResize(m_window, edges, globalPos))
        return true;

    if (!m_window || !m_window->windowHandle() || edges == Qt::Edges())
        return false;

    return m_window->windowHandle()->startSystemResize(edges);
}

bool WindowChromeCompat::performTitleBarDoubleClick() {
    return detail::performPlatformTitleBarDoubleClick(m_window, m_options);
}

bool WindowChromeCompat::usesCustomWindowChrome() const {
    return m_options.useCustomWindowChrome && currentPlatform() == Platform::Windows;
}

bool WindowChromeCompat::prefersNativeMacControls() const {
    return m_options.preferNativeMacControls && currentPlatform() == Platform::MacOS;
}

int WindowChromeCompat::nativeTitleBarLeadingInset() const {
    return prefersNativeMacControls() ? 140 : 0;
}

WindowChromeCompat::HitTest WindowChromeCompat::hitTestLocal(const QPoint& localPos) const {
    const QSize size = m_window ? m_window->size() : QSize();
    return classifyHitTest(m_options, size, localPos);
}

WindowChromeCompat::Platform WindowChromeCompat::currentPlatform() {
#ifdef Q_OS_WIN
    return Platform::Windows;
#elif defined(Q_OS_MAC)
    return Platform::MacOS;
#else
    return Platform::Other;
#endif
}

bool WindowChromeCompat::platformPrefersCustomWindowChrome() {
    return currentPlatform() == Platform::Windows;
}

WindowChromeCompat::HitTest WindowChromeCompat::classifyHitTest(
    const WindowChromeOptions& options,
    const QSize& windowSize,
    const QPoint& localPos) {
    if (!QRect(QPoint(0, 0), windowSize).contains(localPos))
        return HitTest::Client;

    const int border = qMax(0, options.resizeBorderWidth);
    if (border > 0) {
        const bool left = localPos.x() < border;
        const bool right = localPos.x() >= windowSize.width() - border;
        const bool top = localPos.y() < border;
        const bool bottom = localPos.y() >= windowSize.height() - border;

        if (top && left) return HitTest::TopLeft;
        if (top && right) return HitTest::TopRight;
        if (bottom && left) return HitTest::BottomLeft;
        if (bottom && right) return HitTest::BottomRight;
        if (left) return HitTest::Left;
        if (right) return HitTest::Right;
        if (top) return HitTest::Top;
        if (bottom) return HitTest::Bottom;
    }

    if (options.titleBarRect.contains(localPos)) {
        for (const QRect& r : options.dragExclusionRects) {
            if (r.contains(localPos))
                return HitTest::Client;
        }
        return HitTest::Caption;
    }

    return HitTest::Client;
}

} // namespace compatibility
