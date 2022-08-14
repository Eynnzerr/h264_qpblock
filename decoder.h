#ifndef H264_QPBLOCK_DECODER_H
#define H264_QPBLOCK_DECODER_H

#include "encoder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

class H264Decoder {
public:
    explicit H264Decoder(char *inputPath);
    ~H264Decoder();

    AVFormatContext *formatContext;
    int info;
    AVStream *stream;
    const AVCodec *codec;
    AVCodecContext *videoCodecContext;
    AVPacket *packet;
    AVFrame *frame;

    int width;
    int height;

    // callback which is to be called each time a frame is decoded by decoder
    typedef void (*onFrameDecoded)(AVFrame* avFrame, X264Encoder encoder, unsigned int *frameCount);
    void decodeFrames(onFrameDecoded callback) const;
};

#endif
