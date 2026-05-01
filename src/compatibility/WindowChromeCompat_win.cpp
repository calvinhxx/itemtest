#include "WindowChromeCompat.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>

namespace compatibility {
namespace {

int toNativeHitTest(WindowChromeCompat::HitTest hitTest) {
    switch (hitTest) {
    case WindowChromeCompat::HitTest::Caption: return HTCAPTION;
    case WindowChromeCompat::HitTest::Left: return HTLEFT;
    case WindowChromeCompat::HitTest::Right: return HTRIGHT;
    case WindowChromeCompat::HitTest::Top: return HTTOP;
    case WindowChromeCompat::HitTest::Bottom: return HTBOTTOM;
    case WindowChromeCompat::HitTest::TopLeft: return HTTOPLEFT;
    case WindowChromeCompat::HitTest::TopRight: return HTTOPRIGHT;
    case WindowChromeCompat::HitTest::BottomLeft: return HTBOTTOMLEFT;
    case WindowChromeCompat::HitTest::BottomRight: return HTBOTTOMRIGHT;
    case WindowChromeCompat::HitTest::Client:
    default:
        return HTCLIENT;
    }
}

} // namespace

namespace detail {

void applyPlatformWindowFlags(QWidget* window, const WindowChromeOptions& options) {
    if (!window)
        return;

    if (options.useCustomWindowChrome) {
        window->setWindowFlag(Qt::Window, true);
        window->setWindowFlag(Qt::FramelessWindowHint, true);
        window->setWindowFlag(Qt::CustomizeWindowHint, true);
    }
}

bool handlePlatformNativeEvent(QWidget* window,
                               const WindowChromeOptions& options,
                               const QByteArray& eventType,
                               void* message,
                               FluentNativeEventResult* result) {
    if (!window || !message || !result || !options.useCustomWindowChrome)
        return false;

    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG")
        return false;

    MSG* msg = static_cast<MSG*>(message);
    if (!msg || msg->message != WM_NCHITTEST)
        return false;

    const QPoint globalPos(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam));
    const QPoint localPos = window->mapFromGlobal(globalPos);
    const auto hitTest = WindowChromeCompat::classifyHitTest(options, window->size(), localPos);
    if (hitTest == WindowChromeCompat::HitTest::Client)
        return false;

    *result = static_cast<FluentNativeEventResult>(toNativeHitTest(hitTest));
    return true;
}

bool beginPlatformSystemMove(QWidget* window, const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    if (!window)
        return false;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd)
        return false;

    ReleaseCapture();
    SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
    return true;
}

bool beginPlatformSystemResize(QWidget* window, Qt::Edges edges, const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    if (!window || edges == Qt::Edges())
        return false;

    int nativeEdge = HTCLIENT;
    const bool left = edges.testFlag(Qt::LeftEdge);
    const bool right = edges.testFlag(Qt::RightEdge);
    const bool top = edges.testFlag(Qt::TopEdge);
    const bool bottom = edges.testFlag(Qt::BottomEdge);

    if (top && left) nativeEdge = HTTOPLEFT;
    else if (top && right) nativeEdge = HTTOPRIGHT;
    else if (bottom && left) nativeEdge = HTBOTTOMLEFT;
    else if (bottom && right) nativeEdge = HTBOTTOMRIGHT;
    else if (left) nativeEdge = HTLEFT;
    else if (right) nativeEdge = HTRIGHT;
    else if (top) nativeEdge = HTTOP;
    else if (bottom) nativeEdge = HTBOTTOM;

    if (nativeEdge == HTCLIENT)
        return false;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd)
        return false;

    ReleaseCapture();
    SendMessageW(hwnd, WM_NCLBUTTONDOWN, nativeEdge, 0);
    return true;
}

bool performPlatformTitleBarDoubleClick(QWidget* window, const WindowChromeOptions& options) {
    Q_UNUSED(window);
    Q_UNUSED(options);
    return false;
}

void syncPlatformTitleBarGeometry(QWidget* window, const WindowChromeOptions& options) {
    Q_UNUSED(window);
    Q_UNUSED(options);
}

} // namespace detail
} // namespace compatibility

#endif // Q_OS_WIN
