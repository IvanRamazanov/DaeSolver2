TEMPLATE += app
QT += core gui widgets svgwidgets printsupport
HEADERS += $$files(src/*.h, true)
SOURCES +=  $$files("src/*.cpp", true)
RESOURCES = DaeSolver.qrc

CONFIG += c++20 console

CONFIG(debug, debug|release) {
    message("Building Debug version")
    DEFINES += DEBUG
    OBJECTS_DIR = $$PWD/build/debug/obj
}
CONFIG(release, debug|release) {
    message("Building Release version")
    OBJECTS_DIR = $$PWD/build/release/obj
}

MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR
