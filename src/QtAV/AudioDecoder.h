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

#ifndef QAV_AUDIODECODER_H
#define QAV_AUDIODECODER_H

#include <QtAV/AVDecoder.h>

//TODO: decoder.in/outAudioFormat()?
namespace QtAV {

class AudioResampler;
class AudioDecoderPrivate;
class Q_AV_EXPORT AudioDecoder : public AVDecoder
{
    Q_DISABLE_COPY(AudioDecoder)
    DPTR_DECLARE_PRIVATE(AudioDecoder)
public:
    AudioDecoder();
    virtual bool prepare() Q_DECL_OVERRIDE;
    QTAV_DEPRECATED virtual bool decode(const QByteArray &encoded) Q_DECL_OVERRIDE;
    virtual bool decode(const Packet& packet) Q_DECL_OVERRIDE;
    virtual QByteArray data() const; //decoded data
    AudioResampler *resampler();
};

} //namespace QtAV
#endif // QAV_AUDIODECODER_H
