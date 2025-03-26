#-------------------------------------------------
#
# Project created for combined Analog Input and Output
#
#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AO_AI_Combined
TEMPLATE = app

INCLUDEPATH += ../../inc/bdaqctrl.h
           	   

SOURCES += configuredialog.cpp\
           main.cpp\
           combinedaoai.cpp\
           ../common/simplegraph.cpp

HEADERS += configuredialog.h\
           combinedaoai.h\
           ../common/simplegraph.h\
           ../common/WaveformGenerator.h\

FORMS   += configuredialog.ui \ 
           combinedaoai.ui

RESOURCES += aoairesources.qrc

CONFIG += debug_and_release

CONFIG(debug, debug|release){
        DESTDIR += $$PWD/../bin/debug
        OBJECTS_DIR = $$PWD/debug
        UI_DIR      = $$PWD/debug/ui
        MOC_DIR     = $$PWD/debug/moc
        RCC_DIR     = $$PWD/debug/rcc

} else {
        DESTDIR += $$PWD/../bin/release
        OBJECTS_DIR = $$PWD/release
        UI_DIR      = $$PWD/release/ui
        MOC_DIR     = $$PWD/release/moc
        RCC_DIR     = $$PWD/release/rcc
}

unix: LIBS += -lbiodaq
