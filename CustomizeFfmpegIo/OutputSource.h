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

	int writePacket(AVPacket *pkt);
private:
	AVFormatContext *mOutputCtx;
	AVFormatContext *mInputCtx;
	int64_t mLastVideoDts;

	//
	int mVideoPktCount;
};

