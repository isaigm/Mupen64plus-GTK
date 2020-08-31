TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -rdynamic
SOURCES += \
        main.c
unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += gtk+-3.0
LIBS += -lzip
