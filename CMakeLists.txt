cmake_minimum_required(VERSION 3.14)
project(testapp LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS ShaderTools)

qt_add_executable(testapp
    main.cpp
    rhiwidget.cpp rhiwidget.h rhiwidget_p.h
    examplewidget.cpp examplewidget.h cube.h
)
target_link_libraries(testapp PUBLIC
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
    Qt::WidgetsPrivate
)

qt_add_shaders(testapp "testapp-shaders"
    PREFIX
        "/"
    FILES
        "texture.vert"
        "texture.frag"
)
