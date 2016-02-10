QT += core
QT -= gui

TARGET = memchecker
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp dmemalloc.cpp

