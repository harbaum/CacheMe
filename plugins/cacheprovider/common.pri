#Includes common configuration for all subdirectory .pro files.
INCLUDEPATH += . ..
WARNINGS += -Wall

TEMPLATE = lib

include(../../common.pri)

DESTDIR = ..

unix {
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = /usr
  }

  #MAKE INSTALL

  INSTALLS += target

  target.path =$$PREFIX/lib/cacheme/plugins/cacheprovider
}
