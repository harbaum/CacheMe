#include <QDebug>
#include <QQueue>

#include "mapwidget.h"
#include "mapbanner.h"

MapBanner::MapBanner(MapWidget *mapWidget) : 
  m_mapWidget(mapWidget) {
}

MapBanner::~MapBanner() {
};

void MapBanner::message(const QString &msg) {
  MapBannerEntry *entry = new MapBannerEntry(m_mapWidget, msg);

  // connect to entries "done" signal to delete it from list
  QObject::connect( entry, SIGNAL(done(MapBannerEntry*)), 
		    this, SLOT(flush(MapBannerEntry*)) );
  m_entries.append(entry);
}

void MapBanner::flush(MapBannerEntry *entry) {
  // find entry in list
  int removed = m_entries.removeAll(entry);
  Q_ASSERT(removed == 1);

  entry->deleteLater();
}

void MapBanner::paint(QPainter *painter) {
  // for some very odd reason, the list gets deleted when using
  // the "foreach" operator instead
  int j;
  MapBannerEntryList::const_iterator i;
  for(j=0, i = m_entries.begin(); i != m_entries.end(); ++i, j++ ) {
    // a banner will start to appear if it's in the waiting state
    // and we give it the start flag
    // the start flag is given, if a banner is first in the list or
    // if it's second and the first banner is already disappearing
    bool start = (j==0);
    if(j==1)
      start = (*(i-1))->isDisappearing();

    (*i)->paint(painter, start);
  }
}
