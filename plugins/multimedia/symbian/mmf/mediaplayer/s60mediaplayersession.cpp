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

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <mmf/common/mmferrors.h>
#include <qmediatimerange.h>
#include "s60mediaplayersession.h"
#include "s60mediaplayerutils.h"
#include "s60mmtrace.h"

S60MediaPlayerSession::S60MediaPlayerSession(QObject *parent)
    : QObject(parent)
    , m_stream(false)
    , m_playbackRate(0)
    , m_muted(false)
    , m_volume(0)
    , m_state(QMediaPlayer::StoppedState)
    , m_mediaStatus(QMediaPlayer::NoMedia)
    , m_progressTimer(new QTimer(this))
    , m_stalledTimer(new QTimer(this))
    , m_error(KErrNone)
    , m_play_requested(false)
    , m_seekable(true)
    , m_duration(0)
    , m_progressduration(0)
{
    TRACE("S60MediaPlayerSession::S60MediaPlayerSession" << qtThisPtr());
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(tick()));
    connect(m_stalledTimer, SIGNAL(timeout()), this, SLOT(stalled()));
}

S60MediaPlayerSession::~S60MediaPlayerSession()
{
    TRACE("S60MediaPlayerSession::~S60MediaPlayerSession" << qtThisPtr());
}

int S60MediaPlayerSession::volume() const
{
    return m_volume;
}

void S60MediaPlayerSession::setVolume(int volume)
{
    if (m_volume == volume)
        return;
    TRACE("S60MediaPlayerSession::setVolume" << qtThisPtr() << "volume" << volume);
    m_volume = volume;
    emit volumeChanged(m_volume);
    if (!m_muted)
        updateVolume();
}

bool S60MediaPlayerSession::isMuted() const
{
    return m_muted;
}

bool S60MediaPlayerSession::isSeekable() const
{
    return m_seekable;
}

void S60MediaPlayerSession::setMediaStatus(QMediaPlayer::MediaStatus status)
{
    if (m_mediaStatus == status)
        return;
    TRACE("S60MediaPlayerSession::setMediaStatus" << qtThisPtr() << "status" << status);
    m_mediaStatus = status;
    emit mediaStatusChanged(m_mediaStatus);
    if (m_play_requested && m_mediaStatus == QMediaPlayer::LoadedMedia)
        play();
}

void S60MediaPlayerSession::setState(QMediaPlayer::State state)
{
    if (m_state == state)
        return;
    TRACE("S60MediaPlayerSession::setMediaStatus" << qtThisPtr() << "state" << state);
    m_state = state;
    emit stateChanged(m_state);
}

QMediaPlayer::State S60MediaPlayerSession::state() const
{
    return m_state;
}

QMediaPlayer::MediaStatus S60MediaPlayerSession::mediaStatus() const
{
    return m_mediaStatus;
}

void S60MediaPlayerSession::load(const QMediaContent source)
{
    TRACE("S60MediaPlayerSession::load" << qtThisPtr()
          << "source" << source.canonicalUrl().toString());
    m_source = source;
    setMediaStatus(QMediaPlayer::LoadingMedia);
    startStalledTimer();
    m_stream = (source.canonicalUrl().scheme() != "file");
    m_url = source.canonicalUrl();
    TRAPD(err,
        if (m_stream)
            doLoadUrlL(QString2TPtrC(source.canonicalUrl().toString()));
        else
            doLoadL(QString2TPtrC(QDir::toNativeSeparators(source.canonicalUrl().toLocalFile()))));
    setError(err);
    m_isaudiostream = false;
}

bool S60MediaPlayerSession::getIsSeekable() const
{
    return ETrue;
}

TBool S60MediaPlayerSession::isStreaming()
{
    return m_stream;
}

void S60MediaPlayerSession::play()
{
    TRACE("S60MediaPlayerSession::play" << qtThisPtr()
          << "state" << state() << "mediaStatus" << mediaStatus());
    if (   (state() == QMediaPlayer::PlayingState && m_play_requested == false)
        || mediaStatus() == QMediaPlayer::UnknownMediaStatus
        || mediaStatus() == QMediaPlayer::NoMedia
        || mediaStatus() == QMediaPlayer::InvalidMedia)
        return;
    setState(QMediaPlayer::PlayingState);
    if (mediaStatus() == QMediaPlayer::LoadingMedia ||
       (mediaStatus() == QMediaPlayer::StalledMedia &&
        state() == QMediaPlayer::StoppedState)) {
        m_play_requested = true;
        return;
    }
    m_play_requested = false;
    m_duration = duration();
    updateVolume();
    startProgressTimer();
    doPlay();
}

void S60MediaPlayerSession::pause()
{
    TRACE("S60MediaPlayerSession::pause" << qtThisPtr());
    if (state() != QMediaPlayer::PlayingState)
        return;
    if (mediaStatus() == QMediaPlayer::NoMedia ||
        mediaStatus() == QMediaPlayer::InvalidMedia)
        return;
    setState(QMediaPlayer::PausedState);
    stopProgressTimer();
    TRAP_IGNORE(doPauseL());
    m_play_requested = false;
}

void S60MediaPlayerSession::stop()
{
    TRACE("S60MediaPlayerSession::stop" << qtThisPtr());
    if (state() == QMediaPlayer::StoppedState)
        return;
    m_play_requested = false;
    m_state = QMediaPlayer::StoppedState;
    if (mediaStatus() == QMediaPlayer::BufferingMedia ||
        mediaStatus() == QMediaPlayer::BufferedMedia ||
        mediaStatus() == QMediaPlayer::StalledMedia) 
        setMediaStatus(QMediaPlayer::LoadedMedia);
    if (mediaStatus() == QMediaPlayer::LoadingMedia)
        setMediaStatus(QMediaPlayer::UnknownMediaStatus);    
    stopProgressTimer();
    stopStalledTimer();
    doStop();
    emit positionChanged(0);
    emit stateChanged(m_state);
}

void S60MediaPlayerSession::setVideoRenderer(QObject *renderer)
{
    TRACE("S60MediaPlayerSession::setVideoRenderer" << qtThisPtr()
          << "videoRenderer" << renderer);
    Q_UNUSED(renderer);
}

int S60MediaPlayerSession::bufferStatus()
{
    if (state() ==QMediaPlayer::StoppedState)
        return 0;
    if(   mediaStatus() == QMediaPlayer::LoadingMedia
       || mediaStatus() == QMediaPlayer::UnknownMediaStatus
       || mediaStatus() == QMediaPlayer::NoMedia
       || mediaStatus() == QMediaPlayer::InvalidMedia)
        return 0;
    int progress = 0;
    TRAPD(err, progress = doGetBufferStatusL());
    // If buffer status query not supported by codec return 100
    // do not set error
    if (err == KErrNotSupported)
        return 100;
    setError(err);
    return progress;
}

bool S60MediaPlayerSession::isMetadataAvailable() const
{
    return !m_metaDataMap.isEmpty();
}

QVariant S60MediaPlayerSession::metaData(const QString &key) const
{
    return m_metaDataMap.value(key);
}

QVariant S60MediaPlayerSession::metaData(QtMultimediaKit::MetaData key) const
{
    return metaData(metaDataKeyAsString(key));
}

QList<QtMultimediaKit::MetaData> S60MediaPlayerSession::availableMetaData() const
{
    QList<QtMultimediaKit::MetaData> metaDataTags;
    if (isMetadataAvailable()) {
        for (int i=QtMultimediaKit::Title; i<=QtMultimediaKit::ThumbnailImage; ++i) {
            QString metaDataItem = metaDataKeyAsString((QtMultimediaKit::MetaData)i);
            if (!metaDataItem.isEmpty()) {
                if (!metaData(metaDataItem).isNull())
                    metaDataTags.append((QtMultimediaKit::MetaData)i);
            }
        }
    }
    return metaDataTags;
}

QStringList S60MediaPlayerSession::availableExtendedMetaData() const
{
    return m_metaDataMap.keys();
}

QString S60MediaPlayerSession::metaDataKeyAsString(QtMultimediaKit::MetaData key) const
{
    switch(key) {
    case QtMultimediaKit::Title:
        return "title";
    case QtMultimediaKit::AlbumArtist:
        return "artist";
    case QtMultimediaKit::Comment:
        return "comment";
    case QtMultimediaKit::Genre:
        return "genre";
    case QtMultimediaKit::Year:
        return "year";
    case QtMultimediaKit::Copyright:
        return "copyright";
    case QtMultimediaKit::AlbumTitle:
        return "album";
    case QtMultimediaKit::Composer:
        return "composer";
    case QtMultimediaKit::TrackNumber:
        return "albumtrack";
    case QtMultimediaKit::AudioBitRate:
       return "audiobitrate";
    case QtMultimediaKit::VideoBitRate:
        return "videobitrate";
    case QtMultimediaKit::Duration:
        return "duration";
    case QtMultimediaKit::MediaType:
        return "contenttype";
    case QtMultimediaKit::CoverArtImage:
       return "attachedpicture";
    case QtMultimediaKit::SubTitle:
    case QtMultimediaKit::Description:
    case QtMultimediaKit::Category:
    case QtMultimediaKit::Date:
    case QtMultimediaKit::UserRating:
    case QtMultimediaKit::Keywords:
    case QtMultimediaKit::Language:
    case QtMultimediaKit::Publisher:
    case QtMultimediaKit::ParentalRating:
    case QtMultimediaKit::RatingOrganisation:
    case QtMultimediaKit::Size:
    case QtMultimediaKit::AudioCodec:
    case QtMultimediaKit::AverageLevel:
    case QtMultimediaKit::ChannelCount:
    case QtMultimediaKit::PeakValue:
    case QtMultimediaKit::SampleRate:
    case QtMultimediaKit::Author:
    case QtMultimediaKit::ContributingArtist:
    case QtMultimediaKit::Conductor:
    case QtMultimediaKit::Lyrics:
    case QtMultimediaKit::Mood:
    case QtMultimediaKit::TrackCount:
    case QtMultimediaKit::CoverArtUrlSmall:
    case QtMultimediaKit::CoverArtUrlLarge:
    case QtMultimediaKit::Resolution:
    case QtMultimediaKit::PixelAspectRatio:
    case QtMultimediaKit::VideoFrameRate:
    case QtMultimediaKit::VideoCodec:
    case QtMultimediaKit::PosterUrl:
    case QtMultimediaKit::ChapterNumber:
    case QtMultimediaKit::Director:
    case QtMultimediaKit::LeadPerformer:
    case QtMultimediaKit::Writer:
    case QtMultimediaKit::CameraManufacturer:
    case QtMultimediaKit::CameraModel:
    case QtMultimediaKit::Event:
    case QtMultimediaKit::Subject:
    default:
        break;
    }
    return QString();
}

void S60MediaPlayerSession::setMuted(bool muted)
{
    if (muted != m_muted) {
        TRACE("S60MediaPlayerSession::setMuted" << qtThisPtr() << "muted" << muted);
        m_muted = muted;
        emit mutedChanged(m_muted);
        updateVolume();
    }
}

qint64 S60MediaPlayerSession::duration() const
{
    if(   mediaStatus() == QMediaPlayer::LoadingMedia
       || mediaStatus() == QMediaPlayer::UnknownMediaStatus
       || mediaStatus() == QMediaPlayer::NoMedia
       || (mediaStatus() == QMediaPlayer::StalledMedia && state() == QMediaPlayer::StoppedState)
       || mediaStatus() == QMediaPlayer::InvalidMedia)
        return -1;
    qint64 pos = 0;
    TRAP_IGNORE(pos = doGetDurationL());
    return pos;
}

qint64 S60MediaPlayerSession::position() const
{
    if(   mediaStatus() == QMediaPlayer::LoadingMedia
       || mediaStatus() == QMediaPlayer::UnknownMediaStatus
       || mediaStatus() == QMediaPlayer::NoMedia
       || (mediaStatus() == QMediaPlayer::StalledMedia && state() == QMediaPlayer::StoppedState)
       || mediaStatus() == QMediaPlayer::InvalidMedia)
        return 0;
    qint64 pos = 0;
    TRAP_IGNORE(pos = doGetPositionL());
    if (pos < m_progressduration)
        pos = m_duration;
    return pos;
}

void S60MediaPlayerSession::setPosition(qint64 pos)
{
    if (position() == pos)
        return;
    TRACE("S60MediaPlayerSession::setPosition" << qtThisPtr() << "pos" << pos);
    QMediaPlayer::State originalState = state();
    if (originalState == QMediaPlayer::PlayingState)
        pause();
    TRAPD(err, doSetPositionL(pos * 1000));
    setError(err);
    if (err == KErrNone) {
        if (mediaStatus() == QMediaPlayer::EndOfMedia)
            setMediaStatus(QMediaPlayer::LoadedMedia);
    } else if (err == KErrNotSupported) {
        m_seekable = false;
        emit seekableChanged(m_seekable);
    }
    if (originalState == QMediaPlayer::PlayingState)
        play();
    TRAP_IGNORE(m_progressduration = doGetPositionL());
    emit positionChanged(m_progressduration);
}

void S60MediaPlayerSession::setAudioEndpoint(const QString& audioEndpoint)
{
    TRACE("S60MediaPlayerSession::setAudioEndpoint" << qtThisPtr()
          << "audioEndpoint" << audioEndpoint);
    doSetAudioEndpoint(audioEndpoint);
}

void S60MediaPlayerSession::loaded()
{
    TRACE("S60MediaPlayerSession::loaded" << qtThisPtr());
    stopStalledTimer();
    if (m_error == KErrNone || m_error == KErrMMPartialPlayback) {
        setMediaStatus(QMediaPlayer::LoadedMedia);
        TRAPD(err, updateMetaDataEntriesL());
        setError(err);
        emit durationChanged(duration());
        emit positionChanged(0);
        emit videoAvailableChanged(isVideoAvailable());
        emit audioAvailableChanged(isAudioAvailable());
        emit mediaChanged();
        m_seekable = getIsSeekable();
    }
}

void S60MediaPlayerSession::endOfMedia()
{
    TRACE("S60MediaPlayerSession::endOfMedia" << qtThisPtr());
    stopProgressTimer();
    m_state = QMediaPlayer::StoppedState;
    setMediaStatus(QMediaPlayer::EndOfMedia);
    // There is a chance that user might have called play from EOF callback.
    // If we are already in playing state, do not send state change callback.
    emit positionChanged(m_duration);
    if (m_state == QMediaPlayer::StoppedState)
        emit stateChanged(QMediaPlayer::StoppedState);
}

void S60MediaPlayerSession::buffering()
{
    TRACE("S60MediaPlayerSession::buffering" << qtThisPtr());
    startStalledTimer();
    setMediaStatus(QMediaPlayer::BufferingMedia);
    // Buffering cannot happen in stopped state. Hence update the state
    if (state() == QMediaPlayer::StoppedState)
        setState(QMediaPlayer::PausedState);
}

void S60MediaPlayerSession::buffered()
{
    TRACE("S60MediaPlayerSession::buffered" << qtThisPtr());
    stopStalledTimer();
    setMediaStatus(QMediaPlayer::BufferedMedia);
}

void S60MediaPlayerSession::stalled()
{
    TRACE("S60MediaPlayerSession::stalled" << qtThisPtr());
    setMediaStatus(QMediaPlayer::StalledMedia);
}

QMap<QString, QVariant> &S60MediaPlayerSession::metaDataEntries()
{
    return m_metaDataMap;
}

QMediaPlayer::Error S60MediaPlayerSession::fromSymbianErrorToMultimediaError(int error)
{
    switch(error) {
        case KErrNoMemory:
        case KErrNotFound:
        case KErrBadHandle:
        case KErrAbort:
        case KErrNotSupported:
        case KErrCorrupt:
        case KErrGeneral:
        case KErrArgument:
        case KErrPathNotFound:
        case KErrDied:
        case KErrServerTerminated:
        case KErrServerBusy:
        case KErrCompletion:  
        case KErrBadPower:    
        case KErrMMInvalidProtocol:
        case KErrMMInvalidURL:
            return QMediaPlayer::ResourceError;
        
        case KErrMMPartialPlayback:   
            return QMediaPlayer::FormatError;

        case KErrMMAudioDevice:
        case KErrMMVideoDevice:
        case KErrMMDecoder:
        case KErrUnknown:    
            return QMediaPlayer::ServiceMissingError;
            
        case KErrMMNotEnoughBandwidth:
        case KErrMMSocketServiceNotFound:
        case KErrMMNetworkRead:
        case KErrMMNetworkWrite:
        case KErrMMServerSocket:
        case KErrMMServerNotSupported:
        case KErrMMUDPReceive:
        case KErrMMMulticast:
        case KErrMMProxyServer:
        case KErrMMProxyServerNotSupported:
        case KErrMMProxyServerConnect:
        case KErrCouldNotConnect:
            return QMediaPlayer::NetworkError;

        case KErrNotReady:
        case KErrInUse:
        case KErrAccessDenied:
        case KErrLocked:
        case KErrMMDRMNotAuthorized:
        case KErrPermissionDenied:
        case KErrCancel:
        case KErrAlreadyExists:
            return QMediaPlayer::AccessDeniedError;

        case KErrNone:
            return QMediaPlayer::NoError;

        default:
            return QMediaPlayer::ResourceError;
    }
}

int S60MediaPlayerSession::error() const
{
    return m_error;
}

void S60MediaPlayerSession::setError(int error, const QString &errorString, bool forceReset)
{
    if (forceReset) {
        TRACE("S60MediaPlayerSession::setError" << qtThisPtr() << "forceReset");
        m_error = KErrNone;
        emit this->error(QMediaPlayer::NoError, QString());
        return;
    }

    // If error does not change and m_error is reseted without forceReset flag
    if (error == m_error || 
        (m_error != KErrNone && error == KErrNone))
        return;

    TRACE("S60MediaPlayerSession::setError" << qtThisPtr()
          << "error" << error << "errorString" << errorString);

    m_error = error;
    QMediaPlayer::Error mediaError = fromSymbianErrorToMultimediaError(m_error);
    QString symbianError = QString(errorString);

    if (mediaError != QMediaPlayer::NoError) {
        // TODO: fix to user friendly string at some point
        // These error string are only dev usable
        symbianError.append("Symbian:");
        symbianError.append(QString::number(m_error));
    }

    if (m_error == KErrInUse) {
        pause();
    } else if (mediaError != QMediaPlayer::NoError) {
        m_play_requested = false;
        setMediaStatus(QMediaPlayer::InvalidMedia);
        stop();
    }

    emit this->error(mediaError, symbianError);
}

void S60MediaPlayerSession::setAndEmitError(int error)
{
    m_error = error;
    QMediaPlayer::Error rateError = fromSymbianErrorToMultimediaError(error);
    QString symbianError;
    symbianError.append("Symbian:");
    symbianError.append(QString::number(error));
    emit this->error(rateError, symbianError);
}

void S60MediaPlayerSession::tick()
{
    m_progressduration = position();
    emit positionChanged(m_progressduration);
    if (bufferStatus() < 100)
        emit bufferStatusChanged(bufferStatus());
}

void S60MediaPlayerSession::startProgressTimer()
{
    TRACE("S60MediaPlayerSession::startProgressTimer" << qtThisPtr());
    m_progressTimer->start(500);
}

void S60MediaPlayerSession::stopProgressTimer()
{
    TRACE("S60MediaPlayerSession::stopProgressTimer" << qtThisPtr());
    m_progressduration = 0;
    m_progressTimer->stop();
}

void S60MediaPlayerSession::startStalledTimer()
{
    TRACE("S60MediaPlayerSession::startStalledTimer" << qtThisPtr());
    m_stalledTimer->start(30000);
}

void S60MediaPlayerSession::stopStalledTimer()
{
    TRACE("S60MediaPlayerSession::stopStalledTimer" << qtThisPtr());
    m_stalledTimer->stop();
}

void S60MediaPlayerSession::updateVolume()
{
    TRACE("S60MediaPlayerSession::updateVolume" << qtThisPtr());
    // Don't set player volume until media loaded
    if( m_mediaStatus == QMediaPlayer::LoadedMedia
       || (m_mediaStatus == QMediaPlayer::StalledMedia && state() != QMediaPlayer::StoppedState)
       || m_mediaStatus == QMediaPlayer::BufferingMedia
       || m_mediaStatus == QMediaPlayer::BufferedMedia
       || m_mediaStatus == QMediaPlayer::EndOfMedia) {
        TRAPD(err, doSetVolumeL(m_muted ? 0 : m_volume));
        setError(err);
    }
}
