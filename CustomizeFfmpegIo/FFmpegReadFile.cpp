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

int64_t seekCallback(void *opaque, int64_t offset, int whence){
	if (whence == AVSEEK_SIZE){
		return 2922084;
	}

	printf("offset:%lld \n", offset);
	return offset;
}

FFmpegReadFile::FFmpegReadFile(std::string &file) : mFile(file){
	av_register_all();
}


FFmpegReadFile::~FFmpegReadFile(void){
	if (mAvFmtCtx != NULL){
		avformat_close_input(&mAvFmtCtx);
	}
	
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
			char szBuffer[512] = { 0 };
			av_strerror(ret, szBuffer, sizeof(szBuffer));
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

//////////////////////////////////////////////
CustomizedFile::CustomizedFile(std::string &file) : FFmpegReadFile(file){

}
CustomizedFile::~CustomizedFile(){

}
bool CustomizedFile::init(const AVFormatContext *otherFmt){
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
	ret = customize_stream_info(mAvFmtCtx, otherFmt);
	if (ret < 0){
		return false;
	}

	return true;
}
int CustomizedFile::customize_stream_info(AVFormatContext *avformat,const AVFormatContext *otherFmt){
	if (avformat == NULL){
		return -1;
	}

	int audio_index = 1;
	int video_index = 0;
	if (avformat->nb_streams == 0){
		AVStream *st = avformat_new_stream(avformat, NULL);
		if (st == NULL){
			return -1;
		}
		st->codec->codec_type = AVMEDIA_TYPE_VIDEO;

		AVStream *st_audio = avformat_new_stream(avformat, NULL);
		if (st_audio == NULL){
			return -1;
		}
		st_audio->codec->codec_type = AVMEDIA_TYPE_AUDIO;

	}

	avcodec_parameters_copy(avformat->streams[0]->codecpar, otherFmt->streams[0]->codecpar);
	avcodec_parameters_copy(avformat->streams[1]->codecpar, otherFmt->streams[1]->codecpar);

	avformat->streams[0]->time_base = otherFmt->streams[0]->time_base;
	avformat->streams[0]->r_frame_rate = otherFmt->streams[0]->r_frame_rate;

	return 0;
}