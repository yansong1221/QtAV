#pragma once
#include <QtGlobal>

struct AVFrame;
struct AVPacket;
struct AVCodecContext;
struct AVCodecParameters;

namespace QtAV::Wrapper {

	class AVPacketWrapper {
	public:
		AVPacketWrapper();
		AVPacketWrapper(const AVPacket* packet);
		~AVPacketWrapper();
		
	public:
		AVPacketWrapper(const AVPacketWrapper& other);
		AVPacketWrapper& operator=(const AVPacketWrapper& other);

		const ::AVPacket* operator&() const;
		::AVPacket* operator&();

		const ::AVPacket* data() const;
		::AVPacket* data();

		::AVPacket* operator->() const;

		int calculatePacketSize() const;
	private:
		::AVPacket* packet_;
	};

	class AVFrameWapper
	{
	public:
		AVFrameWapper();
		AVFrameWapper(const AVFrame* frame);
		~AVFrameWapper();
	public:

		AVFrameWapper(const AVFrameWapper& other);
		AVFrameWapper(AVFrameWapper&& other);
		AVFrameWapper& operator=(const AVFrameWapper& other);
		AVFrameWapper& operator=(AVFrameWapper&& other);

		const AVFrame* data() const { return frame_; }
		AVFrame* data() { return frame_; }

		const AVFrame* operator&() const { return frame_; }
		AVFrame* operator&() { return frame_; }

		AVFrame* operator->() const { return frame_; }

		qreal getDAR(const AVCodecContext* codec_ctx) const;
		qreal timestamp() const;

		void reset();
	private:
		AVFrame* frame_;
	};

	class AVCodecContextWrapper {
	public:
		AVCodecContextWrapper();
		AVCodecContextWrapper(const AVCodecParameters* par);
		~AVCodecContextWrapper();
	public:
		const AVCodecContext* codecContext() const;
		AVCodecContext* codecContext();
	private:
		AVCodecContext* ctx_;
	};

}