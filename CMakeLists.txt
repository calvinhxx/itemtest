cmake_minimum_required(VERSION 3.16)

project(itemstest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer a specific Qt installation when multiple versions exist.
# Usage:
#   cmake -B build-qt693 -DQT_ROOT="/Users/calvinhxx/Qt/6.9.3/macos && cmake --build build-qt693"
set(QT_ROOT "" CACHE PATH "Qt installation prefix to prefer (e.g. /Users/you/Qt/6.x.x/macos or /opt/homebrew/opt/qt).")
if(QT_ROOT)
	list(PREPEND CMAKE_PREFIX_PATH "${QT_ROOT}")
endif()
# Debug: print where Qt was found from (helps diagnose Conda/Homebrew/SDK conflicts)
message(STATUS "Using Qt major=${QT_VERSION_MAJOR}")
if(QT_VERSION_MAJOR EQUAL 6)
	message(STATUS "Qt6_DIR=${Qt6_DIR}")
else()
	message(STATUS "Qt5_DIR=${Qt5_DIR}")
endif()

if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:utf-8>")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# add source_files macro function
macro(source_group_by_dir source_files)
		set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR}/)
		foreach(sgbd_file ${${source_files}})
				string(REGEX REPLACE ${sgbd_cur_dir} "" sgbd_fpath ${sgbd_file})
				string(REGEX REPLACE "[^/\\\\]+$" "" sgbd_group_name ${sgbd_fpath})
				if(sgbd_group_name)
						string(REPLACE "/"   "\\\\"  sgbd_group_name_normal ${sgbd_group_name})
						source_group(${sgbd_group_name_normal} FILES ${sgbd_file})
				endif(sgbd_group_name)
		endforeach(sgbd_file)
endmacro(source_group_by_dir)

# find files recurse
file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
source_group_by_dir(SRC)
foreach(v ${SRC})
	message(STATUS ${v})
endforeach()

add_library(itemstest_lib STATIC ${SRC})
target_link_libraries(itemstest_lib PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(itemstest_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ut
enable_testing()
add_subdirectory(third_party/googletest)
add_subdirectory(tests)
