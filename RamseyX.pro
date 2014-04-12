#-------------------------------------------------
#
# Project created by QtCreator 2014-01-27T16:06:59
#
#-------------------------------------------------

QT += core gui network #sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ramseyx
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    dhry_2.c dhry_1.c \
    accountdialog.cpp \
    signupdialog.cpp \
    graph.cpp \
    ramseyxcontroller.cpp \
    ramseyxcurlwrapper.cpp \
    ramseyxtask.cpp
HEADERS += mainwindow.h \
    dhry.h \
    accountdialog.h \
    signupdialog.h \
    checkforupdateworker.h \
    whatsupworker.h \
    refresh20worker.h \
    refreshoverallworker.h \
    validateaccountworker.h \
    signupworker.h \
    bitsetiterator.h \
    graph.h \
    ramseyxcontroller.h \
    ramseyxcurlwrapper.h \
    ramseyxutils.h \
    ramseyxtask.h \
    ramseyxdefs.h
FORMS += mainwindow.ui \
    accountdialog.ui \
    signupdialog.ui
RESOURCES += resource.qrc
OTHER_FILES += ramseyx.rc \
    COPYING \
    README.md

VERSION = 5.0.3
QMAKE_TARGET_PRODUCT = RamseyX Client
QMAKE_TARGET_DESCRIPTION = RamseyX Client
DEFINES += RX_QT APP_VERSION=\\\"$$VERSION\\\"

# Specify the paths of your boost (unnecessary for MSVC) and libcurl libraries below
CONFIG(debug, debug|release) {
    CURL_DIR = "C:/Dev/Libraries/curl-7.35.0_msvc2013_x86/builds/libcurl-vc12-x86-debug-dll-ipv6-sspi-spnego-winssl"
    BOOST_DIR = "C:/Dev/Libraries/boost_1_55_0"
}
CONFIG(release, debug|release) {
    CURL_DIR = "C:/Dev/Libraries/curl-7.35.0_msvc2013_x86/builds/libcurl-vc12-x86-release-dll-ipv6-sspi-spnego-winssl"
    BOOST_DIR = "C:/Dev/Libraries/boost_1_55_0"
}

win32 {
    DEFINES += NOMINMAX
    RC_FILE += ramseyx.rc
}

msvc {
    DEFINES += _CRT_SECURE_NO_WARNINGS

    # Configure this for your Windows SDKs
    INCLUDEPATH += "%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Include"
    LIBS += -L"%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Lib" -lshell32

    # Configure this for your libcurl library
    CONFIG(debug, debug|release) {
        INCLUDEPATH += "$$CURL_DIR/include"
        LIBS += -L"$$CURL_DIR/lib" -llibcurl_debug
    }
    # Configure this for your libcurl library
    CONFIG(release, debug|release) {
        INCLUDEPATH += "$$CURL_DIR/include"
        LIBS += -L"$$CURL_DIR/lib" -llibcurl
    }

    QMAKE_CXXFLAGS += /EHsc /W4 /WX
    QMAKE_CXXFLAGS_RELEASE += /O2 /GL

    QMAKE_LFLAGS = /WX
    QMAKE_LFLAGS_RELEASE += /LTCG
}

gcc|clang {
    # Configure this for your boost:atomic and libcurl libraries
    INCLUDEPATH += "$$CURL_DIR/include" "$$BOOST_DIR" \
	"/usr/local/include" "/usr/include" "/usr/share/include" "/opt/local/include"
    LIBS += -L"$$CURL_DIR/lib" -L"/usr/local/lib" -L"/usr/share/lib" -L"/usr/lib" -L"/opt/local/lib"  -lcurl

    # Configure this for your boost:atomic library
    CONFIG(debug, debug|release) {
        LIBS += -L"$$BOOST_DIR/stage/lib" -lboost_atomic-mt-d
    }
    # Configure this for your boost:atomic library
    CONFIG(release, debug|release) {
        LIBS += -L"$$BOOST_DIR/stage/lib" -lboost_atomic-mt
    }

    QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic
    win32: QMAKE_CXXFLAGS_DEBUG += -static -ggdb # For memory check
    QMAKE_CXXFLAGS_RELEASE -= -O -O1 -O2 -Os
    QMAKE_CXXFLAGS_RELEASE += -O3 -flto

    win32: QMAKE_LFLAGS_DEBUG += -static -ggdb # For memory check
    QMAKE_LFLAGS_RELEASE += -flto
}

