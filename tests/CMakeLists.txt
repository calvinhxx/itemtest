include(GoogleTest)

# 定义一个便捷的宏：添加一个新的 Qt GTest 模块（可在子目录中调用）
# 可执行文件输出到当前 CMake 子目录，便于按目录结构归类
macro(add_qt_test_module module_name source_file)
    add_executable(${module_name} ${source_file})
    get_filename_component(_test_subdir ${source_file} DIRECTORY)
    if(_test_subdir)
        set_target_properties(${module_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_test_subdir}"
        )
    endif()
    target_link_libraries(${module_name}
        PRIVATE
        itemstest_lib
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        GTest::gtest_main
    )
    target_include_directories(${module_name} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
    gtest_discover_tests(${module_name})
endmacro()

# 按源码目录结构添加子目录，Qt Creator 工程树会呈现层级
add_subdirectory(views)
