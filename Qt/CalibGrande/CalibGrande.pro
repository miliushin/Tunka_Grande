#-------------------------------------------------
#
# Project created by QtCreator 2023-11-07T13:45:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CalibGrande
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /home/ilushin/programs/root/include
LIBS += -L/home/ilushin/programs/root/lib -lCore -lRIO -lNet \
        -lHist -lGraf -lGraf3d -lGpad -lTree \
        -lRint -lPostscript -lMatrix -lPhysics \
        -lGui -lRGL -Wl,-rpath,/home/ilushin/programs/root/lib



SOURCES += \
        main.cpp \
        calibration.cpp \
        mainwindow.cpp \
        configuration.cpp \
    runprog.cpp


HEADERS += \
        mainwindow.h \
        calibration.h \
        configuration.h \
        global.h \
    runprog.h

FORMS += \
        mainwindow.ui
