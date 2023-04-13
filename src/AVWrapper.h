#pragma once

struct AVFrame;
struct AVPacket;
struct AVCodecContext;
struct AVCodecParameters;

namespace QtAV::Wrapper {

	class AVPacketWrapper {
	public:
		AVPacketWrapper();
		~AVPacketWrapper();
	public:
		AVPacketWrapper(const AVPacketWrapper& other);
		AVPacketWrapper& operator=(const AVPacketWrapper& other);

		const ::AVPacket* operator&() const;
		::AVPacket* operator&();

		const ::AVPacket* data() const;
		::AVPacket* data();

		::AVPacket* operator->() const;
	private:
		::AVPacket* packet_;
	};

	class AVFrameWapper
	{
	public:
		AVFrameWapper();
		~AVFrameWapper();
	public:
		const AVFrame* data() const { return frame_; }
		AVFrame* data() { return frame_; }

		const AVFrame* operator&() const { return frame_; }
		AVFrame* operator&() { return frame_; }

		AVFrame* operator->() const { return frame_; }
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