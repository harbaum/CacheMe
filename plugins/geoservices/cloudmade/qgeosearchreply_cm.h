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

#ifndef QGEOSEARCHREPLY_CM_H
#define QGEOSEARCHREPLY_CM_H

#include <qgeosearchreply.h>
#include <QNetworkReply>

QTM_USE_NAMESPACE

class QGeoSearchReplyCm : public QGeoSearchReply
{
    Q_OBJECT
public:
    QGeoSearchReplyCm(QNetworkReply *reply, int limit, int offset, QGeoBoundingArea *viewport, QObject *parent = 0);
    ~QGeoSearchReplyCm();

    void abort();

private slots:
    void networkFinished();
    void networkError(QNetworkReply::NetworkError error);
    void replyDestroyed();

private:
    QNetworkReply *m_reply;
};

#endif  // QGEOSEARCHREPLY_CM_H
