cmake_minimum_required(VERSION 4.1)
project(ward)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    # 优先查找 Qt，如果找不到且路径存在，则使用硬编码路径
    if (NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "E:/compiler/Qt/6.9.2/mingw_64")
        set(CMAKE_PREFIX_PATH "E:/compiler/Qt/6.9.2/mingw_64")
    endif ()
endif ()

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        MultimediaWidgets
        REQUIRED)

file(GLOB_RECURSE SRC_FILES
        "src/*.cpp"
        "src/*.h"
)
set(RESOURCES "resources/resources.qrc")

find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})

add_executable(ward ${SRC_FILES} ${RESOURCES})

target_link_libraries(ward
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        ${OpenCV_LIBS}
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()

    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_INSTALL_PATH}/bin")
    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-opengl-sw
                --no-translations
                "$<TARGET_FILE:${PROJECT_NAME}>"
                COMMENT "Running windeployqt to copy dependencies..."
        )
    else ()
        message(WARNING "windeployqt not found, skipping deployment step.")
    endif ()
endif ()
