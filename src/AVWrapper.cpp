#include "AVWrapper.h"
#include <memory>
#include <stdexcept>

extern "C"
{
#include <libavcodec/packet.h>
#include <libavcodec/avcodec.h>
}


namespace QtAV::Wrapper {

	AVPacketWrapper::AVPacketWrapper()
		:packet_(::av_packet_alloc())
	{

	}

	AVPacketWrapper::AVPacketWrapper(const AVPacketWrapper& other)
		: AVPacketWrapper()
	{
		*this = other;
	}

	AVPacketWrapper::AVPacketWrapper(const AVPacket* packet)
		: packet_(::av_packet_clone(packet))
	{

	}

	AVPacketWrapper::~AVPacketWrapper()
	{
		::av_packet_free(&packet_);
	}

	const ::AVPacket* AVPacketWrapper::operator&() const
	{
		return packet_;
	}

	::AVPacket* AVPacketWrapper::operator&()
	{
		return packet_;
	}

	const ::AVPacket* AVPacketWrapper::data() const
	{
		return packet_;
	}

	::AVPacket* AVPacketWrapper::data()
	{
		return packet_;
	}

	AVPacketWrapper& AVPacketWrapper::operator=(const AVPacketWrapper& other)
	{
		if (this == std::addressof(other))
			return *this;

		av_packet_unref(packet_);
		av_packet_ref(packet_, other.data());
		return *this;
	}

	::AVPacket* AVPacketWrapper::operator->() const
	{
		return packet_;
	}

	int AVPacketWrapper::calculatePacketSize() const
	{
		auto dataSize = packet_->size;
		for (auto i = 0; i < packet_->side_data_elems; ++i)
			dataSize += packet_->side_data[i].size;
		return dataSize;
	}

	AVFrameWapper::AVFrameWapper()
		:frame_(av_frame_alloc())
	{
		
	}

	AVFrameWapper::AVFrameWapper(const AVFrame* frame)
		: frame_(::av_frame_clone(frame))
	{

	}

	AVFrameWapper::AVFrameWapper(const AVFrameWapper& other)
		: AVFrameWapper()
	{
		*this = other;
	}

	AVFrameWapper::AVFrameWapper(AVFrameWapper&& other)
		: AVFrameWapper()
	{
		*this = std::move(other);
	}

	AVFrameWapper::~AVFrameWapper()
	{
		av_frame_free(&frame_);
	}

	qreal AVFrameWapper::getDAR(const AVCodecContext* codec_ctx) const
	{
		// lavf 54.5.100 av_guess_sample_aspect_ratio: stream.sar > frame.sar
		qreal dar = 0;
		if (frame_->height > 0)
			dar = (qreal)frame_->width / (qreal)frame_->height;
		// prefer sar from AVFrame if sar != 1/1
		if (frame_->sample_aspect_ratio.num > 1)
			dar *= av_q2d(frame_->sample_aspect_ratio);
		else if (codec_ctx && codec_ctx->sample_aspect_ratio.num > 1) // skip 1/1
			dar *= av_q2d(codec_ctx->sample_aspect_ratio);
		return dar;
	}

	qreal AVFrameWapper::timestamp() const
	{
		return double(frame_->pts) / 1000.0;
	}

	AVFrameWapper& AVFrameWapper::operator=(const AVFrameWapper& other)
	{
		if (this == std::addressof(other))
			return *this;
		av_frame_unref(frame_);
		av_frame_ref(frame_, other.data());
		return *this;
	}

	AVFrameWapper& AVFrameWapper::operator=(AVFrameWapper&& other)
	{
		if (this == std::addressof(other))
			return *this;
		av_frame_unref(frame_);
		av_frame_move_ref(frame_, other.data());
		return *this;
	}

	void AVFrameWapper::reset()
	{
		av_frame_free(&frame_);
		frame_ = av_frame_alloc();
	}

	AVCodecContextWrapper::AVCodecContextWrapper()
		:ctx_(avcodec_alloc_context3(nullptr))
	{

	}

	AVCodecContextWrapper::AVCodecContextWrapper(const AVCodecParameters* par)
		: AVCodecContextWrapper()
	{
		if (avcodec_parameters_to_context(ctx_, par) < 0)
		{
			throw std::runtime_error("avcodec_parameters_to_context");
		}
	}

	AVCodecContextWrapper::~AVCodecContextWrapper()
	{
		avcodec_free_context(&ctx_);
	}

	const AVCodecContext* AVCodecContextWrapper::codecContext() const
	{
		return ctx_;
	}

	AVCodecContext* AVCodecContextWrapper::codecContext()
	{
		return ctx_;
	}



}
