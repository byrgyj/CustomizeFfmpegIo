#include "StdAfx.h"
#include "DolbyAudio.h"


FILE *gAudioHeaderFile = NULL;
FILE *gAudioDataFile = NULL;

int readAudioPacket(void *opaque, uint8_t *buf, int buf_size)
{
	int static totalSize = 0;
	int readSize = 0;

	if (gAudioHeaderFile != NULL && ! feof(gAudioHeaderFile)){
		readSize = fread(buf, 1, buf_size, gAudioHeaderFile);
		totalSize += readSize;
		printf("readPacket[readSize:%d, totalSize:%d]\n", readSize, totalSize);
		return readSize;
	}

	 if (gAudioDataFile != NULL && !feof(gAudioDataFile)){
		 readSize = fread(buf, 1, buf_size, gAudioDataFile);
	 }
	totalSize += readSize;
	printf("readPacket[readSize:%d, totalSize:%d]\n", readSize, totalSize);
	return readSize;
}



DolbyAudio::DolbyAudio(std::string audioHeaderPath, std::string audioDataPath) : mFmtCtx(NULL), mAudioHeaderPath(audioHeaderPath), mAudioDataPath(audioDataPath) {
	av_register_all();
}


DolbyAudio::~DolbyAudio(){
	if (mFmtCtx != NULL) {
		avformat_close_input(&mFmtCtx);
	}
}

bool DolbyAudio::init(){
	gAudioHeaderFile = fopen(mAudioHeaderPath.c_str(), "rb");
	if (gAudioHeaderFile == NULL){
		return false;
	}

	gAudioDataFile = fopen(mAudioDataPath.c_str(), "rb");
	if (gAudioDataFile == NULL) {
		return false;
	}


	mFmtCtx = avformat_alloc_context();

	int buffSize = 1024;
	uint8_t *buffer = (uint8_t*)av_malloc(buffSize);
	AVIOContext *avio = avio_alloc_context(buffer, buffSize, 0, NULL, readAudioPacket, NULL, NULL);
	if (avio == NULL){
		return false;
	}

	mFmtCtx->pb = avio;

	int ret = avformat_open_input(&mFmtCtx, NULL, NULL, NULL);
	if (ret < 0){
		return false;
	}

	ret = avformat_find_stream_info(mFmtCtx, NULL);
	if (ret < 0){
		return false;
	}



	return true;
}

AVPacket *DolbyAudio::getPacket(){
	if (mFmtCtx == NULL){
		return NULL;
	}

	AVPacket *pkt = av_packet_alloc();

	int ret = av_read_frame(mFmtCtx, pkt);

	if (ret < 0){
		return NULL;
	}

	return pkt;
}