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
    gtest_discover_tests(${module_name} PROPERTIES ENVIRONMENT "SKIP_VISUAL_TEST=1")
endmacro()

# VisualCheck 测试使用 qApp->exec() 阻塞，需手动关闭窗口。
# 直接运行二进制（Qt Creator 中运行）时，可视化测试默认执行。
# ctest 自动化运行时，通过 ENVIRONMENT 属性注入 SKIP_VISUAL_TEST=1，自动跳过。
# 运行方式：
#   ctest --verbose                                              ← 跑全部单元测试，VisualCheck 自动 SKIP
#   ./build/bin/test_xxx                                         ← 启动某个组件的全部测试（含可视化）
#   ./build/bin/test_xxx --gtest_filter="*VisualCheck*"         ← 只跑可视化测试

# 按源码目录结构添加子目录，Qt Creator 工程树会呈现层级
add_subdirectory(views)
