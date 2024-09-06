cmake_minimum_required(VERSION 3.16)

project(WaveformViewer VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型为 Debug，启用调试信息
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# 查找Qt库
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# 设置共享库目录
set(SHARED_LIB_SUBDIR "${CMAKE_SOURCE_DIR}/SharedLibrary/Windows/X64/Debug")

# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/SharedLibrary
)

# 启用Qt的自动化功能
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 链接目录
link_directories(${SHARED_LIB_SUBDIR})
# 设置资源文件
set(RESOURCE_FILES resources.qrc)
# 自动查找所有源文件和头文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCE_FILES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)

# 链接共享库
if(EXISTS "${SHARED_LIB_SUBDIR}/vmmso.lib")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SHARED_LIB_SUBDIR}/vmmso.lib)
elseif(EXISTS "${SHARED_LIB_SUBDIR}/vmmso.dll")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SHARED_LIB_SUBDIR}/vmmso.dll)
endif()

# 添加调试信息（仅在 Debug 模式下）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
endif()