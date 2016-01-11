#ifndef CACHE_H
#define CACHE_H

#if !defined(Q_OS_SYMBIAN) && !defined(Q_WS_MAEMO_5)
#if QT_VERSION < 0x040800  // doesn't seem to work with qt 4.8
#define STAGING   // define this to test via staging
#endif
#endif

#include <QLandmark>
#include <QDate>

class QXmlStreamWriter;
class QXmlStreamReader;
class QXmlStreamAttributes;

QTM_USE_NAMESPACE

class Description {
 public:
  Description();
  friend QDebug operator<<(QDebug, const Description &);
  void set(bool, const QString &);
  QString text() const;
  bool isHtml() const;
  bool isSet() const;
  void save(QXmlStreamWriter *, const QString &);
  void load(QXmlStreamReader &);

 private:
  QString m_text;
  bool m_isHtml;
};

class Image {
 public:
  Image();
  friend QDebug operator<<(QDebug, const Image &);
  void setName(const QString &);
  void setDescription(const QString &);
  void setUrl(const QString &);
  QString name() const;
  QString description() const;
  QString url() const;
  void save(QXmlStreamWriter *);
  bool load(QXmlStreamReader &);

 private:
  QString m_name, m_description;
  QString m_url;
};

class Waypoint : public QLandmark {
 public:
  enum Type {
    Unknown, 
    Multistage, Parking, Final, Question, Trailhead, Refpoint,
    Custom
  };
  
  Waypoint();
  friend QDebug operator<<(QDebug, const Waypoint &);
  void setComment(const QString &);
  QString comment() const;
  Type type() const;
  void setType(const Type &);
  QString iconFile() const;
  void save(QXmlStreamWriter *);
  bool load(QXmlStreamReader &);

 private:  
  Type m_type;
  QString m_comment;
};

class Attribute {
 public:
  enum Id {
    Unknown = 0, 
    Dogs = 1, Fee = 2, Rappelling = 3, Boat = 4, Scuba = 5, Kids = 6, 
    OneHour = 7, Scenic = 8, Hiking = 9, Climbing = 10, Wading = 11, 
    Swimming = 12, Available = 13, Night = 14, Winter = 15, Cactus = 16, 
    PoisonOak = 17, Snakes = 18, Ticks = 19, Mine = 20, Cliff = 21, 
    Hunting = 22, Danger = 23, WheelChair = 24, Parking = 25, Public = 26, 
    Water = 27, Restrooms = 28, Phone = 29, Picnic = 30, Camping = 31, 
    Bicycles = 32, Motorcycles = 33, Quads = 34, Jeeps = 35, Snowmobiles = 36, 
    Horses = 37, Campfires = 38, Thorn = 39, Stealth = 40, Stroller = 41, 
    FirstAid = 42, Cow = 43, Flashlight = 44, LandF = 45, RV = 46, 
    FieldPuzzle = 47, UV = 48, Snowshoes = 49, Skiis = 50, STool = 51, 
    NightCache = 52, ParkNGrab = 53, AbandonedBuilding = 54, HikeShort = 55, 
    HikeMed = 56, HikeLong = 57, Fuel = 58, Food = 59, WirelessBeacon = 60,
    PartnerShip = 61, Seasonal = 62, TouristOk = 63, TreeClimbing = 64,
    FrontYard = 65, TeamWork = 66
  };
  
  Attribute(const Id &, bool);
  friend QDebug operator<<(QDebug, const Attribute &);
  QString toString() const;
  QStringList iconFile() const;
  bool isOn() const;
  Id id() const;

 private:
  Id m_id;
  bool m_yes;
};

class Log {
 public:
  class Type {
  public:
    enum Id {
      Unknown, 
      Found, NotFound, Maintenance, WriteNote, ReviewerNote,
      EnableListing, PublishListing, WillAttend, Attended,
      Photo, TempDisable, NeedsMaintenance, UpdatedCoordinates,
      Unarchive, NeedsArchived, Archive
    };

    Type();
    void set(Type::Id);
    bool is(Type::Id) const;
    Type::Id id() const;
    static QString iconFile(Type::Id);
    QString iconFile() const;
    static QString toString(Type::Id);
    QString toString() const;

  private:
    Id m_id;
  };

  Log();
  void setType(const Type &);
  void setType(Type::Id);
  void setDate(const QDate &);
  Type type() const;
  QDate date() const;
  void setFinder(const QString &);
  QString finder() const;
  void setDescription(const Description &);
  Description description() const;
  void save(QXmlStreamWriter *);
  bool load(QXmlStreamReader &);
  friend QDebug operator<<(QDebug, const Log &);

 private:
  Type m_type;
  QDate m_date;
  QString m_finder;
  Description m_description;
};

class Tribool {
 public:
  Tribool();
  friend QDebug operator<<(QDebug, const Tribool &);
  void set(bool);
  bool value() const;
  bool isSet() const;
  bool isSetAndTrue() const;
  void saveAttribute(QXmlStreamWriter *, const QString &);
  void loadAttribute(QXmlStreamAttributes &, const QString &);

 private:
  bool m_value;
  bool m_set;
};

class Rating {
 public:
  Rating();
  friend QDebug operator<<(QDebug, const Rating &);
  void set(qreal);
  qreal value() const;
  QString toString() const;
  bool isSet() const;
  void save(QXmlStreamWriter *, const QString &);
  void load(QXmlStreamReader &);
  
 private:
  int m_value;
};

class Container {
 public:
  enum Type {
    ContainerUnknown, ContainerRegular, ContainerSmall, ContainerMicro,
    ContainerOther,   ContainerNotChosen, ContainerLarge, ContainerVirtual
  };
  
  Container();
  friend QDebug operator<<(QDebug, const Container &);
  void set(Type);
  Type get() const;
  static QString toString(Type);
  QString toString() const;
  bool isSet() const;
  static QString iconFile(Type, bool = false);
  QString iconFile(bool = false) const;
  void save(QXmlStreamWriter *, const QString &);
  void load(QXmlStreamReader &);
 
 private:
  Type m_value;
};

class Cache : public QLandmark {
 public:

  enum Type {
    TypeUnknown, TypeTraditional, TypeMulti, TypeMystery, TypeVirtual,
    TypeWebcam, TypeEvent, TypeLetterbox, TypeEarthcache, TypeWherigo,
    TypeMegaEvent, TypeCito };
  
  Cache();
  ~Cache();
  friend QDebug operator<<(QDebug, const Cache &);

  // functions dealing with generic flags
  void setOwned(bool);
  Tribool owned() const;

  void setFound(bool);
  Tribool found() const;

  void setAvailable(bool);
  Tribool available() const;

  // function dealing with owner
  void setOwner(const QString &);
  QString owner() const;

  // functions dealing with the cache type
  void setType(Type);
  Type type() const;
  QString typeString() const;
  static QString typeString(Type);
  QString typeIconFile() const;
  static QString typeIconFile(Type);
  QString overlayIconFile() const;

  // functions dealing with the geocaching.com id
  void setId(int);
  int id() const;

  // functions dealing with the caches uuid
  void setGuid(const QString &);
  QString guid() const;

  // functions dealing with difficulty and terrain ratings
  void setDifficulty(qreal);
  Rating difficulty() const;

  void setTerrain(qreal);
  Rating terrain() const;

  // functions dealing with the container
  void setContainer(Container::Type);
  Container container() const;

  // functions dealing with date of placement
  void setDateOfPlacement(const QDate &);
  QDate dateOfPlacement() const;
  QString dateOfPlacementString() const;

  // functions dealing with descriptions
  void setShortDescription(const Description &);
  Description shortDescription() const;

  void setLongDescription(const Description &);
  Description longDescription() const;

  // functions dealing with the hint
  void setHint(const Description &);
  Description hint() const;
 
  // functions dealing with logs
  QList<Log> logs() const;
  void appendLog(const Log &);
  void clearLogs();

  // functions dealing with attributes
  QList<Attribute> attributes() const;
  void appendAttribute(const Attribute &);
  void clearAttributes();

  // functions dealing with waypoints
  QList<Waypoint> waypoints() const;
  void appendWaypoint(const Waypoint &);
  void updateWaypoint(const Waypoint &);
  void clearWaypoints();

  // functions dealing with images
  QList<Image> images() const;
  void appendImage(const Image &);
  void clearImages();

  // functions for writing and restoring
  bool save(QXmlStreamWriter *);
  bool load(QXmlStreamReader &);
  bool loadWaypoints(QXmlStreamReader &);
  bool loadLogs(QXmlStreamReader &);
  bool loadImages(QXmlStreamReader &);

 private:
  Tribool m_owned, m_found, m_available;
  Type m_type;
  Rating m_terrain, m_difficulty;
  int m_id;
  Container m_container;
  QString m_owner;
  QDate m_dateOfPlacement;
  QString m_guid;
  Description m_shortDescription, m_longDescription;
  Description m_hint;
  QList<Log> m_logs;
  QList<Attribute> m_attributes;
  QList<Waypoint> m_waypoints;
  QList<Image> m_images;
};

#endif // CACHE_H
