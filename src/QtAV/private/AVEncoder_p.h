/******************************************************************************
    QtAV:  Media play library based on Qt and FFmpeg
    Copyright (C) 2015 Wang Bin <wbsecg1@gmail.com>

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

#ifndef QTAV_AVENCODER_P_H
#define QTAV_AVENCODER_P_H

#include <QtCore/QVariant>
#include "QtAV/Packet.h"
#include "QtAV/private/AVCompat.h"

namespace QtAV {

class Q_AV_PRIVATE_EXPORT AVEncoderPrivate : public DPtrPrivate<AVEncoder>
{
public:
    AVEncoderPrivate():
        avctx(0)
      , is_open(false)
      , dict(0)
    {
    }
    virtual ~AVEncoderPrivate() {
        if (dict) {
            av_dict_free(&dict);
        }
    }
    virtual bool open() {return true;}
    virtual bool close() {return true;}
    // used iff avctx != null
    void applyOptionsForDict();
    void applyOptionsForContext();

    AVCodecContext *avctx; // null if not avcodec. allocated in ffmpeg based encoders
    bool is_open;
    QString codec_name;
    QVariantHash options;
    AVDictionary *dict; // null if not avcodec
    Packet packet;
};

class AudioResampler;
class AudioEncoderPrivate : public AVEncoderPrivate
{
public:
    AudioEncoderPrivate();
    virtual ~AudioEncoderPrivate();

    AudioResampler *resampler;
};

class Q_AV_PRIVATE_EXPORT VideoEncoderPrivate : public AVEncoderPrivate
{
public:
    VideoEncoderPrivate():
        AVEncoderPrivate()
      , width(0)
      , height(0)
    {}
    virtual ~VideoEncoderPrivate() {}
    int width, height;
};
} //namespace QtAV
#endif // QTAV_AVENCODER_P_H
