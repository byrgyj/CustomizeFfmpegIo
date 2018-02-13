#pragma once

extern "C"{
#include <libavformat/avformat.h>
};

class OutputSource
{
public:
	OutputSource(AVFormatContext *ctx);
	~OutputSource();

	bool init();

	void writePacket(AVPacket *pkt);
private:
	AVFormatContext *mOutputCtx;
	AVFormatContext *mInputCtx;
};

