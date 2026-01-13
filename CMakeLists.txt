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
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

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
	add_subdirectory(third_party/googletest)
	add_subdirectory(tests)
endif()
