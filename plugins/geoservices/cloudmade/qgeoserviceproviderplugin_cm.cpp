/****************************************************************************
**
** Copyright (C) 2011 Cloudmade 
** All rights reserved.
** Contact: Cloudmade (info@cloudmade.com)
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
** Cloudmade at info@cloudmade.com.
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

#include "qgeoserviceproviderplugin_cm.h"
#include "qgeomappingmanagerengine_cm.h"
#include "qgeosearchmanagerengine_cm.h"
#include "qgeoroutingmanagerengine_cm.h"

#include <QtPlugin>
#include <QNetworkProxy>

QGeoServiceProviderFactoryCm::QGeoServiceProviderFactoryCm() {}

QGeoServiceProviderFactoryCm::~QGeoServiceProviderFactoryCm() {}

QString QGeoServiceProviderFactoryCm::providerName() const
{
    return "cloudmade";
}

int QGeoServiceProviderFactoryCm::providerVersion() const
{
    return 1;
}

QGeoMappingManagerEngine* QGeoServiceProviderFactoryCm::createMappingManagerEngine(const QMap<QString, QVariant> &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString)const
{
    return new QGeoMappingManagerEngineCm(parameters, error, errorString);
}

QGeoSearchManagerEngine* QGeoServiceProviderFactoryCm::createSearchManagerEngine(const QMap<QString, QVariant> &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return new QGeoSearchManagerEngineCm(parameters, error, errorString);
}

QGeoRoutingManagerEngine* QGeoServiceProviderFactoryCm::createRoutingManagerEngine(const QMap<QString, QVariant> &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return new QGeoRoutingManagerEngineCm(parameters, error, errorString);
}

const QString QGeoServiceProviderFactoryCm::defaultToken("8c290f7de20b4ba1a1576612780bad02"); 


Q_EXPORT_PLUGIN2(qtgeoservices_cm, QGeoServiceProviderFactoryCm)
