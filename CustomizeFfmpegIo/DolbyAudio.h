#pragma once

extern "C"{
#include <libavformat/avformat.h>
};

#include <string>
#include <stdint.h>

class DolbyAudio
{
public:
	DolbyAudio(std::string audioHeaderPath, std::string audioDataPath);
	~DolbyAudio();

	bool init();
	AVFormatContext *getContext() { return mFmtCtx; }
	AVPacket *getPacket();
private:
	std::string mAudioHeaderPath;
	std::string mAudioDataPath;

	AVFormatContext *mFmtCtx;
};

