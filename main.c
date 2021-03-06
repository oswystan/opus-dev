/*
 **************************************************************************************
 *       Filename:  main.c
 *    Description:   source file
 *
 *        Version:  1.0
 *        Created:  2017-09-14 21:51:55
 *
 *       Revision:  initial draft;
 **************************************************************************************
 */

#include <sys/mman.h>
#include <errno.h>
#include <arpa/inet.h>
#include <inttypes.h>
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

int main(int argc, const char *argv[]) {
    logi("version: %s", opus_get_version_string());
    if (argc != 2) {
        log("usage: %s <file>\n", argv[0]);
        return -EINVAL;
    }

    int err = 0;
    size_t file_size = 0;
    OpusDecoder* opus = NULL;
    int framesize = 0;
    short out[1024];
    int fec_samples = 0;

    uchar* ptr = map_file(argv[1], &file_size);
    uchar* end = ptr + file_size;
    if(ptr == (uchar*)MAP_FAILED) {
        return -ENOMEM;
    }
    FILE* fp = fopen("dec.pcm", "wb");
    if (!fp) {
        loge("fail to open out file");
        err = errno;
        goto exit;
    }

    opus = opus_decoder_create(16000, 1, &err);
    if(!opus) {
        loge("fail to create opus decoder: %d", err);
        goto exit;;
    }

    /*
     * formats:
     * int(size)|opus data|int(size)|opus data|....
     */
    while (ptr < end) {
        fec_samples = 0;
        /*framesize = ntohl(*(int*)(ptr));*/
        framesize = (*(int*)(ptr));
        ptr += 4;
        err = opus_decode(opus, ptr, framesize, out, sizeof(out)/sizeof(out[0]), 0);
        if (err > 0) {
            fwrite(out, err, sizeof(out[0]), fp);
        } else {
            fec_samples = opus_packet_get_samples_per_frame(ptr, 16000);
            err = opus_decode(opus, ptr, framesize, out, fec_samples, 1);
            if (err > 0) {
                fwrite(out, err, sizeof(out[0]), fp);
            } else {
                err = opus_decode(opus, ptr, framesize, out, sizeof(out)/sizeof(out[0]), 0);
            }
        }
        ptr += framesize;
        logd("out size: %d fec: %d framesize=%d", err, fec_samples, framesize);
    }
    logi("success");

exit:
    if (opus) opus_decoder_destroy(opus);
    if (ptr) munmap(ptr, file_size);
    if (fp) fclose(fp);
    return err;
}

/********************************** END **********************************************/
