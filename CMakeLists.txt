cmake_minimum_required(VERSION 3.16)
project(viewpro VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

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
    src/ui/widgets/panels/connection/connection_panel.cpp
    src/ui/widgets/panels/connection/ip_address_input.cpp
    src/ui/widgets/panels/connection/port_input.cpp
    src/ui/widgets/panels/connection/connect_toggle.cpp
    src/ui/widgets/panels/telemetry/telemetry_panel.cpp
    src/ui/widgets/panels/control/control_panel.cpp
    src/ui/widgets/panels/control/camera/subpanel_camera.cpp
    src/ui/widgets/panels/control/camera/toggle_switch.cpp
    src/ui/widgets/panels/control/camera/zoom_button.cpp
    src/ui/widgets/panels/control/lrf/subpanel_lrf.cpp
    src/ui/widgets/panels/control/movement/subpanel_movement.cpp
    src/ui/widgets/panels/control/movement/toggle_button.cpp
    src/ui/widgets/panels/control/movement/action_button.cpp
    src/ui/widgets/panels/control/movement/arrow_button.cpp
)

target_link_libraries(viewpro PRIVATE Qt6::Widgets)
