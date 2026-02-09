include(GoogleTest)

# 定义一个便捷的宏：添加一个新的 Qt GTest 模块
macro(add_qt_test_module module_name source_file)
    add_executable(${module_name} ${source_file})
    target_link_libraries(${module_name} 
        PRIVATE 
        itemstest_lib
        Qt${QT_VERSION_MAJOR}::Widgets
        GTest::gtest_main
    )
    target_include_directories(${module_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR})
    # 自动将该执行文件中的所有 TEST_F 注册到 CTest 系统
    gtest_discover_tests(${module_name})
endmacro()

# views
add_qt_test_module(test_anchor_layout views/TestAnchorLayout.cpp)
add_qt_test_module(test_fluent_element views/TestFluentElement.cpp)
add_qt_test_module(test_qml_plus views/TestQMLPlus.cpp)
add_qt_test_module(test_segoe views/TestSegoe.cpp)
add_qt_test_module(test_menu_bar views/menus_toolbars/TestMenuBar.cpp)

# basicinput
add_qt_test_module(test_button views/basicinput/TestButton.cpp)
add_qt_test_module(test_dropdown_button views/basicinput/TestDropDownButton.cpp)
add_qt_test_module(test_hyperlink_button views/basicinput/TestHyperlinkButton.cpp)
add_qt_test_module(test_repeat_button views/basicinput/TestRepeatButton.cpp)

# textfields
add_qt_test_module(test_label views/textfields/TestLabel.cpp)

# dialogs_flyouts
add_qt_test_module(test_dialog views/dialogs_flyouts/TestDialog.cpp)
