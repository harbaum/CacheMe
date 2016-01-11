TEMPLATE = lib
CONFIG += plugin
TARGET = $$qtLibraryTarget(qtgeoservices_cm)
PLUGIN_TYPE=geoservices

include(../common.pri)

QT += network

CONFIG += mobility
MOBILITY = location

HEADERS += \
            qgeomappingmanagerengine_cm.h \
            qgeomapreply_cm.h \
            qgeoserviceproviderplugin_cm.h \
            qgeocodeparser.h \
            qgeosearchreply_cm.h \
            qgeosearchmanagerengine_cm.h \
            qgeoroutingmanagerengine_cm.h \
            qgeoroutereply_cm.h \
            qgeoroutejsonparser.h \
            json.h debug_cm.h


SOURCES += \
            qgeomappingmanagerengine_cm.cpp \
            qgeomapreply_cm.cpp \
            qgeoserviceproviderplugin_cm.cpp \
            qgeocodeparser.cpp \
            qgeosearchreply_cm.cpp \
            qgeosearchmanagerengine_cm.cpp \
            qgeoroutingmanagerengine_cm.cpp \
            qgeoroutereply_cm.cpp \
            qgeoroutejsonparser.cpp \
            json.cpp

symbian {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = Location NetworkServices
    #TARGET.UID3 = 0x20045CF8
}
