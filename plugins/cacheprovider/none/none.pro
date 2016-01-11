TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../../src
TARGET = $$qtLibraryTarget(none)
PLUGIN_TYPE=cacheprovider

include(../common.pri)

# this is kind of ugly, but we really need the cache class here as well
HEADERS += none.h ../../../src/cache.h ../../../src/cachelist.h ../../../src/cacheproviderplugin.h
SOURCES += none.cpp ../../../src/cache.cpp ../../../src/cachelist.cpp

symbian: {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = Location NetworkServices
    #TARGET.UID3 = 0x20045CF8
}
