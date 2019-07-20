#include "stdafx.h"
#include "decodeVideoToPicture.h"
#include <Windows.h>

FILE *gInputFile = NULL;

int readPacketCallBack(void *opaque, uint8_t *buf, int buf_size)
{
    int static totalSize = 0;
    int readSize = 0;

    if (gInputFile != NULL && ! feof(gInputFile)){
        readSize = fread(buf, 1, buf_size, gInputFile);
        totalSize += readSize;
        //printf("readPacket[readSize:%d, totalSize:%d]\n", readSize, totalSize);
        return readSize;
    }

    return readSize;
}

DecodeVideoToPicture::DecodeVideoToPicture(const std::string &file) : mInputFile(file), mAvFmtCtx(NULL), mVideoCodec(NULL), mVideoCodecCtx(NULL), mFrameCount(0) {
    av_register_all();
}

DecodeVideoToPicture::~DecodeVideoToPicture() {

}

bool DecodeVideoToPicture::init() {
    gInputFile = fopen(mInputFile.c_str(), "rb");

    int bufferSize = 1024;
    uint8_t *buffer = (uint8_t*)av_mallocz(bufferSize);

    AVIOContext *ioContext = avio_alloc_context(buffer, bufferSize, 0, NULL, readPacketCallBack, NULL, NULL);
    mAvFmtCtx = avformat_alloc_context();
    mAvFmtCtx->pb = ioContext;
    //mAvFmtCtx->flags |= AVFMT_FLAG_GENPTS;
    //mAvFmtCtx->debug |= FF_FDEBUG_TS;

    int ret = avformat_open_input(&mAvFmtCtx, NULL, NULL, NULL);
    if (ret <0){
        return false;
    }

    ret = avformat_find_stream_info(mAvFmtCtx, NULL);
    if (ret < 0){
        return false;
    }

    for (int i = 0 ; i < mAvFmtCtx->nb_streams; i++) {
        AVStream *cur = mAvFmtCtx->streams[i];
        AVCodecParameters *par = cur->codecpar;

        if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
            mVideoCodec = avcodec_find_decoder(par->codec_id);
        }

        printf("media type:%d \n", par->codec_type);
    }
 
    mVideoCodecCtx = avcodec_alloc_context3(mVideoCodec);
    ret = avcodec_open2(mVideoCodecCtx, mVideoCodec, NULL);
    mYUVFile = fopen("out.yuv", "wb");

    return ret == 0;
}

AVPacket *DecodeVideoToPicture::getPacket(int type) {
    AVPacket *pkt = NULL;
    AVFrame *frame = av_frame_alloc();

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


        if (pkt->stream_index == 0){
            av_log(NULL, AV_LOG_INFO, "av_read_frame end, pts:%lld, dts:%lld \n", pkt->pts, pkt->dts);
        }

        if (pkt->stream_index == type || type == 2){
            break;
        } else {
            av_packet_free(&pkt);
        }
    } while (true);

    return pkt;
}

int DecodeVideoToPicture::decodeVideoFrame(AVPacket *pkt) {
     if (pkt == NULL ) {
        return 0;
    }


    if (pkt->stream_index == 0) {
        int ret = avcodec_send_packet(mVideoCodecCtx, pkt);
        AVFrame *frame = av_frame_alloc();

        while(ret >= 0) {
            
            ret = avcodec_receive_frame(mVideoCodecCtx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                fprintf(stderr, "Error during decoding\n");
                break;
            } else {
                // 
                mFrameCount++;
                char* buf = new char[mVideoCodecCtx->height * mVideoCodecCtx->width * 3 / 2];
                memset(buf, 0, mVideoCodecCtx->height * mVideoCodecCtx->width * 3 / 2);
                int height = mVideoCodecCtx->height;
                int width = mVideoCodecCtx->width;
                printf("decode video ok\n");
                int a = 0, i;
                for (i = 0; i<height; i++)
                {
                    memcpy(buf + a, frame->data[0] + i * frame->linesize[0], width);
                    a += width;
                }
                for (i = 0; i<height / 2; i++)
                {
                    memcpy(buf + a, frame->data[1] + i * frame->linesize[1], width / 2);
                    a += width / 2;
                }
                for (i = 0; i<height / 2; i++)
                {
                    memcpy(buf + a, frame->data[2] + i * frame->linesize[2], width / 2);
                    a += width / 2;
                }
                fwrite(buf, 1, mVideoCodecCtx->height * mVideoCodecCtx->width * 3 / 2, mYUVFile);
                delete buf;
                buf = NULL;

                mYUVSize += mVideoCodecCtx->height * mVideoCodecCtx->width * 3 / 2;

                /*SwsContext* swsContext = sws_getContext(frame->width, frame->height, mVideoCodecCtx->pix_fmt,frame->width, frame->height, AV_PIX_FMT_BGR24,
                    NULL, NULL, NULL, NULL);

                AVFrame  * pFrameRGB = av_frame_alloc();
                int num_bytes = avpicture_get_size(AV_PIX_FMT_BGR24, frame->width, frame->height);
                uint8_t *out_buffer = (uint8_t *) av_malloc(num_bytes * sizeof(uint8_t));
                avpicture_fill((AVPicture *) pFrameRGB, out_buffer, AV_PIX_FMT_RGB24, frame->width, frame->height);
                sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, pFrameRGB->data, pFrameRGB->linesize);
                saveToBMP(pFrameRGB, mVideoCodecCtx->width, mVideoCodecCtx->height, mFrameCount, 24);
                if (out_buffer != NULL) {
                    av_free(out_buffer);
                }
                av_frame_free(&pFrameRGB);
                */

                if (mFrameCount < 5) {
                    saveToJPEG(frame, mVideoCodecCtx->width, mVideoCodecCtx->height, mFrameCount);
                }

               
            }
        }

         av_frame_free(&frame);
    }

    return 0;
    
}

int DecodeVideoToPicture::saveToJPEG(AVFrame *pFrame, int width, int height, int iIndex) {
    char out_file[1024] = {0};
    sprintf(out_file, "./pic/temp%d.jpg",  iIndex);

    // 分配AVFormatContext对象
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
    if (pFormatCtx->oformat == NULL) {
       goto error;
    }

    if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Couldn't open output file.");
        goto error;
    }

    // 构建一个新stream
    AVStream *pAVStream = avformat_new_stream(pFormatCtx, NULL);
    if (pAVStream == NULL) {
        goto error;
    }

    AVCodecContext *pCodecCtx = pAVStream->codec;
    pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    av_dump_format(pFormatCtx, 0, out_file, 1);
    AVCodec *pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        goto error;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        goto error;
    }

    avformat_write_header(pFormatCtx, NULL);
    AVPacket pkt;
    av_new_packet(&pkt, pCodecCtx->width * pCodecCtx->height * 3);

    int got_picture = 0;
    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
    if (ret < 0) {
        goto error;
    }

    if (got_picture == 1) {
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    av_free_packet(&pkt);
    av_write_trailer(pFormatCtx);
    

error:

    if (pAVStream != NULL) {
        avcodec_close(pAVStream->codec);
    }

    avio_close(pFormatCtx->pb);

    if(pFormatCtx != NULL){
        avformat_free_context(pFormatCtx);
    }

   

    return 0;
}

int DecodeVideoToPicture::saveToBMP(AVFrame *pFrameRGB, int width, int height, int index, int bpp)
{
    char buf[5] = {0};
    BITMAPFILEHEADER bmpheader;
    BITMAPINFOHEADER bmpinfo;
    FILE *fp;

    char *filename = new char[255];
    sprintf_s(filename, 255, "./pic/%d.bmp", index);
    if( (fp = fopen(filename,"wb+")) == NULL ) {
        printf ("open file failed!\n");
        return -1;
    }

    bmpheader.bfType = 0x4d42;
    bmpheader.bfReserved1 = 0;
    bmpheader.bfReserved2 = 0;
    bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp/8;

    bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
    bmpinfo.biWidth = width;
    bmpinfo.biHeight = height;
    bmpinfo.biPlanes = 1;
    bmpinfo.biBitCount = bpp;
    bmpinfo.biCompression = BI_RGB;
    bmpinfo.biSizeImage = (width*bpp+31)/32*4*height;
    bmpinfo.biXPelsPerMeter = 100;
    bmpinfo.biYPelsPerMeter = 100;
    bmpinfo.biClrUsed = 0;
    bmpinfo.biClrImportant = 0;

    fwrite(&bmpheader, sizeof(bmpheader), 1, fp);
    fwrite(&bmpinfo, sizeof(bmpinfo), 1, fp);
    fwrite(pFrameRGB->data[0], width*height*bpp/8, 1, fp);

    fclose(fp);


    return 0;
}
