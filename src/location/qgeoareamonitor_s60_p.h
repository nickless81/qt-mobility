/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QGEOAREAMONITOR60_H_
#define QGEOAREAMONITOR60_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qgeoareamonitor.h"
#include "qmlbackendmonitorao_s60_p.h"
#include "notificationmonitorcallback_s60_p.h"
#include "qmlbackendtriggerchangeao_s60_p.h"
#include "qmlbackendmonitorcreatetriggerao_s60_p.h"

#include <lbt.h>
#include <lbttriggerinfo.h>
#include <lbttriggerconditionarea.h>
#include <lbtgeocircle.h>

QTM_BEGIN_NAMESPACE

/**
 *  QGeoAreaMonitorS60
 *
 */
class QGeoAreaMonitorS60 : public INotificationMonitorCallback,
            public QGeoAreaMonitor

{
public :
    static QGeoAreaMonitorS60* NewL(QObject *parent = 0);
    void setCenter(const QGeoCoordinate &coordinate);
    void setRadius(qreal radius);
    void handleTriggerEvent(TPositionInfo aPosInfo, enTriggerType aStatus);
    ~QGeoAreaMonitorS60();

    /*
     * checks whether the object is valid
     */
    TBool isValid() {
        if (iTriggerAO && iNotifyTriggerAO && iTriggerCreateAO)
            return TRUE;
        else
            return FALSE;
    }

private:
    static QGeoAreaMonitorS60* NewLC(QObject* parent);

    void ConstructL();
    QGeoAreaMonitorS60(QObject* aParent = 0);
    void setMonitoredArea(const QGeoCoordinate & aCoordinate, qreal aRadius);
    void TPositionInfoToQGeoPositionInfo(TPositionInfo& aPosInfo, QGeoPositionInfo& aQInfo);
    int QCoordinateToTCoordinate(const QGeoCoordinate& aQCoord, TCoordinate& aTCoord);

private slots:

    void  connectNotify(const char* signal);
    void  disconnectNotify(const char* signal);

private:
    RLbtServer lbtServ;
    QMLBackendMonitorAO *iTriggerAO;
    QMLBackendMonitorCreateTriggerAO* iTriggerCreateAO;
    QMLBackendTriggerChangeAO *iNotifyTriggerAO;
    bool connectedLbt;
    static TInt refCount;
};

QTM_END_NAMESPACE

#endif /* QGEOAREAMONITORS60_H_ */
