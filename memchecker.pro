TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    memchecker.cpp
HEADERS += \ 
    memchecker.h

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
