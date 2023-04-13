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
	{
		*this = other;
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
