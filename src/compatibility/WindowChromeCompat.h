#ifndef WINDOWCHROMECOMPAT_H
#define WINDOWCHROMECOMPAT_H

#include <QByteArray>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QVector>
#include <QWidget>
#include <QtGlobal>

namespace compatibility {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using FluentNativeEventResult = qintptr;
#else
using FluentNativeEventResult = long;
#endif

struct WindowChromeOptions {
    QRect titleBarRect;
    QVector<QRect> dragExclusionRects;
    int resizeBorderWidth = 8;
    bool useCustomWindowChrome = false;
    bool preferNativeMacControls = true;
};

class WindowChromeCompat {
public:
    enum class Platform {
        Windows,
        MacOS,
        Other
    };

    enum class HitTest {
        Client,
        Caption,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    explicit WindowChromeCompat(QWidget* window);

    void configure(const WindowChromeOptions& options);
    const WindowChromeOptions& options() const { return m_options; }

    void applyPlatformWindowFlags();
    bool handleNativeEvent(const QByteArray& eventType,
                           void* message,
                           FluentNativeEventResult* result);

    bool beginSystemMove(const QPoint& globalPos);
    bool beginSystemResize(Qt::Edges edges, const QPoint& globalPos);
    bool performTitleBarDoubleClick();

    bool usesCustomWindowChrome() const;
    bool prefersNativeMacControls() const;
    int nativeTitleBarLeadingInset() const;
    QWidget* window() const { return m_window; }

    HitTest hitTestLocal(const QPoint& localPos) const;

    static Platform currentPlatform();
    static bool platformPrefersCustomWindowChrome();
    static HitTest classifyHitTest(const WindowChromeOptions& options,
                                   const QSize& windowSize,
                                   const QPoint& localPos);

private:
    QWidget* m_window = nullptr;
    WindowChromeOptions m_options;
};

} // namespace compatibility

#endif // WINDOWCHROMECOMPAT_H
