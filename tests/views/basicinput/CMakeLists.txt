add_qt_test_module(test_button TestButton.cpp)
add_qt_test_module(test_dropdown_button TestDropDownButton.cpp)
add_qt_test_module(test_hyperlink_button TestHyperlinkButton.cpp)
add_qt_test_module(test_repeat_button TestRepeatButton.cpp)
add_qt_test_module(test_toggle_button TestToggleButton.cpp)
add_qt_test_module(test_split_button TestSplitButton.cpp)
add_qt_test_module(test_toggle_split_button TestToggleSplitButton.cpp)
add_qt_test_module(test_checkbox TestCheckBox.cpp)
add_qt_test_module(test_radio_button TestRadioButton.cpp)
add_qt_test_module(test_color_picker TestColorPicker.cpp)
add_qt_test_module(test_slider TestSlider.cpp)
add_qt_test_module(test_rating_control TestRatingControl.cpp)
add_qt_test_module(test_toggle_switch TestToggleSwitch.cpp)
# ComboBox 下拉使用 ListView；FluentListItemDelegate 仅在测试中组装（与 test_list_view 共用实现）
add_executable(test_combo_box
    TestComboBox.cpp
    ../collections/FluentListItemDelegate.cpp
)
target_link_libraries(test_combo_box
    PRIVATE
    itemstest_lib
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
    GTest::gtest_main
)
target_include_directories(test_combo_box PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/tests/views/collections
)
gtest_discover_tests(test_combo_box PROPERTIES ENVIRONMENT "SKIP_VISUAL_TEST=1")
