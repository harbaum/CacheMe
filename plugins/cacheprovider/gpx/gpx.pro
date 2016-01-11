TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../../src ./quazip-0.3/quazip
TARGET = $$qtLibraryTarget(gpx)
PLUGIN_TYPE=cacheprovider
QUAZIP=./quazip-0.3/quazip

include(../common.pri)

# this is kind of ugly, but we really need the cache class here as well
HEADERS += gpx.h ../../../src/cache.h ../../../src/cachelist.h ../../../src/cacheproviderplugin.h
SOURCES += gpx.cpp ../../../src/cache.cpp ../../../src/cachelist.cpp
HEADERS += gpxparser.h gpxfileparser.h
SOURCES += gpxparser.cpp gpxfileparser.cpp

# Include local Quazip for ZIP handling
HEADERS += $$QUAZIP/crypt.h $$QUAZIP/quachecksum32.h $$QUAZIP/quazipfileinfo.h $$QUAZIP/quazipnewinfo.h $$QUAZIP/zip.h $$QUAZIP/ioapi.h $$QUAZIP/quacrc32.h $$QUAZIP/quaadler32.h $$QUAZIP/quazipfile.h $$QUAZIP/quazip.h $$QUAZIP/unzip.h
SOURCES += $$QUAZIP/ioapi.c $$QUAZIP/unzip.c $$QUAZIP/zip.c $$QUAZIP/quaadler32.cpp $$QUAZIP/quazip.cpp $$QUAZIP/quacrc32.cpp $$QUAZIP/quazipnewinfo.cpp $$QUAZIP/quazipfile.cpp

symbian: {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = Location NetworkServices
    #TARGET.UID3 = 0x20045CF8
}
