TEMPLATE = lib
CONFIG += plugin
TARGET = $$qtLibraryTarget(qtgeoservices_osm)
PLUGIN_TYPE=geoservices

include(../common.pri)

QT += network

CONFIG += mobility
MOBILITY = location

HEADERS += \
            qgeomappingmanagerengine_osm.h \
            qgeomapreply_osm.h \
            qgeoserviceproviderplugin_osm.h \
            qgeocodeparser.h \
            qgeosearchreply_osm.h \
            qgeosearchmanagerengine_osm.h


SOURCES += \
            qgeomappingmanagerengine_osm.cpp \
            qgeomapreply_osm.cpp \
            qgeoserviceproviderplugin_osm.cpp \
            qgeocodeparser.cpp \
            qgeosearchreply_osm.cpp \
            qgeosearchmanagerengine_osm.cpp

symbian {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = Location NetworkServices
    #TARGET.UID3 = 0x20045CF8
}
