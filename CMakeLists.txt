cmake_minimum_required(VERSION 3.16)
project(viewpro VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Поиск GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

# Поиск SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Путь к SDK
set(VIEWLINK_SDK_DIR "${CMAKE_SOURCE_DIR}/sdk")

add_executable(viewpro
    src/main.cpp
    src/ui/main_window.cpp
    src/ui/widgets/video/video_widget.cpp
    src/ui/widgets/transparent/transparent_widget.cpp
    src/ui/widgets/titles/close_widget.cpp
    src/ui/widgets/titles/hide_widget.cpp
    src/ui/widgets/titles/resize_widget.cpp
    src/ui/widgets/buttons/connection_button.cpp
    src/ui/widgets/buttons/telemetry_button.cpp
    src/ui/widgets/buttons/control_button.cpp
    src/gimbal/gimbal.cpp
    src/gimbal/video_stream.cpp
    src/gimbal/config_manager.cpp
    src/gimbal/control_stream.cpp
    src/gimbal/keyboard_handler.cpp
    src/gimbal/joystick_handler.cpp
    src/gimbal/command_handler.cpp
    src/gimbal/telemetry_stream.cpp
    src/gimbal/viewlink_stub.cpp
)

target_include_directories(viewpro PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${VIEWLINK_SDK_DIR}
    ${GSTREAMER_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(viewpro PRIVATE
    Qt6::Widgets
    ${GSTREAMER_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${VIEWLINK_SDK_DIR}/libViewLink.so
)

target_link_directories(viewpro PRIVATE
    ${GSTREAMER_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
    ${VIEWLINK_SDK_DIR}
)

# Копирование файла конфигурации в директорию сборки
configure_file(
    ${CMAKE_SOURCE_DIR}/gimbal_config.json
    ${CMAKE_BINARY_DIR}/gimbal_config.json
    COPYONLY
)
