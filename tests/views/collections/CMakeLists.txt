# ListView 仅为视图库组件；FluentListItemDelegate 与 model 组装放在测试中作为业务示例。
add_executable(test_list_view
    TestListView.cpp
    FluentListItemDelegate.cpp
)
set_target_properties(test_list_view PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(test_list_view
    PRIVATE
    itemstest_lib
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Test
    GTest::gtest_main
)
target_include_directories(test_list_view PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)
include(GoogleTest)
gtest_discover_tests(test_list_view PROPERTIES ENVIRONMENT "SKIP_VISUAL_TEST=1")
