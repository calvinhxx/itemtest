cmake_minimum_required(VERSION 3.16)

project(itemstest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Enable testing and build tests" ON)

if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test Network)

# Enforce minimum supported Qt versions
if(QT_VERSION_MAJOR EQUAL 5 AND Qt5_VERSION VERSION_LESS 5.15)
	message(FATAL_ERROR "Qt 5.15 or newer is required (found ${Qt5_VERSION})")
endif()
if(QT_VERSION_MAJOR EQUAL 6 AND Qt6_VERSION VERSION_LESS 6.2)
	message(FATAL_ERROR "Qt 6.2 or newer is required (found ${Qt6_VERSION})")
endif()
message(STATUS "Building with Qt ${QT_VERSION_MAJOR} (${Qt${QT_VERSION_MAJOR}_VERSION})")

# MSVC CRT 必须与所链接的 Qt 二进制保持一致：
#   - Qt5 官方 Windows 预编译包使用动态 CRT (/MD, /MDd)，否则会出现
#     `__acrt_first_block == header` 等跨 CRT heap 断言/崩溃。
#   - Qt6 官方包同样用 /MD；保持 /MD 最安全，避免 CRT 不匹配。
# 如需 /MT 静态运行时，必须使用同样以 /MT 编译的 Qt 二进制（自行重编）。
if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
endif()

# find files recurse
file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
foreach(v ${SRC})
	message(STATUS ${v})
endforeach()
set(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${SRC})

add_library(itemstest_lib STATIC ${SRC} ${RESOURCES})
target_link_libraries(itemstest_lib PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(itemstest_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ut
if(BUILD_TESTING)
	enable_testing()
	# GoogleTest 跟随项目使用动态 CRT (/MD)，与 Qt 官方二进制保持一致。
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	add_subdirectory(third_party/googletest)
	add_subdirectory(tests)
endif()
