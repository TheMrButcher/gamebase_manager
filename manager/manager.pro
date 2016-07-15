#-------------------------------------------------
#
# Project created by QtCreator 2016-07-14T19:59:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = manager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    aboutwindow.cpp \
    version.cpp \
    settings.cpp \
    settingsform.cpp \
    librarysourcestablemodel.cpp \
    appsourcestablemodel.cpp

HEADERS  += mainwindow.h \
    aboutwindow.h \
    version.h \
    settingsform.h \
    settings.h \
    librarysource.h \
    librarysourcestablemodel.h \
    sourcestatus.h \
    appsourcestablemodel.h \
    appsource.h

FORMS    += mainwindow.ui \
    aboutwindow.ui \
    settingsform.ui

RESOURCES += \
    resources.qrc
