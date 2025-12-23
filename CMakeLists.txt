cmake_minimum_required(VERSION 3.16)

project(itemstest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_executable(itemstest
    ${SRC}
)

target_link_libraries(itemstest PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

