// CustomizeFfmpegIo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include "Mpegts.h"
#include "DolbyAudio.h"
#include "OutputSource.h"

int _tmain(int argc, _TCHAR* argv[])
{
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

