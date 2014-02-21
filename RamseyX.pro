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
    dhry.h \
    Graph.h \
    RamseyXController.h \
    RamseyXcURLWrapper.h \
    RamseyXHeader.h \
    RamseyXTask.h \
    RamseyXUtils.h \
    accountdialog.h \
    signupdialog.h \
    checkforupdateworker.h \
    whatsupworker.h \
    refresh20worker.h \
    refreshoverallworker.h \
    validateaccountworker.h \
    signupworker.h
FORMS += mainwindow.ui \
    accountdialog.ui \
    signupdialog.ui
RESOURCES += resource.qrc

INCLUDEPATH += $$PWD/../curl-7.35.0/include $$PWD/../boost_1_55_0
DEPENDPATH += $$PWD/../curl-7.35.0/include $$PWD/../boost_1_55_0
LIBS += -L$$PWD/../curl-7.35.0/lib/ -lcurldll
release: LIBS += -L$$PWD/../boost_1_55_0/stage/lib/ -lboost_atomic-mgw48-mt-1_55
debug: LIBS += -L$$PWD/../boost_1_55_0/stage/lib/ -lboost_atomic-mgw48-mt-d-1_55

VERSION = 5.0.1
QMAKE_TARGET_PRODUCT = RamseyX Client
QMAKE_TARGET_DESCRIPTION = RamseyX Client
DEFINES += RX_QT APP_VERSION=\\\"$$VERSION\\\"

gcc {
    QMAKE_CFLAGS += -std=c99 -Wall -Wextra -pedantic
    QMAKE_CFLAGS -= -O1 -O2 -O3 -flto # Prevent the compiler from optimizing Dhrystone code
    QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic
    QMAKE_CXXFLAGS_RELEASE -= -O -O1 -O2
    QMAKE_CXXFLAGS_RELEASE += -O3 -flto
    QMAKE_LFLAGS_RELEASE += -flto
}

msvc {
    DEFINES += _CRT_SECURE_NO_WARNINGS=1
    QMAKE_CXXFLAGS += /EHsc /Wall /GL
    QMAKE_CXXFLAGS_RELEASE += /O2
    QMAKE_LFLAGS_RELEASE += /LTCG
}

win32 {
    RC_FILE += RamseyX.rc
}

