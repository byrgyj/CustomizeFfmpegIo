// CustomizeFfmpegIo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include "Mpegts.h"
#include "DolbyAudio.h"
#include "OutputSource.h"
#include "FileFragment.h"

FILE *logFile = NULL;

void logCallback(void*, int index, const char* fmt, va_list vl){
	vfprintf(logFile, fmt,vl);
	fflush(logFile);
}


int _tmain(int argc, _TCHAR* argv[])
{
	logFile = fopen("debug.log", "w");
	av_log_set_level(AV_LOG_DEBUG);
	av_log_set_callback(logCallback);

// 	Mpegts ts;
// 	ts.test();

	FileFragment fo("fmp4/20s.mp4");
	fo.segmentFile();

// 	DolbyAudio *dolbyAudio = new DolbyAudio("./fmp4/header.mp4", "./fmp4/data3.m4s");
// 	if (!dolbyAudio->init()){
// 		goto end;
// 	}
// 
// 
// 	OutputSource *outputSource = new OutputSource(dolbyAudio->getContext());
// 	if (!outputSource->init()){
// 		goto end;
// 	}
// 
// 	while(true){
// 		AVPacket *pkt = dolbyAudio->getPacket();
// 		if (pkt == NULL){
// 			break;
// 		}
// 
// 		outputSource->writePacket(pkt);
// 	}
// 
// end:
// 	if (dolbyAudio != NULL){
// 		delete dolbyAudio;
// 	}
// 	
// 	if (outputSource != NULL) {
// 		delete outputSource;
// 	}
	
	if (logFile != NULL){
		fclose(logFile);
	}

	return 0;
}

