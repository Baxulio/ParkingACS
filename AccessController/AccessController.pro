#-------------------------------------------------
#
# Project created by QtCreator 2018-05-12T12:54:09
#
#-------------------------------------------------

QT       += core gui sql network

CONFIG  += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QTPLUGIN += gif

TARGET = AccessController
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

SOURCES += \
        main.cpp \
        MainWindow.cpp \
    dialogs/SettingsDialog.cpp \
    CustomServer.cpp \
    StatusForm.cpp

HEADERS += \
        MainWindow.h \
    dialogs/SettingsDialog.h \
    CustomServer.h \
    StatusForm.h

FORMS += \
        MainWindow.ui \
    dialogs/SettingsDialog.ui \
    StatusForm.ui

# Set your rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /home/pi/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    accesscontrollerresources.qrc


unix|win32: LIBS += -lopenalpr
unix:!macx: LIBS += -lwiringPi
#unix:!macx: LIBS += -lpigpio   USE THIS IF YOU WORK WITH PIGPIO

unix:!macx: LIBS += -L$$OUT_PWD/../Core/ -lCore

INCLUDEPATH += $$PWD/../Core
DEPENDPATH += $$PWD/../Core
