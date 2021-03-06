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


#ifndef QMLBACKENDAO_H
#define QMLBACKENDAO_H

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

#include <e32base.h>    // For CActive, link against: euser.lib
#include <lbs.h>
#include <lbscommon.h>
#include <lbssatellite.h>
#include "qgeopositioninfosource.h"
//#include "qgeopositioninfosources60.h"

QTM_BEGIN_NAMESPACE

class CQGeoPositionInfoSourceS60 ;
class CQGeoSatelliteInfoSourceS60;

// Types of request
enum RequestType {
    RegularUpdate = 1,  // Regular updates
    OnceUpdate,     // request update
    DeviceStatus       // device updates
};

//
class CQMLBackendAO : public CActive
{
public:
    // Cancel and destroy
    ~CQMLBackendAO();

    // Two-phased constructor.
    static CQMLBackendAO* NewL(QObject *aRequester,
                               RequestType  aRequestType, TPositionModuleId  aModId = TUid::Uid(0)
                              );

    // Two-phased constructor.
    static CQMLBackendAO* NewLC(QObject *aRequester,
                                RequestType  aRequestType, TPositionModuleId  aModID);

    // checks any pending request in activeobject
    bool isRequestPending();

    // Async call to get notifications about device status.
    void notifyDeviceStatus(TPositionModuleStatusEventBase &aStatusEvent) ;

    // requesting for position update once
    void requestUpdate(int aTimeout);

    // cancels an outstanding update request
    void cancelUpdate();

    // Sets an interval in millisecs for regular notifications
    int setUpdateInterval(int aMilliSec);

    void startUpdates();

private:
    // C++ constructor
    CQMLBackendAO();

    // Second-phase constructor
    TInt ConstructL(QObject *aRequester, RequestType  aRequestType,
                    TPositionModuleId  aModId);

    // Device Notifications are handled
    void handleDeviceNotification(int aError);

    // regular position notifications are handled
    void handlePosUpdateNotification(int aError);

    void CancelAll();

    bool initializePosInfo();
    
    

private:
    // From CActive
    // Handle completion
    void RunL();

    // How to cancel me
    void DoCancel();

    // Override to handle leaves from RunL(). Default implementation causes
    // the active scheduler to panic.
    int RunError(int aError);

private:

    HPositionGenericInfo *mPosInfo;

    // Request is a device or a regular
    CQGeoPositionInfoSourceS60 *mRequester;

    //Request is a device for Satellite update only
    CQGeoSatelliteInfoSourceS60 *mRequesterSatellite;

    // Request type once, regular, device
    RequestType  mRequestType;

    // Positioning Information
    RPositioner mPositioner;


    TPositionSatelliteInfo mPosSatInfo;
};

QTM_END_NAMESPACE

#endif // QMLBACKENDAO_H
