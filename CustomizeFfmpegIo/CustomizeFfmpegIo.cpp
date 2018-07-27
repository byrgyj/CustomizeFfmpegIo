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
#include "Eac3Parser.h"
#include "DynamicFunction.h"
#include <set>

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

//     DynamicFunction df;
//     if (df.init()) {
//        AVFormatContext *ctx = NULL;
//         int ret = df.wrapper_avformat_open_input(&ctx, "./3f39abf673c468ef68fd00887a13e02f.f4v", NULL, NULL);
//         ret = df.wrapper_avformat_find_stream_info(ctx, NULL);
//         return 0;
//     }


    
	int debugIndex = 4;
	if (debugIndex == 0){
		Mpegts ts;
		ts.testPtsDts();
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
		std::string file = "out_1.mp4";
		FFmpegReadFile ff(file);
		if (!ff.init()){
			return false;
		}

        AVCodecParameters *param = ff.getVideoParam();

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
		std::string file = "./gmz3.ts";
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
        int64_t  lastDts = 0;
        int videoPacketCount = 0;

        std::set<int64_t> ptsSet;
        std::set<int64_t> dtsSet;
		do 
		{
			pkt = ff.getPacket(M_ALL);
			if(pkt != NULL){
				if (pkt->stream_index == 0){
					uint8_t * data = pkt->data;
					if (pkt->flags & AV_PKT_FLAG_KEY){
						//printf("key");
					} else {
						//printf("no key");
					}

                    //av_log(NULL, AV_LOG_DEBUG, "VIDEO INFO:dts:%lld, pts:%lld \n", pkt->dts, pkt->pts);

                    videoPacketCount++;
                    //printf("pts:%lld, dts:%lld\n", pkt->pts, pkt->dts);
                    if (pkt->pts == AV_NOPTS_VALUE || pkt->dts == AV_NOPTS_VALUE) {
                        printf("invalidate pts, key frame:%d", pkt->flags & AV_PKT_FLAG_KEY);
                    } else {
                        lastDts = pkt->dts;

                        if (pkt->pts < pkt->dts) {
                            printf("invalidate");
                        }

                        if (ptsSet.find(pkt->pts) != ptsSet.end()){
                            printf("invalidate pts");
                        } else {
                            ptsSet.insert(pkt->pts);
                        }
                        
                        if (dtsSet.find(pkt->dts) != dtsSet.end()) {
                            printf("invalidate dts");
                        } else {
                            dtsSet.insert(pkt->dts);
                        }
                        
                    }

				}
// 
// 				if (out->writePacket(pkt) <0){
// 					printf("write failed");
// 				}
				av_packet_free(&pkt);
			} else {
				break;
			}
		} while (true);


        std::set<int64_t>::iterator ptsIt = ptsSet.begin();
        std::set<int64_t>::iterator dtsIt = dtsSet.begin();

        int64_t lastPts = 0;
        while(ptsIt != ptsSet.end()){
            av_log(NULL, AV_LOG_DEBUG, "VIDEO_INFO_PTS: pts:%lld, distance:%lld \n", *ptsIt, *ptsIt - lastPts);
            lastPts = *ptsIt;
            ptsIt++;
        }

        lastDts = 0;
        while(dtsIt != dtsSet.end()){
            av_log(NULL, AV_LOG_DEBUG, "VIDEO_INFO_DTS: pts:%lld, distance:%lld \n", *dtsIt, *dtsIt - lastDts);
            lastDts = *dtsIt;
            dtsIt++;
        }


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
	} else if (debugIndex == 6){
		Eac3Parser parser("./out_1.mp4");
		if (parser.init()){
			parser.parse();
		}
	} else if (debugIndex == 7) {
        Eac3Context  ec;
        AVPacket *pkt = NULL;
        do 
        {
            pkt = ec.readPacket();
            if (pkt == NULL) {
                break;
            }

            printf("pkt->size:%d pkt->pts:%lld\n", pkt->size, pkt->pts);
            av_packet_unref(pkt);
            av_packet_free(&pkt);
        } while (true);
    }

	if (logFile != NULL){
		fclose(logFile);
	}
	return 0;
}

