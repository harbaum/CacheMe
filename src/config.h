/* external string MACRO handling is broken under symbian^1 */
#ifdef Q_OS_SYMBIAN
#define APPNAME "cacheme"
#define VERSION "0.9.4"
#define DATADIR "/data/cacheme"
#define LIBDIR "/data/cacheme"
#ifdef EXPIRATION_DATE
#define EXPIRATION_DATE "01102012"
#endif
#endif

class Config {
 public:  
#ifdef Q_OS_SYMBIAN
  // symbian has a lower resolution and needs smaller graphics
  static const int MAP_BUTTON_MAX_SIZE = 52;
  static const int MAP_BUBBLE_RADIUS = 16;
  static const int MAP_BUBBLE_BASE_SIZE = 16;
  static const int MAP_WIDGET_ICON_SIZE = 36;
  static const int MAP_BANNER_FONT_SIZE = 18;
#else
  // desktop/maemo5/meego/...
  static const int MAP_BUTTON_MAX_SIZE = 64;
  static const int MAP_BUBBLE_RADIUS = 20;
  static const int MAP_BUBBLE_BASE_SIZE = 18;
  static const int MAP_WIDGET_ICON_SIZE = 48;
  static const int MAP_BANNER_FONT_SIZE = 24;
#endif
  static const int MAP_BUTTON_MAX_SIZE_HIREZ = 1.5*MAP_BUTTON_MAX_SIZE;
  static const int MAP_WIDGET_ICON_SIZE_HIREZ = 1.5*MAP_WIDGET_ICON_SIZE;
  static const int MAP_BUBBLE_BIG_SIZE = 4*MAP_BUBBLE_BASE_SIZE/3;
  static const int MAP_BANNER_BORDER = MAP_BANNER_FONT_SIZE/4;
};
