#pragma once
extern "C"{
#include <libavformat/avformat.h>
};

enum MediaType { M_Video, M_Audio, M_ALL};
class FFmpegReadFile{
public:
	FFmpegReadFile(std::string &file);
	virtual ~FFmpegReadFile();

	virtual bool init();

	AVPacket *getPacket(int type);
	AVFormatContext *getContext() { return mAvFmtCtx; }

	AVCodecParameters *getVideoParam() { return mAvFmtCtx->streams[0]->codecpar; }
	AVCodecParameters *getAudioParam() { return mAvFmtCtx->streams[1]->codecpar; }

protected:
	std::string mFile;
	AVFormatContext *mAvFmtCtx;
};


class CustomizedFile : public FFmpegReadFile{
public:
	CustomizedFile(std::string &file);
	~CustomizedFile();

	virtual bool init(const AVFormatContext *otherFmt);

private:
	int customize_stream_info(AVFormatContext *avformat, const AVFormatContext *otherFmt);

	
};
