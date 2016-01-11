/****************************************************************************
**
** Copyright (C) 2011 Cloudmade 
** All rights reserved.
** Contact: Cloudmade (qtmobility@cloudmade.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you have questions regarding the use of this file, please contact
** Cloudmade at qtmobility@cloudmade.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DEBUG_CM_H
#define DEBUG_CM_H

#include <QString>
#include <QDebug>


// Cloudmade plugin dbg modules
#define SEARCH_M	"Search"
#define TILES_M		"Tiles"
#define ROUTE_M		"Route"
#define REV_GEOCODE_M	"ReverseGeocoding"

// Debug levels
#define INFO_L		0
#define NOTICE_L	1
#define WARN_L		2
#define ERR_L		3
#define CRIT_L		4
#define FATAL_L		5


#if 1
#define DBG_CM(module, lvl, msg)
#else
static int debug_lvl = 0;

#define DBG_CM(module, lvl, msg) 							\
	do {													\
			if (lvl >= debug_lvl) {							\
				qDebug() << "CM:" << module << ":" << msg;	\
			}												\
	} while ( 0 )
#endif

#endif // DEBUG_CM_H
