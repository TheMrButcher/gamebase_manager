#-------------------------------------------------
#
# Project created by QtCreator 2016-07-14T19:59:00
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = manager
TEMPLATE = app

INCLUDEPATH += ../zlib
LIBS += -L../zlib -lz

INCLUDEPATH += ../quazip/quazip
LIBS += -L../quazip/quazip/release -lquazip

SOURCES += main.cpp\
        mainwindow.cpp \
    aboutwindow.cpp \
    version.cpp \
    settings.cpp \
    settingsform.cpp \
    librarysourcestablemodel.cpp \
    appsourcestablemodel.cpp \
    librariesform.cpp \
    librariestablemodel.cpp \
    library.cpp \
    githublibrarydownloader.cpp \
    librarysourcemanager.cpp \
    directorysourcemanager.cpp \
    librarysourcemanagerlist.cpp \
    workingdirmanager.cpp \
    librarysource.cpp \
    files.cpp \
    librarydeployer.cpp \
    archive.cpp \
    libraryremover.cpp \
    app.cpp \
    appstablemodel.cpp \
    appsform.cpp \
    librarycopier.cpp \
    appsource.cpp \
    appsourcemanager.cpp \
    appsourcemanagerlist.cpp \
    appconfigurationdialog.cpp \
    appconfig.cpp \
    filesmanager.cpp \
    progressmanager.cpp \
    largefilemanager.cpp \
    newappdialog.cpp \
    appcompressor.cpp \
    appcompressiondialog.cpp \
    appadder.cpp \
    appdeployer.cpp \
    compiler.cpp \
    appdeploysuccessdialog.cpp

HEADERS  += mainwindow.h \
    aboutwindow.h \
    version.h \
    settingsform.h \
    settings.h \
    librarysource.h \
    librarysourcestablemodel.h \
    sourcestatus.h \
    appsourcestablemodel.h \
    appsource.h \
    librariesform.h \
    library.h \
    librariestablemodel.h \
    githublibrarydownloader.h \
    librarysourcemanager.h \
    directorysourcemanager.h \
    librarysourcemanagerlist.h \
    workingdirmanager.h \
    files.h \
    librarydeployer.h \
    archive.h \
    libraryremover.h \
    app.h \
    appstablemodel.h \
    appsform.h \
    librarycopier.h \
    appsourcemanager.h \
    appsourcemanagerlist.h \
    appconfigurationdialog.h \
    appconfig.h \
    filesmanager.h \
    progressmanager.h \
    largefilemanager.h \
    newappdialog.h \
    appcompressor.h \
    appcompressiondialog.h \
    appadder.h \
    appdeployer.h \
    compiler.h \
    appdeploysuccessdialog.h

FORMS    += mainwindow.ui \
    aboutwindow.ui \
    settingsform.ui \
    librariesform.ui \
    appsform.ui \
    appconfigurationdialog.ui \
    newappdialog.ui \
    appcompressiondialog.ui \
    appdeploysuccessdialog.ui

RESOURCES += \
    resources.qrc
