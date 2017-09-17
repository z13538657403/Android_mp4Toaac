//
// Created by 张涛 on 17/9/10.
//

#include <stdio.h>
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

extern "C"
{
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libavutil/log.h"
#include "include/libswscale/swscale.h"
#include "include/libavutil/opt.h"
#include "include/libavutil/imgutils.h"
#include "include/libavutil/frame.h"
}

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG,"Native",__VA_ARGS__)
#define nullptr (void *)0

extern "C"
JNIEXPORT jint JNICALL Java_com_imooc_extractinfotest_MainActivity_mp4ToAac
        (JNIEnv *env, jobject obj, jstring srcPath, jstring audioPath)
{
    AVFormatContext *pFormatCtx;
    int i , audioindex;
    AVCodecContext  *pCodecCtx;
    AVPacket *packet;

    char *filepath = (char*) env->GetStringUTFChars(srcPath , (unsigned char*)nullptr);
    char *aacPath = (char*) env->GetStringUTFChars(audioPath , (unsigned char*)nullptr);
    FILE *f_aac = fopen(aacPath , "wb+");

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    if(avformat_open_input(&pFormatCtx , filepath , NULL , NULL) < 0)
    {
        LOG("Couldn't open input stream.\n");
        return -1;
    }

    if(avformat_find_stream_info(pFormatCtx , NULL) < 0)
    {
        LOG("Couldn't find stream information.\n");
        return -1;
    }

    audioindex = -1;
    for (i = 0 ; i < pFormatCtx->nb_streams ; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioindex = i;
            break;
        }
    }

    if(audioindex == -1)
    {
        LOG("Didn't find a video stream.\n");
        return -1;
    }

    pCodecCtx = avcodec_alloc_context3(NULL);
    if (pCodecCtx == NULL)
    {
        LOG("Could not allocate AVCodecContext\n");
        return -1;
    }

    int result = avcodec_parameters_to_context(pCodecCtx , pFormatCtx->streams[audioindex]->codecpar);
    packet=(AVPacket *)av_malloc(sizeof(AVPacket));

    av_dump_format(pFormatCtx , 0 , filepath , 0);
    while(av_read_frame(pFormatCtx , packet) >= 0)
    {
        if(packet->stream_index == audioindex)
        {
            char bits[7] = {0};
            int sample_index = 0 , channel = 0;
            char temp = 0;
            int length = 7 + packet->size;
            sample_index = (pCodecCtx->extradata[0] & 0x07) << 1;
            temp = (pCodecCtx->extradata[1]&0x80);
            switch(pCodecCtx->sample_rate)
            {
                case 44100:
                {
                    sample_index = 0x7;
                }break;
                default:
                {
                    sample_index = sample_index + (temp>>7);
                }break;
            }
            channel = ((pCodecCtx->extradata[1] - temp) & 0xff) >> 3;
            bits[0] = 0xff;
            bits[1] = 0xf1;
            bits[2] = 0x40 | (sample_index<<2) | (channel>>2);
            bits[3] = ((channel&0x3)<<6) | (length >>11);
            bits[4] = (length>>3) & 0xff;
            bits[5] = ((length<<5) & 0xff) | 0x1f;
            bits[6] = 0xfc;

            fwrite(bits,1,7,f_aac);
            fwrite(packet->data , 1 , packet->size , f_aac);
        }
        av_packet_unref(packet);
    }

    fclose(f_aac);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 1;
}
