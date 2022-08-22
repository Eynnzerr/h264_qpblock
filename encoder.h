#ifndef H264_QPBLOCK_ENCODER_H
#define H264_QPBLOCK_ENCODER_H

#include <fstream>
#include "parseOptUtils.h"

extern "C" {
#include <libavutil/log.h>
#include <libswscale/swscale.h>
#include <x264.h>
};

using namespace std;

class X264Encoder {
public:
    X264Encoder(char *outputPath, int inWidth, int inHeight, int fps);
    ~X264Encoder();

    bool setBlockQp(const vector<Regions>& rois, float baseQp);
    bool encode(uint8_t *data[], const int linesize[]);
    bool flush();

    SwsContext *sws;
    x264_t *encoder;
    x264_param_t *x264Param;
    x264_nal_t *nals;
    int nalCount;
    x264_picture_t inFrame;
    x264_picture_t outFrame;

    int width;
    int height;

    // ofstream outputStream;
    FILE *outputFile;
    float *mbQp;  // store qp value for each macroBlock
};

#endif
