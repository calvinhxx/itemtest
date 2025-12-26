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

# items
add_qt_test_module(test_responsive_label items/TestResponsiveLabel.cpp)
add_qt_test_module(test_responsive_pushbutton items/TestResponsivePushbutton.cpp)
add_qt_test_module(test_responsive_dialog items/TestResponsiveDialog.cpp)
# layouts
add_qt_test_module(test_anchor_layout layouts/TestAnchorLayout.cpp)

