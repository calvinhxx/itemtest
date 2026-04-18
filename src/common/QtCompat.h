#pragma once

// Qt5/Qt6 compatibility layer for the Fluent Design Qt component library.
// Centralizes API differences between Qt 5.15+ and Qt 6.2+.

#include <QtGlobal>
#include <QEvent>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
using FluentEnterEvent = QEnterEvent;
#else
using FluentEnterEvent = QEvent;
#endif

#include <type_traits>
static_assert(std::is_base_of<QEvent, FluentEnterEvent>::value,
              "FluentEnterEvent must derive from QEvent");
