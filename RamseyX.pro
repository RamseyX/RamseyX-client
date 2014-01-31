#-------------------------------------------------
#
# Project created by QtCreator 2014-01-27T16:06:59
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RamseyX
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    Graph.cpp \
    RamseyXController.cpp \
    RamseyXcURLWrapper.cpp \
    RamseyXTask.cpp \
    dhry_2.c \
    dhry_1.c \
    accountdialog.cpp \
    signupdialog.cpp
HEADERS += mainwindow.h \
    BitsetIterator.h \
    CPULimiter.h \
    dhry.h \
    Graph.h \
    RamseyXController.h \
    RamseyXcURLWrapper.h \
    RamseyXHeader.h \
    RamseyXTask.h \
    RamseyXUtils.h \
    accountdialog.h \
    signupdialog.h \
    signupthread.h \
    validateaccountthread.h \
    whatsupthread.h \
    refreshthread.h
FORMS += mainwindow.ui \
    accountdialog.ui \
    signupdialog.ui
RC_FILE += RamseyX.rc

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_RELEASE -= -O -O1 -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -flto
QMAKE_LFLAGS_RELEASE += -flto

win32: LIBS += -L$$PWD/libcurl/lib/ -llibcurl_imp

INCLUDEPATH += $$PWD/libcurl/include
DEPENDPATH += $$PWD/libcurl/include

RESOURCES += \
    resource.qrc


