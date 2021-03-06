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

#include "telephonyinfo_s60.h"
#ifdef ETELMM_SUPPORTED
#include <etelmm.h>
#include <mmtsy_names.h>
#endif


CTelephonyInfo::CTelephonyInfo(CTelephony &telephony) : CActive(EPriorityStandard),
    m_telephony(telephony)
{
    CActiveScheduler::Add(this);
}

CTelephonyInfo::~CTelephonyInfo()
{
  delete iEventLoop;
}

void CTelephonyInfo::addObserver(MTelephonyInfoObserver *observer)
{
    m_observers.append(observer);
}

void CTelephonyInfo::removeObserver(MTelephonyInfoObserver *observer)
{
    m_observers.removeOne(observer);
}

void CTelephonyInfo::RunL()
{
   TRACES (qDebug() << "CTelephonyInfo::RunL<---");

    if ( iEventLoop ){
        TRACES (qDebug() << "eventloop::exiting");
        if (iEventLoop->isRunning()) iEventLoop->exit();
        }
}

void CTelephonyInfo::makeRequest()
{
    if (!IsActive())
        SetActive();

    if ( !iEventLoop ){
        iEventLoop = new QEventLoop();
        TRACES ( qDebug() << "started event loop...");
        }
    iEventLoop->exec(QEventLoop::ExcludeUserInputEvents); //start the loop
}

CPhoneInfo::CPhoneInfo(CTelephony &telephony) : CTelephonyInfo(telephony),
    m_phoneIdV1Pckg(m_phoneIdV1)
{
 TRACES (qDebug() << "CPhoneInfo::CPhoneInfo<---");
    m_telephony.GetPhoneId(iStatus, m_phoneIdV1Pckg);

    makeRequest();

    TBuf<CTelephony::KPhoneSerialNumberSize> imei = m_phoneIdV1.iSerialNumber;
    m_imei = QString::fromUtf16(imei.Ptr(), imei.Length());

    TBuf<CTelephony::KPhoneManufacturerIdSize> manufacturer = m_phoneIdV1.iManufacturer;
    m_manufacturer = QString::fromUtf16(manufacturer.Ptr(), manufacturer.Length());

    TBuf<CTelephony::KPhoneModelIdSize> model = m_phoneIdV1.iModel;
    m_model = QString::fromUtf16(model.Ptr(), model.Length());
 TRACES (qDebug() << "CPhoneInfo::CPhoneInfo--->");
}

CPhoneInfo::~CPhoneInfo()
{
    Cancel();
}

void CPhoneInfo::DoCancel()
{
   m_telephony.CancelAsync(CTelephony::EGetPhoneIdCancel);
}

QString CPhoneInfo::imei() const
{
    return m_imei;
}

QString CPhoneInfo::manufacturer() const
{
    return m_manufacturer;
}

QString CPhoneInfo::model() const
{
    return m_model;
}

CSubscriberInfo::CSubscriberInfo(CTelephony &telephony) : CTelephonyInfo(telephony),
    m_subscriberIdV1Pckg(m_subscriberIdV1)
{
 TRACES (qDebug() << "CSubscriberInfo::CSubscriberInfo<---");
    m_telephony.GetSubscriberId(iStatus, m_subscriberIdV1Pckg);
    makeRequest();

    TBuf<CTelephony::KIMSISize> imsi = m_subscriberIdV1.iSubscriberId;
    m_imsi = QString::fromUtf16(imsi.Ptr(), imsi.Length());
 TRACES (qDebug() << "CSubscriberInfo::CSubscriberInfo--->");
}

CSubscriberInfo::~CSubscriberInfo()
{
    Cancel();
}

void CSubscriberInfo::DoCancel()
{
    m_telephony.CancelAsync(CTelephony::EGetSubscriberIdCancel);
}

QString CSubscriberInfo::imsi() const
{
    return m_imsi;
}

CBatteryInfo::CBatteryInfo(CTelephony &telephony) : CTelephonyInfo(telephony),
    m_initializing(true), m_batteryInfoV1Pckg(m_batteryInfoV1)
{
 TRACES (qDebug() << "CBatteryInfo::CBatteryInfo<---");
    m_telephony.GetBatteryInfo(iStatus, m_batteryInfoV1Pckg);

    makeRequest();

    m_batteryLevel = m_batteryInfoV1.iChargeLevel;
    m_previousBatteryLevel = m_batteryLevel;

    startMonitoring();
 TRACES (qDebug() << "CBatteryInfo::CBatteryInfo--->");
}

CBatteryInfo::~CBatteryInfo()
{
    Cancel();
}

void CBatteryInfo::RunL()
{
 TRACES (qDebug() << "CBatteryInfo::RunL<---");
    if (m_initializing) {
         CTelephonyInfo::RunL();
         m_initializing = false;
    } else {
        m_batteryLevel = m_batteryInfoV1.iChargeLevel;
        foreach (MTelephonyInfoObserver *observer, m_observers) {
            if (m_batteryLevel != m_previousBatteryLevel) {
                observer->batteryLevelChanged();
            }
        }
    }
    m_previousBatteryLevel = m_batteryLevel;
    startMonitoring();
 TRACES (qDebug() << "CBatteryInfo::RunL--->");
}

void CBatteryInfo::DoCancel()
{
 TRACES (qDebug() << "CBatteryInfo::DoCancel<---");
    if (m_initializing) {
        m_telephony.CancelAsync(CTelephony::EGetBatteryInfoCancel);
    } else {
        m_telephony.CancelAsync(CTelephony::EBatteryInfoChangeCancel);
    }
 TRACES (qDebug() << "CBatteryInfo::DoCancel--->");
}


int CBatteryInfo::batteryLevel() const
{
    return m_batteryLevel;
}

void CBatteryInfo::startMonitoring()
{
 TRACES (qDebug() << "CBatteryInfo::startMonitoring<---");
    if (!IsActive()) {
        m_telephony.NotifyChange(iStatus, CTelephony::EBatteryInfoChange, m_batteryInfoV1Pckg);
        SetActive();
    }
 TRACES (qDebug() << "CBatteryInfo::startMonitoring--->");
}

CCellNetworkInfo::CCellNetworkInfo(CTelephony &telephony) : CTelephonyInfo(telephony),
    m_initializing(true), m_networkInfoV1Pckg(m_networkInfoV1)
{
 TRACES (qDebug() << "CCellNetworkInfo::CCellNetworkInfo<---");
    m_telephony.GetCurrentNetworkInfo(iStatus, m_networkInfoV1Pckg);
    makeRequest();

    m_cellId = m_networkInfoV1.iCellId;
    m_previouscellId = m_cellId;
    m_locationAreaCode = m_networkInfoV1.iLocationAreaCode;

    TBuf<CTelephony::KNetworkIdentitySize> networkId = m_networkInfoV1.iNetworkId;
    m_networkId = QString::fromUtf16(networkId.Ptr(), networkId.Length());
    m_previousNetworkId = m_networkId;

    TBuf<CTelephony::KNetworkCountryCodeSize> countryCode = m_networkInfoV1.iCountryCode;
    m_countryCode = QString::fromUtf16(countryCode.Ptr(), countryCode.Length());
    m_previousCountryCode = m_countryCode;
    TBuf<CTelephony::KNetworkLongNameSize> longName = m_networkInfoV1.iLongName;
    if (longName.Length() > 0) {
        m_networkName = QString::fromUtf16(longName.Ptr(), longName.Length());
    } else {
        TBuf<CTelephony::KNetworkDisplayTagSize> displayTag = m_networkInfoV1.iDisplayTag;
        m_networkName = QString::fromUtf16(displayTag.Ptr(), displayTag.Length());
    }
    m_previousNetworkName = m_networkName;

    m_networkMode = m_networkInfoV1.iMode;
    m_previousNetworkMode = m_networkMode;

    startMonitoring();
 TRACES (qDebug() << "CCellNetworkInfo::CCellNetworkInfo--->");
}

CCellNetworkInfo::~CCellNetworkInfo()
{
    Cancel();
}

void CCellNetworkInfo::RunL()
{
 TRACES (qDebug() << "CCellNetworkInfo::RunL<---");
    if (m_initializing) {
        CTelephonyInfo::RunL();
        m_initializing = false;
    } else {
        if (iStatus != KErrNone) return; //To avoid looping if app doesn't have ReadDeviceData caps
        m_cellId = m_networkInfoV1.iCellId;
        m_locationAreaCode = m_networkInfoV1.iLocationAreaCode;

        TBuf<CTelephony::KNetworkIdentitySize> networkId = m_networkInfoV1.iNetworkId;
            m_networkId = QString::fromUtf16(networkId.Ptr(), networkId.Length());

        TBuf<CTelephony::KNetworkCountryCodeSize> countryCode = m_networkInfoV1.iCountryCode;
            m_countryCode = QString::fromUtf16(countryCode.Ptr(), countryCode.Length());

        TBuf<CTelephony::KNetworkLongNameSize> longName = m_networkInfoV1.iLongName;
        if (longName.Length() > 0) {
            m_networkName = QString::fromUtf16(longName.Ptr(), longName.Length());
        } else {
            TBuf<CTelephony::KNetworkDisplayTagSize> displayTag = m_networkInfoV1.iDisplayTag;
            m_networkName = QString::fromUtf16(displayTag.Ptr(), displayTag.Length());
        }

        m_networkMode = m_networkInfoV1.iMode;

        foreach (MTelephonyInfoObserver *observer, m_observers) {
            if (m_networkId != m_previousNetworkId) {
                observer->networkCodeChanged();
            }
            if (m_countryCode != m_previousCountryCode) {
                observer->countryCodeChanged();
            }
            if (m_networkName != m_previousNetworkName) {
                observer->networkNameChanged();
            }
            if (m_networkMode != m_previousNetworkMode) {
                observer->networkModeChanged();
            }
            if (m_cellId != m_previouscellId) {
                observer->changedCellId(m_cellId);
            }
        }
        m_previousNetworkId = m_networkId;
        m_previousCountryCode = m_countryCode;
        m_previousNetworkName = m_networkName;
        m_previousNetworkMode = m_networkMode;
        m_previouscellId = m_cellId;
        startMonitoring();
    }
 TRACES (qDebug() << "CCellNetworkInfo::RunL--->");
}

void CCellNetworkInfo::DoCancel()
{
 TRACES (qDebug() << "CCellNetworkInfo::DoCancel--->");
    if (m_initializing) {
        m_telephony.CancelAsync(CTelephony::EGetCurrentNetworkInfoCancel);
    } else {
        m_telephony.CancelAsync(CTelephony::ECurrentNetworkInfoChangeCancel);
    }
 TRACES (qDebug() << "CCellNetworkInfo::DoCancel<---");
}

int CCellNetworkInfo::cellId() const
{
    return m_cellId;
}

int CCellNetworkInfo::locationAreaCode() const
{
    return m_locationAreaCode;
}

QString CCellNetworkInfo::countryCode() const
{
    return m_countryCode;
}

QString CCellNetworkInfo::networkCode() const
{
    return m_networkId;
}

QString CCellNetworkInfo::networkName() const
{
    return m_networkName;
}

QString CCellNetworkInfo::homeNetworkCode()
    {
#ifdef ETELMM_SUPPORTED
        RTelServer telServer;
        RMobilePhone mobilePhone;
        TInt error = telServer.Connect();
        if ( error != KErrNone )
            {
            return QString();
            }
        error = telServer.LoadPhoneModule( KMmTsyModuleName );
        if ( error != KErrNone )
            {
            telServer.Close();
            return QString();
            }
        RTelServer::TPhoneInfo phoneInfo;
        const TInt KPhoneIndex = 0;
        error = telServer.GetPhoneInfo( KPhoneIndex, phoneInfo );
        if ( error != KErrNone )
            {
            telServer.Close();
            return QString();
            }
        error = mobilePhone.Open( telServer, phoneInfo.iName );
        if ( error != KErrNone )
            {
            telServer.Close();
            return QString();
            }
          TRequestStatus networkStatus;
          RMobilePhone::TMobilePhoneNetworkInfoV1 infov1;
          RMobilePhone::TMobilePhoneNetworkInfoV1Pckg statusPkg(infov1);
          mobilePhone.GetHomeNetwork(networkStatus, statusPkg);
          User::WaitForRequest(networkStatus);
          mobilePhone.Close();
          telServer.Close();
          if (networkStatus == KErrNone)
              {
              QString homeNetworkCode= QString::fromUtf16(infov1.iNetworkId.Ptr(), infov1.iNetworkId.Length());
              return homeNetworkCode;
              }
           else
               return QString();
#else
     return QString();
#endif
    }

CTelephony::TNetworkMode CCellNetworkInfo::networkMode() const
{
    return m_networkMode;
}

void CCellNetworkInfo::startMonitoring()
{
 TRACES (qDebug() << "CCellNetworkInfo::startMonitoring<---");
 if (!IsActive()) {
    m_telephony.NotifyChange(iStatus, CTelephony::ECurrentNetworkInfoChange, m_networkInfoV1Pckg);
    SetActive();
  }
 TRACES (qDebug() << "CCellNetworkInfo::startMonitoring--->");
}

CCellNetworkRegistrationInfo::CCellNetworkRegistrationInfo(CTelephony &telephony) : CTelephonyInfo(telephony),
    m_initializing(true), m_networkRegistrationV1Pckg(m_networkRegistrationV1)
{
 TRACES (qDebug() << "CCellNetworkRegistrationInfo::CCellNetworkRegistrationInfo--->");
    m_telephony.GetNetworkRegistrationStatus(iStatus, m_networkRegistrationV1Pckg);
    makeRequest();

    m_networkStatus = m_networkRegistrationV1.iRegStatus;
    m_previousNetworkStatus = m_networkStatus;

    startMonitoring();
 TRACES (qDebug() << "CCellNetworkRegistrationInfo::CCellNetworkRegistrationInfo<---");
}

CCellNetworkRegistrationInfo::~CCellNetworkRegistrationInfo()
{
    Cancel();
}

void CCellNetworkRegistrationInfo::RunL()
{
 TRACES (qDebug() << "CCellNetworkRegistrationInfo::RunL<---");
    if (m_initializing) {
        CTelephonyInfo::RunL();
        m_initializing = false;
    } else {
        if (iStatus != KErrNone) return; //To avoid looping if app doesn't have ReadDeviceData caps
        m_networkStatus = m_networkRegistrationV1.iRegStatus;

        foreach (MTelephonyInfoObserver *observer, m_observers) {
            if (m_networkStatus != m_previousNetworkStatus) {
                observer->cellNetworkStatusChanged();
            }
        }
        m_previousNetworkStatus = m_networkStatus;
        startMonitoring();
    }
 TRACES (qDebug() << "CCellNetworkRegistrationInfo::RunL--->");
}

void CCellNetworkRegistrationInfo::DoCancel()
{
 TRACES (qDebug() << "CCellNetworkRegistrationInfo::DoCancel<---");
    if (m_initializing) {
        m_telephony.CancelAsync(CTelephony::EGetNetworkRegistrationStatusCancel);
    } else {
        m_telephony.CancelAsync(CTelephony::ENetworkRegistrationStatusChangeCancel);
    }
 TRACES (qDebug() << "CCellNetworkRegistrationInfo::DoCancel--->");
}

CTelephony::TRegistrationStatus CCellNetworkRegistrationInfo::cellNetworkStatus() const
{
    return m_networkStatus;
}

void CCellNetworkRegistrationInfo::startMonitoring()
{
 TRACES (qDebug() << "CCellNetworkRegistrationInfo::startMonitoring<---");
 if (!IsActive()) {
    m_telephony.NotifyChange(iStatus, CTelephony::ENetworkRegistrationStatusChange, m_networkRegistrationV1Pckg);
    SetActive();
  }
TRACES (qDebug() << "CCellNetworkRegistrationInfo::startMonitoring--->");
}

CCellSignalStrengthInfo::CCellSignalStrengthInfo(CTelephony &telephony) : CTelephonyInfo(telephony),
    m_initializing(true), m_signalStrengthV1Pckg(m_signalStrengthV1)
{
 TRACES (qDebug() << "CCellSignalStrengthInfo::CCellSignalStrengthInfo<---");
    m_telephony.GetSignalStrength(iStatus, m_signalStrengthV1Pckg);
    makeRequest();

    m_cellNetworkSignalStrength = m_signalStrengthV1.iSignalStrength;
    m_previousCellNetworkSignalStrength = m_cellNetworkSignalStrength;

    m_signalBar = m_signalStrengthV1.iBar;
    m_previousSignalBar = m_signalBar;

    startMonitoring();
 TRACES (qDebug() << "CCellSignalStrengthInfo::CCellSignalStrengthInfo--->");
}

CCellSignalStrengthInfo::~CCellSignalStrengthInfo()
{
    Cancel();
}

void CCellSignalStrengthInfo::RunL()
{
 TRACES (qDebug() << "CCellSignalStrengthInfo::RunL<---");
    if (m_initializing) {
        CTelephonyInfo::RunL();
        m_initializing = false;
    } else {
        if (iStatus != KErrNone) return; //To avoid looping if app doesn't have ReadDeviceData caps
        m_cellNetworkSignalStrength = m_signalStrengthV1.iSignalStrength;
        m_signalBar = m_signalStrengthV1.iBar;

        if (m_signalBar != m_previousSignalBar) {
            foreach (MTelephonyInfoObserver *observer, m_observers) {
                observer->cellNetworkSignalStrengthChanged();
            }
        }
        m_previousSignalBar = m_signalBar;
        startMonitoring();
    }
 TRACES (qDebug() << "CCellSignalStrengthInfo::RunL--->");
}

void CCellSignalStrengthInfo::DoCancel()
{
 TRACES (qDebug() << "CCellSignalStrengthInfo::DoCancel<---");
    if (m_initializing) {
        m_telephony.CancelAsync(CTelephony::EGetSignalStrengthCancel);
    } else {
        m_telephony.CancelAsync(CTelephony::ESignalStrengthChangeCancel);
    }
 TRACES (qDebug() << "CCellSignalStrengthInfo::DoCancel--->");
}

int CCellSignalStrengthInfo::cellNetworkSignalStrength() const
{
    //Workaround solution based on the number of signal bars (max. 7)
    return int((TReal(m_signalBar) * 100.0 + 0.5) / 7.0);
}

void CCellSignalStrengthInfo::startMonitoring()
{
 TRACES (qDebug() << "CCellSignalStrengthInfo::startMonitoring<---");
 if (!IsActive()) {
    m_telephony.NotifyChange(iStatus, CTelephony::ESignalStrengthChange, m_signalStrengthV1Pckg);
    SetActive();
  }
 TRACES (qDebug() << "CCellSignalStrengthInfo::startMonitoring--->");
}
