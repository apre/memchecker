TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CC = gcc
GTEST_DIR=$$(GTEST_DIR)

INCLUDEPATH+=$$GTEST_DIR $$GTEST_DIR/include

SOURCES += main.cpp \
    memchecker.cpp \
    $$GTEST_DIR/src/gtest-all.cc \
    tests/tst_c.c \
    tests/tst_c_interfacing.cpp \

HEADERS += \ 
    memchecker.

LIBS += -lpthread

CONFIG += debug
DEFINES += DEBUG

#Release:DEFINES += NDEBUG
#Debug:DEFINES += DEBUG
#Debug:DESTDIR = debug
#Release:DESTDIR = release


#CONFIG += release

#CONFIG(debug, debug|release) {
#CONFIG-=release
#CONFIG += debug
#DEFINES += DEBUG
#}

#CONFIG(release, debug|release) {
#CONFIG -= debug
#CONFIG+=release
#DEFINES += NDEBUG
#}
