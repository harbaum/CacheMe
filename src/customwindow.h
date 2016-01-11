#ifndef CUSTOMWINDOW_H
#define CUSTOMWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QString>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>

#include "iconloader.h"

#ifdef SYMBIAN_BUTTON_INTERFACE
#include <remconcoreapitargetobserver.h>    // link against RemConCoreApi.lib
#include <remconcoreapitarget.h>            // and
#include <remconinterfaceselector.h>        // RemConInterfaceBase.lib
#endif

class CustomWindow : public QMainWindow
#ifdef SYMBIAN_BUTTON_INTERFACE
, public MRemConCoreApiTargetObserver 
#endif
{
  Q_OBJECT;

 public:
  CustomWindow(const QString &, QWidget * = 0);
  ~CustomWindow();
  void setCentralWidget(QWidget *);
  void setWindowTitle(const QString &);
  QAction *addMenuEntry(const QString &, QObject *, const char *, QMenu * = 0);
  void reParent(CustomWindow *);

#ifdef SYMBIAN_BUTTON_INTERFACE
  virtual void MrccatoCommand(TRemConCoreApiOperationId aOperationId,
			      TRemConCoreApiButtonAction aButtonAct);
  void grabSpecialKeys(bool);
#endif
  
 protected:
  void keyPressEvent(QKeyEvent*);

 signals:
  void zoomInPressed();
  void zoomOutPressed();

 public slots:
  void toggleFullscreen();
#ifdef Q_OS_SYMBIAN
  void show();
#endif

 private:
  QString m_name;

  // stuff only used when doing "custom window decorations"
  IconLoader *m_iconLoader;
  QVBoxLayout *m_layout;
  QLabel *m_title;
  CustomWindow *m_parent;

#ifdef SYMBIAN_BUTTON_INTERFACE
  // since the button interface can only be linked to 
  // one window, we need to keep information on the top window
  QList <CustomWindow*> m_children;

  CRemConInterfaceSelector* iInterfaceSelector;
  CRemConCoreApiTarget*     iCoreTarget;
#endif
};

#endif // CUSTOMWINDOW
