/*
 **************************************************************************************
 *       Filename:  enc.c
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2018-04-03 18:29:26
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#include <sys/mman.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>
#include <opus/opus.h>

#define LOG_TAG "main"
#include "log.h"

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;

uchar* map_file(const char* fn, size_t* size) {
    FILE* fp = fopen(fn, "r");
    if(!fp) {
        loge("fail to open file: %s", fn);
        return (uchar*)MAP_FAILED;
    }

    fseek(fp, 0L, SEEK_END);
    long pos = ftell(fp);
    uchar* ptr = (uchar*)mmap(NULL, pos, PROT_READ|PROT_WRITE, MAP_PRIVATE, fileno(fp), 0);
    *size = pos;

    fclose(fp);
    return ptr;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        log("usage: %s <pcm_file>\n", argv[0]);
        return -EINVAL;
    }
    OpusEncoder* encoder = NULL;
    int error = 0;
    size_t filesize = 0;
    int ret = 0;
    int16_t* input = NULL;
    int sample_rate = 16000;
    int frame_duration_ms = 10;

    encoder = opus_encoder_create(sample_rate, 1, OPUS_APPLICATION_VOIP, &error);
    if (!encoder) {
        loge("fail to create encoder: %d", error);
        return error;
    }

    uchar* ptr = map_file(argv[1], &filesize);
    if (ptr == MAP_FAILED) {
        loge("fail to map file");
        return -1;
    }
    input = (int16_t*)ptr;
    uchar* out = (uchar*)malloc(filesize);
    FILE* fp = fopen("enc.opus", "w");
    if (!fp) {
        loge("fail to open file out.opus");
        return -1;
    }

    int frame_size = sample_rate/1000*frame_duration_ms;

    while((uchar*)input < ptr + filesize) {
        ret = opus_encode(encoder, input, frame_size, out, filesize);
        if (ret <= 0) {
            loge("fail to encode");
            break;
        }
        logd("enc: %d", ret);
        fwrite(&ret, 1, sizeof(int), fp);
        fwrite(out, 1, ret, fp);
        input += frame_size;
    }
    fclose(fp);

    munmap(ptr, filesize);
    opus_encoder_destroy(encoder);
    return 0;
}


/********************************** END **********************************************/

