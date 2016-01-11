TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../../src
TARGET = $$qtLibraryTarget(oc)
PLUGIN_TYPE=cacheprovider

include(../common.pri)

QT += network

# this is kind of ugly, but we really need the cache class here as well
HEADERS += json.h oc.h ../../../src/cache.h ../../../src/cachelist.h ../../../src/cacheproviderplugin.h
SOURCES += json.cpp oc.cpp ../../../src/cache.cpp ../../../src/cachelist.cpp

symbian: {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = Location NetworkServices
    #TARGET.UID3 = 0x20045CF8
}
