// CustomizeFfmpegIo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"

};
#include "Mpegts.h"
#include "DolbyAudio.h"
#include "OutputSource.h"

FILE *logFile = NULL;
FILE *header = NULL;
FILE *file_audio = NULL;
FILE *out_file = NULL;

int readPacket(void *opaque, uint8_t *buf, int buf_size)
{
	int static totalSize = 0;
	int readSize = 0;

	if (header != NULL && ! feof(header)){
		readSize = fread(buf, 1, buf_size, header);
		totalSize += readSize;
		printf("readPacket[readSize:%d, totalSize:%d]\n", readSize, totalSize);
		return readSize;
	}

// 	 if (file_audio != NULL && !feof(file_audio)){
// 		 readSize = fread(buf, 1, buf_size, file_audio);
// 	 }
	 totalSize += readSize;
	 	printf("readPacket[readSize:%d, totalSize:%d]\n", readSize, totalSize);
	return readSize;
}
int write_packet(void *opaque, uint8_t *buf, int buf_size){
	if (out_file != NULL){
		int sz = fwrite(buf, 1, buf_size, out_file);
		return sz;
	}
	return buf_size;
}


std::string errorMsg(int ret)
{
	char err[1024] = { 0 };
	av_strerror(ret, err, sizeof(err));

	return std::string(err);
}

void ffmpegOutputLog(void *ptr, int level, const char *fmt, va_list vl){
	char szBuffer[1024] = { 0 };
	sprintf_s(szBuffer, fmt, vl);
	//printf(fmt, vl);
	if (logFile != NULL){
		fwrite(szBuffer, 1, strlen(szBuffer), logFile);
	}
}


int TestDolby(){
	int ret = 0;

	logFile = fopen("log_info.log", "w");

	AVFormatContext *i_fmt_ctx = NULL; 
	AVFormatContext *o_fmt_ctx = NULL;
	AVStream *i_video_stream = NULL;
	//header = fopen("dolby_src/0.mp4", "rb");
	std::string input_audio = "hdr_410b1ef36ffce6f38e9697cb489018b1.265ts";
	//std::string  input_audio = "./dolby_src/591bd952b6f0d7460d36d9a5c8c91f38 (1).ts";
	//std::string input_audio = "dolby_src/170334dbe6a7ae16d05ab52d3b61bc76.amp4";

	header = fopen(input_audio.c_str(), "rb");
	//file_audio = fopen(input_audio.c_str(), "rb");

	std::string out_file_name = input_audio.substr(0, input_audio.find("."));
	out_file_name.append("_.ts");

	out_file = fopen(out_file_name.c_str(), "wb");

	int bufLength = 32*1024;
	uint8_t *avioBuf = (uint8_t*)av_malloc(bufLength);
	AVIOContext *avioCtx = avio_alloc_context(avioBuf, bufLength, 0, NULL, readPacket, NULL, NULL);
	//AVIOContext *avioCtx = avio_alloc_context(NULL, 0, 0, NULL, readPacket, NULL, NULL);
	if (avioCtx == NULL)
		return 0;

	i_fmt_ctx = avformat_alloc_context();
	if (i_fmt_ctx == NULL)
		return 0;

	i_fmt_ctx->pb = avioCtx;

	if ( (ret = avformat_open_input(&i_fmt_ctx, /*input_audio.c_str()*/NULL, NULL, NULL))!=0) 
	{ 
		fprintf(stderr, "could not open input file\n"); 
		errorMsg(ret);
		return -1; 
	}

	if (avformat_find_stream_info(i_fmt_ctx, NULL)<0) 
	{ 
		fprintf(stderr, "could not find stream info\n"); 
		return -1; 
	}

	AVDictionaryEntry *tag = NULL;

	tag = av_dict_get(i_fmt_ctx->metadata, "datasize", NULL, AV_DICT_MATCH_CASE);

	while ((tag = av_dict_get(i_fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
		printf("%s=%s\n", tag->key, tag->value);

	//avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, filename);
	avformat_alloc_output_context2(&o_fmt_ctx, NULL, "mpegts", NULL);

	int bufSize = 32*1024;
	uint8_t *buffer = (uint8_t*)av_malloc(bufSize + 1);
	AVIOContext *out_avio_ctx = avio_alloc_context(buffer, bufSize, 1, NULL, NULL, write_packet, NULL);
	o_fmt_ctx->pb = out_avio_ctx;
	o_fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
	o_fmt_ctx->flags |= AVFMT_FLAG_AUTO_BSF;


	for (unsigned i=0; i<i_fmt_ctx->nb_streams; i++) 
	{ 
		AVStream *streaqm = i_fmt_ctx->streams[i];
		if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
		{ 
			i_video_stream = i_fmt_ctx->streams[i]; 
			AVStream *v_steam = avformat_new_stream(o_fmt_ctx, NULL);
			avcodec_parameters_copy(v_steam->codecpar, i_video_stream->codecpar);
			continue;
		} else if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			AVStream *a_steam = avformat_new_stream(o_fmt_ctx, NULL);
			avcodec_parameters_copy(a_steam->codecpar, i_fmt_ctx->streams[i]->codecpar);
		}
	} 
	if (i_video_stream == NULL) 
	{ 
		fprintf(stderr, "didn't find any video stream\n"); 
		return -1; 
	}

	//avio_open(&o_fmt_ctx->pb, filename, AVIO_FLAG_WRITE);

	// "resend_headers", "muxrate", "pcr_period", "pat_period"
	//ret = av_opt_set(o_fmt_ctx->priv_data, "resend_headers", "1", 1);
	//ret = av_opt_set(o_fmt_ctx->priv_data, "pat_period", "10", 1);
	//ret = av_opt_set(o_fmt_ctx->priv_data, "mpegts_copyts", "1", 1);
	o_fmt_ctx->max_delay = 0.7 * 1000000;
	avformat_write_header(o_fmt_ctx, NULL);

	int last_pts = 0; 
	int last_dts = 0;

	int64_t pts, dts; 
	while (1) 
	{ 
		AVPacket i_pkt; 
		av_init_packet(&i_pkt); 
		i_pkt.size = 0; 
		i_pkt.data = NULL; 
		int ret = av_read_frame(i_fmt_ctx, &i_pkt);
		if (ret < 0){ 
			errorMsg(ret);
			break; 
		}

		int is_key = i_pkt.flags & AV_PKT_FLAG_KEY;
		//int offset = 126000;
		int offset = 0;
		i_pkt.pts = av_rescale_q_rnd(i_pkt.pts, i_fmt_ctx->streams[i_pkt.stream_index]->time_base, o_fmt_ctx->streams[i_pkt.stream_index]->time_base, (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX)) + offset;
		i_pkt.dts = av_rescale_q_rnd(i_pkt.dts, i_fmt_ctx->streams[i_pkt.stream_index]->time_base, o_fmt_ctx->streams[i_pkt.stream_index]->time_base, (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX)) + offset;
		i_pkt.duration = av_rescale_q(i_pkt.duration, i_fmt_ctx->streams[i_pkt.stream_index]->time_base, o_fmt_ctx->streams[i_pkt.stream_index]->time_base);

		//printf("%lld %lld\n", i_pkt.pts, i_pkt.dts); 
		static int num = 1; 
		printf("frame %d\n", num++); 
		av_interleaved_write_frame(o_fmt_ctx, &i_pkt); 

	} 

	avformat_close_input(&i_fmt_ctx);

	av_write_trailer(o_fmt_ctx);

	//avcodec_close(o_fmt_ctx->streams[0]->codec); 
	//av_freep(&o_fmt_ctx->streams[0]->codec); 
	//av_freep(&o_fmt_ctx->streams[0]);

	//avio_close(o_fmt_ctx->pb); 
	//av_free(o_fmt_ctx);

	if (logFile != NULL){
		fclose(logFile);
	}
	avformat_free_context(o_fmt_ctx);
}

int _tmain(int argc, _TCHAR* argv[])
{
// 	av_register_all();
// 	av_log_set_callback(ffmpegOutputLog);
// 	av_log_set_level(AV_LOG_INFO);

	//TestDolby();

// 	Mpegts ts;
// 	ts.test();

	DolbyAudio *dolbyAudio = new DolbyAudio("./fmp4/init.mp4", "./fmp4/cmaf25397.m4s");
	dolbyAudio->init();


	OutputSource *outputSource = new OutputSource(dolbyAudio->getContext());
	outputSource->init();
	while(true){
		AVPacket *pkt = dolbyAudio->getPacket();
		if (pkt == NULL){
			break;
		}

		outputSource->writePacket(pkt);
	}

	delete dolbyAudio;
	delete outputSource;

	return 0;
}

