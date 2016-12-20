#-------------------------------------------------
#
# Project created by QtCreator 2016-11-18T09:40:16
#
#-------------------------------------------------

QT       += core gui widgets
QT       += network widgets
QT       += sql
QT       += testlib
#QT       += thread

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FaceRecManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    adddevice.cpp \
    facemanager.cpp \
    devicemanager.cpp \
    mysocketthread.cpp \
    accessrecord.cpp \
    videostream.cpp \
    devicesetting.cpp

HEADERS  += mainwindow.h \
    doormessage.h \
    adddevice.h \
    facemanager.h \
    devicemanager.h \
    mysocketthread.h \
    accessrecord.h \
    videostream.h \
    devicesetting.h

HEADERS += tcustomthread.h

FORMS    += mainwindow.ui \
    ADDDevice.ui \
    facemanager.ui \
    devicemanager.ui \
    accessrecord.ui \
    videostream.ui \
    devicesetting.ui

CONFIG += console
