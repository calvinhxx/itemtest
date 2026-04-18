#pragma once

// Qt5/Qt6 compatibility layer for the Fluent Design Qt component library.
// Centralizes API differences between Qt 5.15+ and Qt 6.2+.
//
// Usage:
//   - Use FluentEnterEvent in `enterEvent()` overrides instead of QEvent / QEnterEvent.
//   - Use fluentMousePos(e) / fluentMouseGlobalPos(e) for mouse coordinates.
//   - Use FLUENT_INIT_VIEW_ITEM_OPTION(opt) in QAbstractItemView subclasses.

#include <QtGlobal>
#include <QEvent>
#include <QPoint>
#include <QMouseEvent>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
using FluentEnterEvent = QEnterEvent;
#else
using FluentEnterEvent = QEvent;
#endif

#include <type_traits>
static_assert(std::is_base_of<QEvent, FluentEnterEvent>::value,
              "FluentEnterEvent must derive from QEvent");

// ── Mouse event coordinates ─────────────────────────────────────────────────
// Qt6: QMouseEvent::position() / globalPosition() return QPointF.
// Qt5: QMouseEvent::pos() / globalPos() return QPoint.
inline QPoint fluentMousePos(const QMouseEvent* e) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position().toPoint();
#else
    return e->pos();
#endif
}

inline QPoint fluentMouseGlobalPos(const QMouseEvent* e) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->globalPosition().toPoint();
#else
    return e->globalPos();
#endif
}

// ── QAbstractItemView::initViewItemOption ───────────────────────────────────
// Qt6: protected void QAbstractItemView::initViewItemOption(QStyleOptionViewItem*) const
// Qt5: protected QStyleOptionViewItem QAbstractItemView::viewOptions() const
//
// Use inside a QAbstractItemView subclass:
//   QStyleOptionViewItem opt;
//   FLUENT_INIT_VIEW_ITEM_OPTION(&opt);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define FLUENT_INIT_VIEW_ITEM_OPTION(optPtr) initViewItemOption(optPtr)
#else
#define FLUENT_INIT_VIEW_ITEM_OPTION(optPtr) do { *(optPtr) = viewOptions(); } while (0)
#endif

// ── QColor::getHsvF / getRgbF / getHslF component type ──────────────────────
// Qt6: takes float*
// Qt5: takes qreal* (= double*)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using FluentColorComponent = float;
#else
using FluentColorComponent = qreal;
#endif

// ── Construct an enter event for tests ──────────────────────────────────────
// Qt6: QEnterEvent(localPos, scenePos, globalPos) all QPointF
// Qt5: QEnterEvent doesn't have that ctor; use a QEvent(Enter) instead.
//
// Usage:
//   FLUENT_MAKE_ENTER_EVENT(ev, 5, 5);
//   QApplication::sendEvent(widget, &ev);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define FLUENT_MAKE_ENTER_EVENT(name, x, y) \
    QEnterEvent name(QPointF((x), (y)), QPointF((x), (y)), QPointF((x), (y)))
#else
#define FLUENT_MAKE_ENTER_EVENT(name, x, y) \
    QEvent name(QEvent::Enter)
#endif
