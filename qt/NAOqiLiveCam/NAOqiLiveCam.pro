#-------------------------------------------------
#
# Project created by QtCreator 2015-03-24T19:05:31
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NAOqiLiveCam
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    audiooutput.cpp

HEADERS  += mainwindow.h NAOqi/nao_interface/nao_interface.h \
    audiooutput.h

FORMS    += mainwindow.ui


LIBS += -L"$$PWD/../build/lib" -lNaoInterface
