/******************************************************************************
    QtAV:  Media play library based on Qt and FFmpeg
    Copyright (C) 2012-2013 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

#ifndef QTAV_AVPLAYER_H
#define QTAV_AVPLAYER_H

#include <QtAV/AVClock.h>
#include <QtAV/AVDemuxer.h> //TODO: remove AVDemuxer dependency. it's not a public class
#include <QtAV/Statistics.h>

namespace QtAV {

class AVOutput;
class AudioOutput;
class AVThread;
class AudioThread;
class VideoThread;
class AudioDecoder;
class VideoDecoder;
class VideoRenderer;
class AVClock;
class AVDemuxThread;
class Filter;
class VideoCapture;
class OutputSet;
class Q_AV_EXPORT AVPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qint64 startPosition READ startPosition WRITE setStartPosition NOTIFY startPositionChanged)
    Q_PROPERTY(qint64 stopPosition READ stopPosition WRITE setStopPosition NOTIFY stopPositionChanged)
    Q_PROPERTY(qint64 repeat READ repeat WRITE setRepeat NOTIFY repeatChanged)
    Q_PROPERTY(int currentRepeat READ currentRepeat NOTIFY currentRepeatChanged)
public:
    explicit AVPlayer(QObject *parent = 0);
    ~AVPlayer();

    //NOT const. This is the only way to access the clock.
    AVClock* masterClock();
    void setFile(const QString& path);
    QString file() const;
    // force reload even if already loaded. otherwise only reopen codecs if necessary
    bool load(const QString& path, bool reload = true);
    bool load(bool reload = true);
    bool isLoaded() const;
    qreal durationF() const; //unit: s, This function may be removed in the future.
    qint64 duration() const; //unit: ms. media duration
    // the media's property.
    qint64 mediaStartPosition() const;
    //qint64 mediaStopPosition() const;
    Q_DECL_DEPRECATED qreal startPositionF() const; //unit: s. used by loop
    Q_DECL_DEPRECATED qreal stopPositionF() const; //unit: s
    // can set by user. may be not the real media start position.
    qint64 startPosition() const;
    qint64 stopPosition() const; //unit: ms
    Q_DECL_DEPRECATED qreal positionF() const; //unit: s.
    qint64 position() const; //unit: ms
    //0: play once. N: play N+1 times. <0: infinity
    int repeat() const; //or repeatMax()?
    int currentRepeat() const;
    /*
     * set audio/video/subtitle stream to n. n=0, 1, 2..., means the 1st, 2nd, 3rd audio/video/subtitle stream
     * if now==true, player will change to new stream immediatly. otherwise, you should call
     * play() to change to new stream
     * If a new file is set(except the first time) then a best stream will be selected. If the file not changed,
     * e.g. replay, then the stream not change
     * return: false if stream not changed, not valid
     */
    /*
     * steps to change stream:
     *    player.setAudioStream(N, true)
     * or player.setAudioStream(N) && player.play()
     * player then will play from previous position. call
     *    player.seek(player.startPosition())
     * to play at beginning
     */
    bool setAudioStream(int n, bool now = false);
    bool setVideoStream(int n, bool now = false);
    bool setSubtitleStream(int n, bool now = false);
    int currentAudioStream() const;
    int currentVideoStream() const;
    int currentSubtitleStream() const;
    int audioStreamCount() const;
    int videoStreamCount() const;
    int subtitleStreamCount() const;
    /*!
     * \brief capture and save current frame to "appdir/filename_pts.png".
     * To capture with custom configurations, such as name and dir, use
     * VideoCapture api through AVPlayer::videoCapture()
     * \return
     */
    bool captureVideo();
    VideoCapture *videoCapture();
    /*
     * replay without parsing the stream if it's already loaded.
     * to force reload the stream, close() then play()
     */
    bool play(const QString& path);
    bool isPlaying() const;
    bool isPaused() const;
    //this will install the default EventFilter. To use customized filter, register after this
    void addVideoRenderer(VideoRenderer *renderer);
    void removeVideoRenderer(VideoRenderer *renderer);
    void clearVideoRenderers();
    void setRenderer(VideoRenderer* renderer);
    VideoRenderer* renderer();
    QList<VideoRenderer*> videoOutputs();
    void setAudioOutput(AudioOutput* ao);

    /*!
     * To change audio format, you should set both AudioOutput's format and AudioResampler's format
     * So signals/slots is a better solution.
     * TODO: AudioOutput.audioFormatChanged (signal)---AudioResampler.setOutAudioFormat (slot)
     */
    AudioOutput* audio();
    void enableAudio(bool enable = true);
    void disableAudio(bool disable = true);
    void setMute(bool mute);
    bool isMute() const;
    /*!
     * \brief setSpeed set playing speed.
     * \param speed  speed > 0. 1.0: normal speed
     */
    void setSpeed(qreal speed);
    qreal speed() const;
    /*
     * only 1 event filter is available. the previous one will be removed.
     * setPlayerEventFilter(0) will remove the event filter.
     * qApp->installEventFilter will be called
     */
    void setPlayerEventFilter(QObject *obj);

    Statistics& statistics();
    const Statistics& statistics() const;

    /*
     * install the filter in AVThread. Filter will apply before rendering data
     * return false if filter is already registered or audio/video thread is not ready(will install when ready)
     */
    bool installAudioFilter(Filter *filter);
    bool installVideoFilter(Filter *filter);
    bool uninstallFilter(Filter *filter);

signals:
    void paused(bool p);
    void started();
    void stopped();
    void speedChanged(qreal speed);
    void repeatChanged(int r);
    void currentRepeatChanged(int r);
    void startPositionChanged(qint64 position);
    void stopPositionChanged(qint64 position);
    void positionChanged(qint64 position);

public slots:
    void togglePause();
    void pause(bool p);
    void play(); //replay
    void stop();
    void playNextFrame();

    /*!
     * \brief setRepeat
     *  repeat max times between startPosition() and endPosition()
     *  max==0: no repeat
     *  max<0: infinity. std::numeric_limits<int>::max();
     * \param max
     */
    void setRepeat(int max);
    /*!
     * \brief startPosition
     *  Used to repeat from startPosition() to endPosition()
     *  startPosition() < 0 equals duration()+startPosition()
     *  startPosition() == 0 means start at the beginning of media stream
     *  (may be not exactly equals 0, seek to demuxer.startPosition()/startTime())
     * \return
     */
    void setStartPosition(qint64 pos);
    /*!
     * \brief endPosition
     *  endPosition() < 0: equals duration()
     * \return
     */
    void setStopPosition(qint64 pos);
    /*!
     * \brief setPosition equals to seek(qreal)
     * \param position in ms
     */
    void setPosition(qint64 position);
    void seek(qreal r); // r: [0, 1]
    void seek(qint64 pos); //ms. same as setPosition(pos)
    void seekForward();
    void seekBackward();
    void updateClock(qint64 msecs); //update AVClock's external clock

private slots:
    void stopFromDemuxerThread();
    void aboutToQuitApp();

protected:
    // TODO: set position check timer interval
    virtual void timerEvent(QTimerEvent *);

private:
    void initStatistics();
    bool setupAudioThread();
    bool setupVideoThread();
    template<class Out>
    void setAVOutput(Out*& pOut, Out* pNew, AVThread* thread);
    //TODO: addAVOutput()


    // TODO: dptr
    bool loaded;
    AVFormatContext	*formatCtx; //changed when reading a packet
    QString path;

    /*
     * unit: s. 0~1. stream's start time/duration(). or last position/duration() if change to new stream
     * auto set to 0 if stop(). to stream start time if load()
     *
     * -1: used by play() to get current playing position
     */
    qint64 last_position; //last_pos
    bool reset_state;
    qint64 start_position, stop_position;
    int repeat_max, repeat_current;
    int timer_id; //notify position change and check AB repeat range. active when playing

    //the following things are required and must be set not null
    AVDemuxer demuxer;
    AVDemuxThread *demuxer_thread;
    AVClock *clock;
    VideoRenderer *_renderer; //list?
    AudioOutput *_audio;
    AudioDecoder *audio_dec;
    VideoDecoder *video_dec;
    AudioThread *audio_thread;
    VideoThread *video_thread;

    //tODO: (un)register api
    QObject *event_filter;
    VideoCapture *video_capture;
    Statistics mStatistics;
    qreal mSpeed;
    bool ao_enable;
    OutputSet *mpVOSet, *mpAOSet;
};

} //namespace QtAV
#endif // QTAV_AVPLAYER_H
