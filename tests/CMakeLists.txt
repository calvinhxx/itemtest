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

# items (flat)
add_qt_test_module(test_label items/TestLabel.cpp)
add_qt_test_module(test_pushbutton items/TestPushButton.cpp)
add_qt_test_module(test_dialog items/TestDialog.cpp)
add_qt_test_module(test_popup items/TestPopup.cpp)

# layouts
add_qt_test_module(test_anchor_layout layouts/TestAnchorLayout.cpp)
