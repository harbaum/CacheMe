//
// gpxfileparser.cpp
//
// parser for groundspeak gpx xml files
//

#include "gpxfileparser.h"

#include "quazip.h"
#include "quazipfile.h"

#include <QFileInfo>

#include <QDebug>

GpxFileParser::GpxFileParser() {
  qDebug() << __FUNCTION__;
}

QString GpxFileParser::parseGpxFile(QIODevice &file) {
  QXmlStreamReader xml(&file);
  parseRoot(xml, m_cacheList);
  
  /* Error handling. */
  if(xml.hasError()) 
    return QObject::tr("Parse error at line %1, column %2:\n%3")
      .arg(xml.lineNumber()).arg(xml.columnNumber()).arg(xml.errorString());

  xml.clear();
  
  qDebug() << __FUNCTION__ << "Parser finished";

  return NULL;
}

QString GpxFileParser::parseGpxWptFile(QIODevice &file) {
  QXmlStreamReader xml(&file);
  parseWptRoot(xml, m_cacheList);
  
  /* Error handling. */
  if(xml.hasError()) 
    return QObject::tr("Parse error at line %1, column %2:\n%3")
      .arg(xml.lineNumber()).arg(xml.columnNumber()).arg(xml.errorString());
  
  xml.clear();
  
  qDebug() << __FUNCTION__ << "Parser for wpt finished";

  return NULL;
}

static bool filenameIsWaypoint(const QString &name) {
  // a waypoint file name ends with -wpts.gpx
  return name.endsWith( "-wpts.gpx", Qt::CaseInsensitive);
}

void GpxFileParser::parse(const QString &name) {
  qDebug() << __FUNCTION__ << name;

  m_cacheList.clear();

  // check for file type
  QFileInfo info(name);
  if(!info.suffix().compare("zip", Qt::CaseInsensitive)) {
    qDebug() << __FUNCTION__ << "ZIP file detected";

    QuaZip zip(name);
    if(!zip.open(QuaZip::mdUnzip)) {
      emit failed(QObject::tr("Cannot open file %1: Code %2")
		  .arg(zip.getZipName()).arg(zip.getZipError()));

      return;
    }
    
    qDebug() << __FUNCTION__ << "ZIP file opened";

    for(bool m=zip.goToFirstFile(); m; m=zip.goToNextFile()) {
      QString gpxName = zip.getCurrentFileName();

      // check if filename is not a waypoint file name
      if(!filenameIsWaypoint(gpxName)) {
	qDebug() << __FUNCTION__ << "found" << gpxName;

	QuaZipFile file(&zip);
	if(!file.open(QIODevice::ReadOnly)) {
	  QString errorStr = QObject::tr("Cannot open file %1 for reading:\n%2")
	    .arg(gpxName).arg(file.errorString());
	  
	  zip.close();	  
	  emit failed(errorStr);
	  return;
	}

	// parse file
	QString errorStr = parseGpxFile(file);
	file.close();	

	// try to also load waypoints from zip file
	if(!errorStr.isEmpty()) {
	  emit failed(errorStr);
	  return;
	}

	QString wptName = gpxName.insert(gpxName.length()-4,"-wpts");
	
	if(zip.setCurrentFile(wptName, QuaZip::csInsensitive)) {
	  
	  QuaZipFile file(&zip);
	  if(file.open(QIODevice::ReadOnly)) {
	    qDebug() << __FUNCTION__ << "found matching" << wptName;
	    
	    parseGpxWptFile(file);
	    
	    file.close();
	  }
	}
	
	// don't try to process another file from the zip
	break;
      }
    }

    if(zip.getZipError() != UNZ_OK) {
      QString errorStr = QObject::tr("Zip error in %1: Code %2")
	.arg(zip.getCurrentFileName()).arg(zip.getZipError());
      zip.close();
      emit failed(errorStr);
      return;
    }
    
    zip.close();
    
    qDebug() << __FUNCTION__ << "ZIP done";
  } else if(!info.suffix().compare("gpx", Qt::CaseInsensitive) ||
	    !info.suffix().compare("loc", Qt::CaseInsensitive)) {

    // check if user requests to load a waypoint file 
    if(filenameIsWaypoint(name)) {
      emit failed(QObject::tr("File is a waypoint file"));
      return;
    }

    QFile file( name );
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      emit failed(QObject::tr("Cannot open file %1 for reading:\n%2")
		  .arg(file.fileName()).arg(file.errorString()));
      return;
    }
    
    QString errorStr = parseGpxFile(file);
    file.close();

    if(!errorStr.isEmpty()) {
      emit failed(errorStr);
      return;
    }

    QString wptName(name);
    wptName.insert(wptName.length()-4,"-wpts");
    qDebug() << __FUNCTION__ << "trying to open " << wptName;
    QFile wptFile( wptName );
    if (wptFile.open(QFile::ReadOnly | QFile::Text)) {
      qDebug() << __FUNCTION__ << "found matching" << wptName;
	
      parseGpxWptFile(wptFile);
      
      file.close();
    }

  } else {
    emit failed(QObject::tr("Unsupported file type: %1").arg(info.suffix()));
    return;
  }

  if(!m_name.isEmpty())
    verifyName(m_cacheList, m_name);

  emit succeeded(m_cacheList);
}

void GpxFileParser::setName(const QString &name) {
  m_name = name;
}
