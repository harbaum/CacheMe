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

#ifndef QGEOSEARCHMANAGERENGINE_CM_H
#define QGEOSEARCHMANAGERENGINE_CM_H

#include "qgeoserviceproviderplugin_cm.h"

#include <qgeoserviceprovider.h>
#include <qgeosearchmanagerengine.h>

#include <QNetworkAccessManager>

QTM_USE_NAMESPACE

class QGeoSearchManagerEngineCm : public QGeoSearchManagerEngine
{
    Q_OBJECT
public:
    QGeoSearchManagerEngineCm(const QMap<QString, QVariant> &parameters,
                                 QGeoServiceProvider::Error *error,
                                 QString *errorString);
    ~QGeoSearchManagerEngineCm();

    QGeoSearchReply* geocode(const QGeoAddress &address,
                             QGeoBoundingArea *bounds);

    QGeoSearchReply* reverseGeocode(const QGeoCoordinate &coordinate,
                                    QGeoBoundingArea *bounds);

    QGeoSearchReply* search(const QString &searchString,
                            QGeoSearchManager::SearchTypes searchTypes,
                            int limit,
                            int offset,
                            QGeoBoundingArea *bounds);


private slots:
    void placesFinished();
    void placesError(QGeoSearchReply::Error error, const QString &errorString);

private:
    QGeoSearchReply* search(QString requestString, QGeoBoundingArea *bounds, int limit=-1, int offset=0);

    QNetworkAccessManager *m_networkManager;
    QString m_host;
    QString m_token;
};

#endif  // QGEOSEARCHMANAGERENGINE_CM_H
