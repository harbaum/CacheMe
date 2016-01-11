APPNAME = cacheme
VERSION = 0.9.4
CONFIG += release

maemo5 {
  QT += maemo5
  CONFIG += mobility12
} else {
  CONFIG += mobility
}

MOBILITY = location
