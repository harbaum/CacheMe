#include <QDebug>
#include <QXmlStreamWriter>

#include "cachelist.h"

CacheList::CacheList(const QString &name) {
  this->m_name = (QString)name;
}

CacheList::CacheList() {
}

CacheList::~CacheList() {
}

void CacheList::append(Cache cache) {
  QList<Cache>::append(cache);
}

int CacheList::size() const {
  return QList<Cache>::size();
}

void CacheList::removeFirst() {
  QList<Cache>::removeFirst();
}

Cache CacheList::at(int index) {
  return QList<Cache>::at(index);
}

Cache CacheList::last() {
  return QList<Cache>::last();
}

QString CacheList::name() {
  return m_name;
}

void CacheList::setName(const QString &name) {
  m_name = name;
}

QList<Cache>::const_iterator CacheList::begin() const {
  return QList<Cache>::begin();
}

QList<Cache>::const_iterator CacheList::end() const {
  return QList<Cache>::end();
}

QList<Cache>::iterator CacheList::begin() {
  return QList<Cache>::begin();
}

QList<Cache>::iterator CacheList::end() {
  return QList<Cache>::end();
}

void CacheList::clear() {
  qDebug() << __FUNCTION__;

  QList<Cache>::clear();
}

void CacheList::setDate(const QDate &date) {
  m_date = date;
}

QDate CacheList::date() const {
  return m_date;
}

// quicksort on distances
void CacheList::sortByDistance(qreal *dists, int left, int right) {
  if (left < right) {
    qreal pivot = dists[right];
    int l = left;
    int r = right;

    do {
      while( dists[l] < pivot) l++;
      while( dists[r] > pivot) r--;

      if (l <= r) {
	swap(l, r);
	qreal tmp = dists[l];
	dists[l] = dists[r];
	dists[r] = tmp;

	l++;
	r--;
      }
    } while (l <= r);

    sortByDistance(dists, left, r);
    sortByDistance(dists, l, right);
  }
}

void CacheList::sortByDistance(const QGeoCoordinate &coo) {
  // create distance table
  qreal *dists = new qreal [ size() ];
  QList<Cache>::const_iterator i;
  int j = 0;

  for(i = begin(); i != end(); ++i )
    dists[j++] = (*i).coordinate().distanceTo(coo);

  sortByDistance(dists, 0, size()-1);

  delete dists;
}

// quicksort on distances
void CacheList::sortByDescription(int left, int right) {
  if (left < right) {
    QString pivot = at(right).description();
    int l = left;
    int r = right;

    do {
      while( at(l).description().compare(pivot, Qt::CaseInsensitive) < 0) l++;
      while( at(r).description().compare(pivot, Qt::CaseInsensitive) > 0) r--;

      if (l <= r) {
	swap(l, r);
	l++;
	r--;
      }
    } while (l <= r);

    sortByDescription(left, r);
    sortByDescription(l, right);
  }
}

void CacheList::sortByDescription() {
  sortByDescription(0, size()-1);
}

void CacheList::clip(const QGeoBoundingBox &bbox) {
  QList<Cache>::iterator i = begin();
  while (i != end()) {
    if(!bbox.contains((*i).coordinate()))
      i = erase(i);
    else
      ++i;
  }
}

void CacheList::removeByName(const QString &name) {
  QList<Cache>::iterator i = begin();
  while (i != end()) {
    if((*i).name().compare(name) == 0)
      i = erase(i);
    else
      ++i;
  }
}

// write an entire cachelist into a file
bool CacheList::save(QXmlStreamWriter *xmlWriter, const QString &name) {
  xmlWriter->writeStartElement(name);
  xmlWriter->writeAttribute("name", m_name);

  QList<Cache>::iterator i;
  for(i=begin();i!=end();++i) 
    (*i).save(xmlWriter);

  xmlWriter->writeEndElement();

  return true;
}

static void drop(QXmlStreamReader &xml) {
  xml.readNext();

  while(xml.tokenType() != QXmlStreamReader::EndElement &&
	!xml.atEnd() && !xml.hasError()) {

    if(xml.tokenType() == QXmlStreamReader::StartElement) 
      drop(xml);

    xml.readNext();
  }
}

bool CacheList::load(QXmlStreamReader &xmlReader) {
  bool retval = true;
  xmlReader.readNext();

  clear();

  // parse contents
  while(xmlReader.tokenType() != QXmlStreamReader::EndElement &&
	!xmlReader.atEnd() && !xmlReader.hasError()) {

    if(xmlReader.tokenType() == QXmlStreamReader::StartElement) {
      if(xmlReader.name() == "cache") {
	Cache cache;
	retval &= cache.load(xmlReader);
	append(cache);
      } else {
	qDebug() << __FUNCTION__ << "Unexpected element: " << xmlReader.name();
	drop(xmlReader);
      }
    }
    
    xmlReader.readNext();
  }
  return retval;
}
