#-------------------------------------------------
#
# Project created by QtCreator 2015-05-20T14:28:04
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = test_hound_jit
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG += c++11

DEFINES += QT_NO_KEYWORDS

SOURCES += main.cpp \
    expression.cpp \
    parser.cpp \
    compiler.cpp \
    virtualmachine.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../asmjit/release/ -lasmjit
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../asmjit/debug/ -lasmjit
else:unix: LIBS += -L$$PWD/../../asmjit/ -lasmjit

INCLUDEPATH += $$PWD/../../asmjit/src
DEPENDPATH += $$PWD/../../asmjit

HEADERS += \
    expression.h \
    parser.h \
    compiler.h \
    virtualmachine.h

RESOURCES += \
    resources.qrc
