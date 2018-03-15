#pragma once
extern "C"{
#include <libavformat/avformat.h>
};

enum MediaType { M_Video, M_Audio, M_ALL};
class FFmpegReadFile{
public:
	FFmpegReadFile(std::string &file);
	~FFmpegReadFile();

	bool init();

	AVPacket *getPacket(int type);
	AVFormatContext *getContext() { return mAvFmtCtx; }
private:
	std::string mFile;
	AVFormatContext *mAvFmtCtx;
};

