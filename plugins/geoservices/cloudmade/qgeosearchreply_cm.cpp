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

#include "qgeosearchreply_cm.h"
#include "qgeocodeparser.h"
#include <QDebug>

QGeoSearchReplyCm::QGeoSearchReplyCm(QNetworkReply *reply, int limit, int offset, QGeoBoundingArea *viewport, QObject *parent)
        : QGeoSearchReply(parent),
        m_reply(reply)
{
    m_reply->setParent(this);
    connect(m_reply,
            SIGNAL(finished()),
            this,
            SLOT(networkFinished()));

    connect(m_reply,
            SIGNAL(error(QNetworkReply::NetworkError)),
            this,
            SLOT(networkError(QNetworkReply::NetworkError)));
    connect(m_reply,
            SIGNAL(destroyed()),
            this,
            SLOT(replyDestroyed()));
    setLimit(limit);
    setOffset(offset);
    setViewport(viewport);
}

QGeoSearchReplyCm::~QGeoSearchReplyCm()
{
}

void QGeoSearchReplyCm::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();
    m_reply->deleteLater();
}

void QGeoSearchReplyCm::replyDestroyed()
{
    m_reply = 0;
}

void QGeoSearchReplyCm::networkFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(QGeoSearchReply::CommunicationError, m_reply->errorString());
        m_reply->deleteLater();
        return;
    };

    QGeoCodeParser parser;
    if (parser.parse(m_reply)) {
        setPlaces(parser.results());
        setFinished(true);
    } else {
        // add a qWarning with the actual parser.errorString()
        setError(QGeoSearchReply::ParseError, "The response from the service was not in a recognisable format.");
    }

    m_reply->deleteLater();
}

void QGeoSearchReplyCm::networkError(QNetworkReply::NetworkError error)
{
    if (!m_reply)
        return;

    setError(QGeoSearchReply::CommunicationError, m_reply->errorString());
    m_reply->deleteLater();
}
