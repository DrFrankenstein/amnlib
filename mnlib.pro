# -------------------------------------------------
# Project created by QtCreator 2009-06-07T01:28:36
# -------------------------------------------------
QT += network \
    script
QT -= widgets
TARGET = mnlib
TEMPLATE = lib
DEFINES += MNLIB_LIBRARY
SOURCES += damnsession.cpp \
    damnpacket.cpp \
    mnlib_global.cpp \
    events.cpp \
    damnchatroom.cpp \
    damnprivclass.cpp \
    timespan.cpp \
    deviant.cpp \
    damnuser.cpp \
    damnobject.cpp \
    damnpacketparser.cpp \
    damnpacketdevice.cpp \
    scrapingauthenticationprovider.cpp \
    damnrichtext.cpp
HEADERS += damnsession.h \
    mnlib_global.h \
    damnpacket.h \
    events.h \
    damnchatroom.h \
    damnprivclass.h \
    timespan.h \
    deviant.h \
    damnuser.h \
    evtfwd.h \
    damnobject.h \
    damnpacketparser.h \
    damnpacketdevice.h \
    scrapingauthenticationprovider.h \
    damnrichtext.h
debug:DEFINES += MNLIB_DEBUG_BUILD
else:DEFINES += MNLIB_RELEASE_BUILD

VERSION = 0.1.1402
VER_MAJ = 0
VER_MIN = 1
VER_PAT = 1402
