#-------------------------------------------------
#
# Project created by QtCreator 2012-06-26T12:00:18
#
#-------------------------------------------------

QT += core gui opengl

TARGET = yarrh
TEMPLATE = app

INCLUDEPATH += "./"

SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    gcodeobject.cpp \
    layer.cpp \
    graphwidget.cpp \
    headcontrol.cpp \
    headcontrolline.cpp \
    printer.cpp \
    aboutwindow.cpp \
    layer2d.cpp \
    calibratedialog.cpp \
    optiondialog.cpp \
    slicedialog.cpp \
    stlview.cpp \
    stlobject.cpp \
    face.cpp \
    halfedge.cpp \
    vertex.cpp \
    slice.cpp \
    material.cpp \
    clipper/clipper.cpp \
    macroswindow.cpp \
    macrobutton.cpp \
    sdcardwindow.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    gcodeobject.h \
    layer.h \
    graphwidget.h \
    headcontrol.h \
    headcontrolline.h \
    printer.h \
    aboutwindow.h \
    layer2d.h \
    calibratedialog.h \
    optiondialog.h \
    slicedialog.h \
    stlview.h \
    stlobject.h \
    face.h \
    halfedge.h \
    vertex.h \
    slice.h \
    material.h \
    clipper/clipper.hpp \
    macroswindow.h \
    macrobutton.h \
    sdcardwindow.h

FORMS    += mainwindow.ui \
    aboutwindow.ui \
    calibratedialog.ui \
    optiondialog.ui \
    slicedialog.ui \
    macroswindow.ui \
    sdcardwindow.ui

RESOURCES += \
    resource.qrc

TRANSLATIONS = yarrh_pl.ts \
               yarrh_en.ts \
               yarrh_de.ts \
               yarrh_bg.ts \
               yarrh_fi.ts


include(qextserialport/qextserialport.pri)
include(qtsingleapplication\src\qtsingleapplication.pri)
win32 {
#LIBS+=-lglut32
    RC_FILE = icon.rc
}

macx {
    ICON = icon.icns
}

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR = build








































