#include <gtest/gtest.h>
#include <QtGlobal>
#include <QEvent>
#include <type_traits>

#include "compatibility/QtCompat.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

TEST(QtCompat, FluentEnterEventDerivesFromQEvent) {
    static_assert(std::is_base_of<QEvent, FluentEnterEvent>::value,
                  "FluentEnterEvent must derive from QEvent");
    SUCCEED();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
TEST(QtCompat, FluentEnterEventMatchesQt6Type) {
    static_assert(std::is_same<FluentEnterEvent, QEnterEvent>::value,
                  "On Qt6, FluentEnterEvent must alias QEnterEvent");
    SUCCEED();
}
#else
TEST(QtCompat, FluentEnterEventMatchesQt5Type) {
    static_assert(std::is_same<FluentEnterEvent, QEvent>::value,
                  "On Qt5, FluentEnterEvent must alias QEvent");
    SUCCEED();
}
#endif

TEST(QtCompat, QtVersionMacrosDefined) {
    EXPECT_GT(QT_VERSION, 0);
    EXPECT_GE(QT_VERSION_MAJOR, 5);
}
