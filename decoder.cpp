#include "decoder.h"

H264Decoder::H264Decoder(char *inputPath) {
    formatContext = avformat_alloc_context();
    avformat_open_input(&formatContext, inputPath, nullptr, nullptr);

    info = avformat_find_stream_info(formatContext, nullptr);
    if (info == AVERROR_EOF)
        av_log(nullptr, AV_LOG_ERROR, "Reach file end.\n");

    // find video stream.
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *avStream = formatContext->streams[i];
        if (AVMEDIA_TYPE_VIDEO == avStream->codecpar->codec_type) {
            stream = avStream;
            av_log(nullptr, AV_LOG_INFO, "Input video: totally %ld frames.\n", stream->nb_frames);

            // log basic information
            width = stream->codecpar->width;
            height = stream->codecpar->height;
            av_log(nullptr, AV_LOG_INFO, "Input video: resolution is %d x %d. bitrate is %ld\n",
                   width, height, stream->codecpar->bit_rate);

            // find decoder
            codec = avcodec_find_decoder(stream->codecpar->codec_id);
            if (!codec)
                av_log(nullptr, AV_LOG_ERROR, "No video codec found. Program terminated.\n");

            // open decoder
            videoCodecContext = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(videoCodecContext, stream->codecpar);
            if (avcodec_open2(videoCodecContext, nullptr, nullptr) < 0) {
                av_log(nullptr, AV_LOG_ERROR, "Failed to open codec. Program terminated.\n");
            }

            // check pixel format. For x264, it must be YUV-420p
            if (videoCodecContext->pix_fmt != AV_PIX_FMT_YUV420P) {
                av_log(nullptr, AV_LOG_ERROR, "Wrong pixel format. Should preset to be yuv420p.\n");
            }

            break;
        }
    }

    if (!stream) {
        av_log(nullptr, AV_LOG_ERROR, "No h264 video stream found. Please check input format.\n");
    }

    packet = av_packet_alloc();
    frame = av_frame_alloc();
}

H264Decoder::~H264Decoder() {
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_close(videoCodecContext);
    avcodec_free_context(&videoCodecContext);
    avformat_close_input(&formatContext);
    formatContext = nullptr;
}

// Deprecated
void H264Decoder::decodeFrames(H264Decoder::onFrameDecoded callback) const {
    int frameCount = 0;

    while(!av_read_frame(formatContext, packet)) {
        // Feed encoded packet to decoder
        if (avcodec_send_packet(videoCodecContext, packet) != 0) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to send packet into decoder.\n");
            break;
        }
        av_packet_unref(packet);

        // Get decoded frame from decoder
        int res = avcodec_receive_frame(videoCodecContext, frame);
        if (res == AVERROR(EAGAIN)) {
            av_log(nullptr, AV_LOG_INFO, "No data output.\n");
            continue;
        }
        else if (res == AVERROR_EOF) {
            av_log(nullptr, AV_LOG_INFO, "End of file.\n");
            continue;
        }
        else if (res < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Receive frame failed with code %d.\n", res);
            continue;
        }

        av_log(nullptr, AV_LOG_INFO, "Receive and re-encode frame %d with pts %ld.\n", ++frameCount,frame->pts);
        // callback(frame);

        av_frame_unref(frame);
    }
}
