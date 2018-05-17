// CustomizeFfmpegIo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include "Mpegts.h"
#include "DolbyAudio.h"
#include "OutputSource.h"
#include "FileFragment.h"
#include "FFmpegReadFile.h"
#include "FileMemoryCheck.h"

FILE *logFile = NULL;

void logCallback(void*, int index, const char* fmt, va_list vl){
	vfprintf(logFile, fmt,vl);
	fflush(logFile);
}

int _tmain(int argc, _TCHAR* argv[])
{
	logFile = fopen("debug.log", "w");
	av_log_set_level(AV_LOG_WARNING);
	av_log_set_callback(logCallback);

	int debugIndex = 5;
	if (debugIndex == 0){
		Mpegts ts;
		ts.test();
	} else if (debugIndex == 1){
		std::string srcFile;

#ifdef USE_VIDEO_1
		srcFile = "fmp4/20s.mp4";
#else
		srcFile = "fmp4/720p_combine.mp4";
#endif
		FileFragment fo(srcFile);
		fo.segmentFile();
	} else if (debugIndex == 2) {
		DolbyAudio *dolbyAudio = new DolbyAudio("./fmp4/header.mp4", "./fmp4/data3.m4s");
		if (!dolbyAudio->init()){
			goto end;
		}
		OutputSource *outputSource = new OutputSource(dolbyAudio->getContext());
		if (!outputSource->init()){
			goto end;
		}
		while(true){
			AVPacket *pkt = dolbyAudio->getPacket();
			if (pkt == NULL){
				break;
			}

			outputSource->writePacket(pkt);
		}
end:
		if (dolbyAudio != NULL){
			delete dolbyAudio;
		}
		if (outputSource != NULL) {
			delete outputSource;
		}
	} else if (debugIndex == 3){
		std::string file = "./4k/f96052b8c9f468eb2bc116ceb3809842 (17).ts";
		FFmpegReadFile ff(file);
		if (!ff.init()){
			return false;
		}

		OutputSource *out = new OutputSource(ff.getContext());
		if (!out->init()){
			delete out;
			return 0;
		}

		AVPacket *pkt = NULL;
		do 
		{
			pkt = ff.getPacket(M_ALL);
			if(pkt != NULL){
				if (out->writePacket(pkt) <0){
					printf("write failed");
				}
				av_packet_free(&pkt);
			} else {
				break;
			}
		} while (true);

		if (out != NULL){
			delete out;
		}
	} else if (debugIndex == 4){
		std::string file = "./ffd5.f4v";
		FFmpegReadFile ff(file);
		if (!ff.init()){
			return false;
		}

		OutputSource *out = new OutputSource(ff.getContext());
		if (!out->init()){
			delete out;
			return 0;
		}

		AVPacket *pkt = NULL;
		do 
		{
			pkt = ff.getPacket(M_ALL);
			if(pkt != NULL){
				if (pkt->stream_index == 0){
					uint8_t * data = pkt->data;
					if (pkt->flags & AV_PKT_FLAG_KEY){
						printf("key");
					} else {
						printf("no key");
					}
				}

				if (out->writePacket(pkt) <0){
					printf("write failed");
				}
				av_packet_free(&pkt);
			} else {
				break;
			}
		} while (true);

		if (out != NULL){
			delete out;
		}
	} else if (debugIndex == 5){
		std::string file = "./ts/a883eace36ce22f8e788b2ed3463b7c7 (2).ts";
		FFmpegReadFile ff(file);
		if (!ff.init()){
			return false;
		}

		std::string file2 = "./ts/a883eace36ce22f8e788b2ed3463b7c7 (3).ts";
		CustomizedFile ffCostimize(file2);
		if (!ffCostimize.init(ff.getContext())){
			return false;
		}

		OutputSource *out = new OutputSource(ffCostimize.getContext());
		if (!out->init()){
			delete out;
			return 0;
		}

		do 
		{
			AVPacket *pkt = ffCostimize.getPacket(M_ALL);
			if (pkt == NULL){
				break;
			}
			
			if (out->writePacket(pkt) <0){
				printf("write failed");
			}

			av_packet_free(&pkt);
		} while (true);
	}

	if (logFile != NULL){
		fclose(logFile);
	}
	return 0;
}

