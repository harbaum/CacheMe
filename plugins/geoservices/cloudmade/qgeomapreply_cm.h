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

#ifndef QGEOMAPREPLY_CM_H
#define QGEOMAPREPLY_CM_H

#include <qgeotiledmapreply.h>
#include <QNetworkReply>
#include <QFile>
#include "qtimer.h"


QTM_USE_NAMESPACE

class QGeoMappingManagerEngineCm;

class QGeoMapReplyCm : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    QGeoMapReplyCm(QNetworkReply *reply, const QGeoTiledMapRequest &request, QObject *parent = 0);
    ~QGeoMapReplyCm();

    void abort();

    QNetworkReply* networkReply() const;


    // caching
    QString getTileKey(const QGeoTiledMapRequest &request) const;
    QString getTileFileName(const QString &tileKey) const;

    // If tile exist in cache - return pointer to it's file
    // otherwise return NULL
    QFile* isTileInCache(const QString &tileKey, QDateTime &lastModified);

    QString toHttpDate(const QDateTime &dt) const;
    //QDateTime fromHttpDate(const QString &value);

private slots:
    void replyDestroyed();
    void networkFinished();
    void networkError(QNetworkReply::NetworkError error);
    void timeout();
    void resendRequest();

private:
    bool cleanedUp;
    QNetworkReply *m_netReply;
    QNetworkRequest *m_netRequest;
    const QGeoTiledMapRequest &m_tileRequest;
    QTimer *m_timeoutTimer;
    int m_resendCounter;
    QString m_tileKey;
    QString m_tileFileName;
    QString m_tileHttpLastModifiedStr;
    QString m_rawRequest;
    QNetworkAccessManager *m_namPtr;
    QGeoMappingManagerEngineCm *m_mapManagerEngineCm;
};

#endif
