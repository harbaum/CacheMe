######################################################################
# 
# cacheme.pro - project file for cacheme
#
######################################################################

#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS += -pg

ENABLE_BUILTINBROWSER = TRUE

TEMPLATE = app
include(../common.pri)
MOBILITY += sensors systeminfo

TARGET = $$APPNAME
DEPENDPATH += .
QT += svg xml network

equals(ENABLE_BUILTINBROWSER, TRUE) { # built-in browser
  message("Including webkit support")
  QT += webkit
  DEFINES += BUILTINBROWSER
}

OTHER_FILES += ../lang/*.qm

# generic helpers
HEADERS += filefinder.h iconloader.h custompixmap.h infowidget.h distancewidget.h directionwidget.h browserwindow.h
SOURCES += filefinder.cpp iconloader.cpp custompixmap.cpp infowidget.cpp distancewidget.cpp directionwidget.cpp browserwindow.cpp

# Input
HEADERS += customwindow.h iconlabel.h mainwindow.h cache.h cachelist.h cacheprovider.h cacheproviderplugin.h coordinatetool.h pickerwidget.h pickermanager.h waypointdb.h
SOURCES += customwindow.cpp iconlabel.cpp mainwindow.cpp main.cpp cache.cpp cachelist.cpp cacheprovider.cpp coordinatetool.cpp pickerwidget.cpp pickermanager.cpp waypointdb.cpp

# cache detail view
HEADERS += swapbox.h cachewindow.h htmlview.h descriptionwindow.h logwindow.h logwidget.h attributewindow.h hintwindow.h logeditwindow.h
SOURCES += swapbox.cpp cachewindow.cpp htmlview.cpp descriptionwindow.cpp logwindow.cpp logwidget.cpp attributewindow.cpp hintwindow.cpp logeditwindow.cpp

# cache list
HEADERS += cachelistwindow.h cachewidget.h searchdialog.h checklistdialog.h
SOURCES += cachelistwindow.cpp cachewidget.cpp searchdialog.cpp checklistdialog.cpp

# the map
HEADERS += config.h mapwidget.h mapoverlay.h mapbutton.h mapbuttonmanager.h mapbubble.h settingsdialog.h mapbanner.h mapbannerentry.h mapspinner.h
SOURCES += mapwidget.cpp mapoverlay.cpp mapbutton.cpp mapbuttonmanager.cpp mapbubble.cpp settingsdialog.cpp mapbanner.cpp mapbannerentry.cpp mapspinner.cpp

# gps and navigation
HEADERS += locationprovider.h navigationwindow.h compass.h
SOURCES += locationprovider.cpp navigationwindow.cpp compass.cpp

# tools
HEADERS += tool.h tool_rot13.h tool_projection.h tool_formula.h tool_roman.h EXParser.h
SOURCES += tool.cpp tool_rot13.cpp tool_projection.cpp tool_formula.cpp tool_roman.cpp EXParser.cpp 

# this primarily intended for desktop linux, but other OSs may also
# have a use for this.
HEADERS += gpsdpositioninfosource.h json.h
SOURCES += gpsdpositioninfosource.cpp json.cpp
DEFINES += GPSDPOSITIONINFOSOURCE

symbian: {
  # TARGET is also the icon label
  TARGET = CacheMe
  DEFINES += APPNAME=\"$$APPNAME\" VERSION=\"$$VERSION\"
  RESOURCES = ../cacheme.qrc

  # set this in config.h!!
  DEFINES += EXPIRATION_DATE

  # symbian uses flick charm
  HEADERS += flickcharm.h
  SOURCES += flickcharm.cpp
  DEFINES += FLICKCHARM

  DEFINES += SYMBIAN_BUTTON_INTERFACE
  INCLUDEPATH += MW_LAYER_SYSTEMINCLUDE
  LIBS += -L/epoc32/release/armv5/lib -lremconcoreapi
  LIBS += -L/epoc32/release/armv5/lib -lremconinterfacebase

  TARGET.CAPABILITY = Location NetworkServices
  TARGET.EPOCHEAPSIZE = 0x20000 0x1000000 // Min 128kb, Max 16Mb
  #TARGET.UID3 = 0x20045CF8

  # datadir on symbian only holds the plugins as other 
  # files are included into the exe
  DATADIR = /data/$$APPNAME

  translations.sources = ../lang/cacheme_de.qm ../lang/cacheme_sk.qm ../lang/cacheme_fi.qm ../lang/cacheme_cs.qm ../lang/cacheme_sv.qm
  translations.path = $$DATADIR/lang
  DEPLOYMENT += translations

  cacheprovider.sources = ../plugins/cacheprovider/gclive.dll  ../plugins/cacheprovider/oc.dll ../plugins/cacheprovider/gpx.dll ../plugins/cacheprovider/none.dll
  cacheprovider.path = $$DATADIR/plugins/cacheprovider
  DEPLOYMENT += cacheprovider

  geoservices.sources = ../plugins/geoservices/qtgeoservices_osm.dll ../plugins/geoservices/qtgeoservices_cm.dll
  geoservices.path = $$DATADIR/plugins/geoservices
  DEPLOYMENT += geoservices

  DEFINES += DATADIR=\"$$DATADIR\" LIBDIR=\"$$DATADIR\"

  ICON = ../data/symbian.svg

  vendorinfo = \
    "%{\"Till Harbaum\"}" \
    ":\"Till Harbaum\""

  my_deployment.pkg_prerules = vendorinfo
  DEPLOYMENT += my_deployment
} else {

  DESTDIR = ..
  DEFINES += APPNAME=\\\"$$APPNAME\\\" VERSION=\\\"$$VERSION\\\"

  unix {
    #VARIABLES
    isEmpty(PREFIX) {
      PREFIX = /usr
    }

    LIBS += -rdynamic
    BINDIR = $$PREFIX/bin
    LIBDIR =$$PREFIX/lib
    DATADIR =$$PREFIX/share

    DEFINES += DATADIR=\\\"$$DATADIR/$$APPNAME\\\" LIBDIR=\\\"$$LIBDIR/$$APPNAME\\\"

    #MAKE INSTALL

    INSTALLS += target desktop iconsvg icon64 icon48 icon26 icons translations

    target.path =$$BINDIR

    desktop.path = $$DATADIR/applications
    desktop.files = ../data/$${APPNAME}.desktop

    iconsvg.path = $$DATADIR/icons/hicolor/scalable/apps
    iconsvg.files = ../data/scalable/$${APPNAME}.svg

    icon64.path = $$DATADIR/icons/hicolor/64x64/apps
    icon64.files = ../data/64x64/$${APPNAME}.png

    icon48.path = $$DATADIR/icons/hicolor/48x48/apps
    icon48.files = ../data/48x48/$${APPNAME}.png

    icon26.path = $$DATADIR/icons/hicolor/26x26/apps
    icon26.files = ../data/26x26/$${APPNAME}.png

    icons.path = $$DATADIR/$${APPNAME}/icons
    icons.files = ../data/icons/*.svg

    translations.path = $$DATADIR/$${APPNAME}/lang
    translations.files = ../lang/*.qm

    maemo5 {
      # maemo5 expects everything in hildon and a 64x64 png in scalable ...
      desktop.path = $$DATADIR/applications/hildon
      iconsvg.path = $$DATADIR/icons/hicolor/scalable/hildon
      iconsvg.files = ../data/64x64/$${APPNAME}.png
      icon64.path = $$DATADIR/icons/hicolor/64x64/hildon
      icon48.path = $$DATADIR/icons/hicolor/48x48/hildon
      icon26.path = $$DATADIR/icons/hicolor/26x26/hildon
    } else {
      # flick charm testing on the desktop and meego also has a use for it
      # symbian uses flick charm
      HEADERS += flickcharm.h
      SOURCES += flickcharm.cpp
      DEFINES += FLICKCHARM
    }
  }
}

simulator {
  RESOURCES = ../cacheme.qrc
}

