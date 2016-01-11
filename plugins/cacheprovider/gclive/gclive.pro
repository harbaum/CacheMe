TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../../src
TARGET = $$qtLibraryTarget(gclive)
PLUGIN_TYPE=cacheprovider

include(../common.pri)

QT += network

# this is kind of ugly, but we really need the cache class here as well
HEADERS += json.h gclive.h gclive_logo.h gclivedialog.h ../../../src/cache.h ../../../src/cachelist.h ../../../src/cacheproviderplugin.h
SOURCES += json.cpp gclive.cpp gclivedialog.cpp ../../../src/cache.cpp ../../../src/cachelist.cpp

# kqoauth public headers
HEADERS += kqoauthmanager.h kqoauthrequest.h kqoauthrequest_1.h \
           kqoauthrequest_xauth.h kqoauthglobals.h
# kqoauth private headers
HEADERS += kqoauthrequest_p.h kqoauthmanager_p.h kqoauthauthreplyserver.h \
           kqoauthauthreplyserver_p.h kqoauthutils.h kqoauthrequest_xauth_p.h

SOURCES += kqoauthmanager.cpp kqoauthrequest.cpp kqoauthutils.cpp \
           kqoauthauthreplyserver.cpp kqoauthrequest_1.cpp kqoauthrequest_xauth.cpp

symbian: {
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = Location NetworkServices
    #TARGET.UID3 = 0x20045CF8
}
