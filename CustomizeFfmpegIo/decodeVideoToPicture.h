#pragma once
extern "C"{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
};


class DecodeVideoToPicture {
public:
    DecodeVideoToPicture(const std::string &file);
    ~DecodeVideoToPicture();


    bool init();
    AVPacket *getPacket(int type);
    int decodeVideoFrame(AVPacket *pkt);
    
private:
    int saveToJPEG(AVFrame *pFrame, int width, int height, int iIndex);
    int saveToBMP(AVFrame *pFrameRGB, int width, int height, int index, int bpp);

private:
    AVFormatContext *mAvFmtCtx;
    AVCodecContext *mVideoCodecCtx;
    AVCodec        *mVideoCodec;

    std::string mInputFile;

    FILE *mYUVFile;
    int64_t mYUVSize;
    int32_t mPacketCount;
    int32_t mFrameCount;
};