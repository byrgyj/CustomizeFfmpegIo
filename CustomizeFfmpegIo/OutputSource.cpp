#include "StdAfx.h"
#include "OutputSource.h"

FILE *gOutPut = NULL;

int outputWritePacket(void *opaque, uint8_t *buf, int buf_size){
	if (gOutPut != NULL){
		int sz = fwrite(buf, 1, buf_size, gOutPut);
		return sz;
	}
	return buf_size;
}


OutputSource::OutputSource(AVFormatContext *inputCtx) : mInputCtx(inputCtx) {
	
}


OutputSource::~OutputSource()
{
	if (mOutputCtx != NULL){
		av_write_trailer(mOutputCtx);
	}

	avformat_free_context(mOutputCtx);
}

bool OutputSource::init() {

	gOutPut = fopen("./fmp4/out.ts", "wb");
	if (gOutPut == NULL){
		return false;
	}

	int ret = avformat_alloc_output_context2(&mOutputCtx, NULL, "mpegts", NULL);
	if (ret < 0){
		return false;
	}

	char error[512] = { 0 };
	av_strerror(ret, error, sizeof(error));


	int size = 1024;
	uint8_t *buffer = (uint8_t*) av_malloc(size);

	AVIOContext *outAvio = avio_alloc_context(buffer, size, 1, NULL, NULL, outputWritePacket, NULL);
	mOutputCtx->pb = outAvio;

	for (int i = 0; i < mInputCtx->nb_streams; i++){
		AVStream *stream = avformat_new_stream(mOutputCtx, NULL);
		avcodec_parameters_copy(stream->codecpar, mInputCtx->streams[i]->codecpar);
	}

	//ret = avio_open(&mOutputCtx->pb, url, AVIO_FLAG_READ_WRITE);
	ret = avformat_write_header(mOutputCtx, NULL);

	return ret == 0;
}

void OutputSource::writePacket(AVPacket *pkt){
	if (pkt == NULL){
		return;
	}

	pkt->pts = av_rescale_q_rnd(pkt->pts, mInputCtx->streams[pkt->stream_index]->time_base, mOutputCtx->streams[pkt->stream_index]->time_base, (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt->dts = av_rescale_q_rnd(pkt->dts, mInputCtx->streams[pkt->stream_index]->time_base, mOutputCtx->streams[pkt->stream_index]->time_base, (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt->duration = av_rescale_q(pkt->duration, mInputCtx->streams[pkt->stream_index]->time_base, mOutputCtx->streams[pkt->stream_index]->time_base);

	int ret = av_interleaved_write_frame(mOutputCtx, pkt);
}
