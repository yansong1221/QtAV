/******************************************************************************
    QtAV:  Multimedia framework based on Qt and FFmpeg
    Copyright (C) 2012-2017 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV (from 2013)

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

#ifndef QTAV_STATISTICS_H
#define QTAV_STATISTICS_H

#include <QtAV/QtAV_Global.h>
#include <QtCore/QHash>
#include <QtCore/QTime>
#include <QtCore/QSharedData>
#include <QSize>
#include <QMutex>

/*!
 * values from functions are dynamically calculated
 */
namespace QtAV {

class Q_AV_EXPORT Statistics
{
public:
    Statistics();
    ~Statistics();

    Statistics(const Statistics &other);
    Statistics &operator=(const Statistics &other);

    void reset();

    QString url;
    int bit_rate = 0;
    QString format;
    QTime start_time, duration;
    QHash<QString, QString> metadata;
    class Common {
    public:
        Common();
        //TODO: dynamic bit rate compute
        bool available = false;
        QString codec, codec_long;
        QString decoder;
        QString decoder_detail;
        QTime current_time, total_time, start_time;
        int bit_rate = 0;
        qint64 frames = 0;
        qreal frame_rate = 0; // average fps stored in media stream information
        //union member with ctor, dtor, copy ctor only works in c++11
        /*union {
            audio_only audio;
            video_only video;
        } only;*/
        QHash<QString, QString> metadata;
    } audio, video; //init them

    //from AVCodecContext
    class Q_AV_EXPORT AudioOnly {
    public:
        AudioOnly();
        int sample_rate = 0; ///< samples per second
        int channels = 0;    ///< number of audio channels
        QString channel_layout;
        QString sample_fmt;  ///< sample format
        /**
         * Number of samples per channel in an audio frame.
         * - decoding: may be set by some decoders to indicate constant frame size
         */
        int frame_size = 0;
        /**
         * number of bytes per packet if constant and known or 0
         * Used by some WAV based audio codecs.
         */
        int block_align = 0;
    } audio_only;
    //from AVCodecContext
    class Q_AV_EXPORT VideoOnly {
    public:
        //union member with ctor, dtor, copy ctor only works in c++11
        VideoOnly();
        VideoOnly(const VideoOnly&);
        VideoOnly& operator =(const VideoOnly&);
        ~VideoOnly();
        // compute from pts history
        qreal currentDisplayFPS() const;
        qreal pts() const; // last pts

        int width = -1, height = -1;
        /**
         * Bitstream width / height, may be different from width/height if lowres enabled.
         * - decoding: Set by user before init if known. Codec should override / dynamically change if needed.
         */
        int coded_width = -1, coded_height = -1;
        /**
         * the number of pictures in a group of pictures, or 0 for intra_only
         */
        int gop_size = 0;
        QString pix_fmt;
        int rotate = 0;
        /// return current absolute time (seconds since epcho
        qint64 frameDisplayed(qreal pts); // used to compute currentDisplayFPS()
    private:
        class Private;
        QExplicitlySharedDataPointer<Private> d;
    } video_only;

    double bandwidthRate = 0;
    double videoBandwidthRate = 0;
    double audioBandwidthRate = 0;
    double fps = 0;
    double displayFPS = 0;
    qint64 totalFrames = 0;
    qint64 droppedPackets = 0;
    qint64 droppedFrames = 0;
    qint64 totalKeyFrames = -3;
    QSize realResolution = QSize(0,0);
    int imageBufferSize = 0;
    mutable QMutex mutex;
    std::atomic<bool> resetValues{true};
};

} //namespace QtAV

#endif // QTAV_STATISTICS_H
