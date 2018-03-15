#include "StdAfx.h"
#include "FFmpegReadFile.h"

FILE *gFile = NULL;

int readPacket(void *opaque, uint8_t *buf, int buf_size)
{
	int static totalSize = 0;
	int readSize = 0;

	if (gFile != NULL && ! feof(gFile)){
		readSize = fread(buf, 1, buf_size, gFile);
		totalSize += readSize;
		//printf("readPacket[readSize:%d, totalSize:%d]\n", readSize, totalSize);
		return readSize;
	}

	return readSize;
}


FFmpegReadFile::FFmpegReadFile(std::string &file) : mFile(file){
	av_register_all();
}


FFmpegReadFile::~FFmpegReadFile(void){
}

bool FFmpegReadFile::init() {
	gFile = fopen(mFile.c_str(), "rb");
	if (gFile == NULL){
		return false;
	}

	int bufferSize = 1024;
	uint8_t *buffer = (uint8_t*)av_mallocz(bufferSize);
	AVIOContext *ioContext = avio_alloc_context(buffer, bufferSize, 0, NULL, readPacket, NULL, NULL);

	mAvFmtCtx = avformat_alloc_context();

	mAvFmtCtx->pb = ioContext;

	int ret = avformat_open_input(&mAvFmtCtx, NULL, NULL, NULL);
	if (ret <0){
		return false;
	}

	ret = avformat_find_stream_info(mAvFmtCtx, NULL);
	if (ret < 0){
		return false;
	}

	return true;
}

AVPacket *FFmpegReadFile::getPacket(int type){
	AVPacket *pkt = NULL;

	do 
	{
		pkt = av_packet_alloc();

		int ret = av_read_frame(mAvFmtCtx, pkt);
		if (ret < 0){
			av_packet_free(&pkt);
			return NULL;
		}

		if (pkt->stream_index == type || type == M_ALL){
			break;
		} else {
			av_packet_free(&pkt);
		}
	} while (true);
	



	return pkt;
}