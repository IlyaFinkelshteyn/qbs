TEMPLATE = app
DESTDIR = ../../../bin

CONFIG += console
CONFIG -= app_bundle qt

TARGET = RezDoc

OBJECTIVE_SOURCES += main.m
LIBS += -framework Cocoa

target.path = $${QBS_INSTALL_PREFIX}/bin
INSTALLS += target
