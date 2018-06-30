#-------------------------------------------------
#
# Project created by QtCreator 2018-03-25T18:49:49
#
#-------------------------------------------------

QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RpiClient
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

CONFIG += c++11

unix:!macx: LIBS += -lwiringPi -lpthread
#unix:!macx: LIBS += -lpigpio   USE THIS IF YOU WORK WITH PIGPIO

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    SettingsDialog.cpp

HEADERS += \
        mainwindow.h \
    SettingsDialog.h

FORMS += \
        mainwindow.ui \
    SettingsDialog.ui

RESOURCES += \
    rpiclientresources.qrc

# Set your rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /home/pi/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$OUT_PWD/../Core/ -lCore

INCLUDEPATH += $$PWD/../Core
DEPENDPATH += $$PWD/../Core
