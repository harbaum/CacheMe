#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QSettings>
#include <QAction>
#include <QPainter>
#include <QHBoxLayout>
#include <QFontInfo>
#include <QToolButton>
#include <QMenuBar>

#include <QLabel>

#include "customwindow.h"

#ifdef Q_WS_MAEMO_5
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

static void grabZoomKeys(QWidget *window) {
  if(!window->winId()) return;
  
  unsigned long val = 1;
  Atom atom = XInternAtom(QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", False);
  if(!atom) return;
  
  XChangeProperty (QX11Info::display(), window->winId(), atom, 
		   XA_INTEGER, 32, PropModeReplace,
		   reinterpret_cast<unsigned char *>(&val),  1);
}
#endif

#ifdef SYMBIAN_BUTTON_INTERFACE
void CustomWindow::reParent(CustomWindow *newParent) {
  m_parent = newParent;
}
#endif

CustomWindow::CustomWindow(const QString &name, QWidget *parent) : 
  QMainWindow(parent), m_name(name), m_iconLoader(NULL), 
  m_layout(NULL), m_title(NULL), m_parent(NULL)
#ifdef SYMBIAN_BUTTON_INTERFACE
  , iInterfaceSelector(NULL), iCoreTarget(NULL)
#endif
{
  qDebug() << __FUNCTION__;

#if !defined(Q_WS_MAEMO_5) && !defined(Q_OS_SYMBIAN)
  // restore window settings
  QSettings settings;
  settings.beginGroup(name);
  resize(settings.value("size", QSize(640, 480)).toSize());
  move(settings.value("pos", QPoint(0, 0)).toPoint());
  settings.endGroup();
#endif

#ifdef Q_WS_MAEMO_5
  setAttribute(Qt::WA_Maemo5StackedWindow);
  setAttribute(Qt::WA_Maemo5AutoOrientation, true);
  grabZoomKeys(this);
#endif

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
  // we use "custom window decorations" in fullscreen
  if(parent && !parent->isFullScreen()) {
    QAction *backAction = new QAction( tr("Back"), this );
    backAction->setSoftKeyRole( QAction::NegativeSoftKey );
    connect(backAction, SIGNAL(triggered()), this, SLOT(close()));
    addAction(backAction );
  }

  // under belle (qt >= 4.7.4) the "Exit" button is replaced with a back arrow
  // just connect an empty action, anyway ...
  // under symbian1 (qt < 4.7.4) just connect an exit button so it doesn't show
  // up in the menu which is related to https://bugreports.qt-project.org/browse/QTBUG-19523
  if(!parent) {
#if (defined(Q_OS_SYMBIAN) && (QT_VERSION < 0x040704)) || defined(Q_WS_SIMULATOR)
    QAction *hideAction = new QAction(tr("Exit"), this );
    connect(hideAction, SIGNAL(triggered()), this, SLOT(close()));   
#else
    QAction *hideAction = new QAction( this );
#endif    

    hideAction->setSoftKeyRole( QAction::NegativeSoftKey );
    addAction(hideAction );
  }
#endif

  setAttribute(Qt::WA_DeleteOnClose);

  if(parent) {
    // parent widget must be a window
    m_parent = static_cast<CustomWindow *>(parent);
    Q_ASSERT(m_parent);

#ifdef SYMBIAN_BUTTON_INTERFACE
    m_parent->grabSpecialKeys(false);

    // append this new window to parents list of children
    m_parent->m_children.append(this);
#endif
  }

#ifdef SYMBIAN_BUTTON_INTERFACE
  grabSpecialKeys(true);
#endif

  /* symbian has a problem displaying maximized subwindows when the */
  /* parent window is fullscreen. Thus we force the child windows */
  /* to be fullscreen as well which in turn requires a means to return */
  /* frm them. We accomplish this by adding a custom widget on top */
  /* incl. a "back" button */

  /* under meegos tablet ux the windows are entirely unmanaged and */
  /* always need this */

#if defined(Q_OS_SYMBIAN)
  if(parent && parent->isFullScreen()) {
    m_iconLoader = new IconLoader(16);

    QWidget *vbox = new QWidget;
    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);
    vbox->setLayout(m_layout);
    
    QWidget *hbox = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    hbox->setLayout(layout);

    m_title = new QLabel();

    QFontInfo fontInfo(m_title->font());
    layout->addWidget(m_iconLoader->newWidget("cacheme", 
				  fontInfo.pixelSize()));

    layout->addWidget(m_title, 1);    

    QToolButton *button = new QToolButton;
    button->setAutoRaise( true );
    button->setContentsMargins(0,0,0,0);
 
    button->setIcon(QIcon(*m_iconLoader->load("button_back", 
					      fontInfo.pixelSize())));

    connect(button, SIGNAL(clicked()), this, SLOT(close()));

    layout->addWidget(button);    
    m_layout->addWidget(hbox);

    // get rid of "Action" menu entry
    vbox->setContextMenuPolicy(Qt::NoContextMenu);

    QMainWindow::setCentralWidget(vbox);
  }
#endif
}

void CustomWindow::setCentralWidget(QWidget *widget) {
  if(m_layout)
    m_layout->addWidget(widget);
  else {
#if defined(Q_OS_SYMBIAN)
    // get rid of "Action" menu entry
    widget->setContextMenuPolicy(Qt::NoContextMenu);
#endif

    QMainWindow::setCentralWidget(widget);    
  }
}

void CustomWindow::setWindowTitle(const QString &title) {
  if(m_title) m_title->setText(title);
  QMainWindow::setWindowTitle(title);
}

CustomWindow::~CustomWindow() {
#if !defined(Q_WS_MAEMO_5) && !defined(Q_OS_SYMBIAN)
  QSettings settings;
  settings.beginGroup(m_name);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();
#endif

#ifdef SYMBIAN_BUTTON_INTERFACE
  // if this window had children, then one of these
  // sure had focus under Symbian
  if(m_children.size() == 0) {
    grabSpecialKeys(false);
    if(m_parent)
      m_parent->grabSpecialKeys(true);
  }

  // inform all childern that their parent is about to be closed
  foreach(CustomWindow *child, m_children)
    child->reParent(m_parent);  

  if(m_parent) {
    // and transfer children list to parent
    m_parent->m_children.append(m_children);
  
    // remove this window from parents list of children
    m_parent->m_children.removeOne(this);
  }
#endif

  if(m_iconLoader) delete m_iconLoader;
}

#ifdef Q_OS_SYMBIAN
void CustomWindow::show() {

  // symbian has problems displaying maximized windows which are
  // children of fullscreen windows. We thus force all child
  // windows to be fullscreen as well. But then we need to add
  // some magic to be able to close such windows
  if(parentWidget() && parentWidget()->isFullScreen()) {
    qDebug() << __FUNCTION__ << "fullscreen";
    QWidget::showFullScreen();
  } else {
    qDebug() << __FUNCTION__ << "maximized";
    QWidget::showMaximized();
  }



}
#endif

void CustomWindow::keyPressEvent(QKeyEvent* event) {
  qDebug() << __FUNCTION__ << event->key();

  // check if device is in portrait mode
  bool portrait = size().height() > size().width();

  switch (event->key()) {
  case Qt::Key_F7:
    if(portrait) emit zoomOutPressed();
    else         emit zoomInPressed();
    event->accept();
    break;
    
  case Qt::Key_F8:
    if(portrait) emit zoomInPressed();
    else         emit zoomOutPressed();
    event->accept();
    break;
  }
  QWidget::keyPressEvent(event);
}

#ifdef SYMBIAN_BUTTON_INTERFACE
void CustomWindow::grabSpecialKeys(bool grab) {
  qDebug() << __FUNCTION__ << m_name << grab;

  if(grab) {
    // under Symbian only one entity can grab the media keys. We let the map get them
    QT_TRAP_THROWING(iInterfaceSelector = CRemConInterfaceSelector::NewL());
    QT_TRAP_THROWING(iCoreTarget = CRemConCoreApiTarget::NewL(*iInterfaceSelector, *this));
    iInterfaceSelector->OpenTargetL();
  } else {
    if(iInterfaceSelector) {
      delete iInterfaceSelector;
      iInterfaceSelector = NULL;
    }
  }
}

// Callback when media keys are pressed
void CustomWindow::MrccatoCommand(TRemConCoreApiOperationId aOperationId,
				  TRemConCoreApiButtonAction aButtonAct) {

  // check if device is in portrait mode
  bool portrait = size().height() > size().width();

  //TRequestStatus status;
  switch( aOperationId ) {
  case ERemConCoreApiVolumeDown:
    if(portrait) emit zoomOutPressed();
    else         emit zoomInPressed();
    break;
  case ERemConCoreApiVolumeUp:
    if(portrait) emit zoomInPressed();
    else         emit zoomOutPressed();
    break;
  default:
    break;
  }
}
#endif

void CustomWindow::toggleFullscreen() {
  if(!isFullScreen())
    showFullScreen();
  else
#ifdef Q_OS_SYMBIAN
    showMaximized();
#else
    showNormal();
#endif
}

QAction *CustomWindow::addMenuEntry(const QString &title, 
	    QObject *target, const char *slot, QMenu *menu) {
  QAction *action = new QAction(title, this);
  connect(action, SIGNAL(triggered()), target, slot);
  if(menu) menu->addAction(action);
  else     menuBar()->addAction(action);
  return action;
}
