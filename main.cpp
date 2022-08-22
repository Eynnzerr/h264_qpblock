#include "parseOptUtils.h"
#include "decoder.h"
#include "encoder.h"

int main(int argc, char **argv) {
    // suppress info log
    // av_log_set_level(AV_LOG_ERROR);

    Arguments arguments;
    arguments.parseArguments(argc, argv);

    H264Decoder decoder = H264Decoder(arguments.inputPath);
    X264Encoder encoder = X264Encoder(arguments.outputPath, decoder.width, decoder.height, 30);
    encoder.setBlockQp(arguments.rois, arguments.base_qp);

    // start to decode and re-encode H264 frames
    int frameCount = 0;

    while(!av_read_frame(decoder.formatContext, decoder.packet)) {
        if (decoder.packet->stream_index != decoder.stream->index) continue;
        // Feed encoded packet to decoder
        if (avcodec_send_packet(decoder.videoCodecContext, decoder.packet) != 0) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to send packet into decoder.\n");
            break;
        }
        av_packet_unref(decoder.packet);

        // Get decoded frame from decoder
        int res = avcodec_receive_frame(decoder.videoCodecContext, decoder.frame);
        if (res == AVERROR(EAGAIN)) {
            // av_log(nullptr, AV_LOG_INFO, "No data output.\n");
            continue;
        }
        else if (res == AVERROR_EOF) {
            // av_log(nullptr, AV_LOG_INFO, "End of file.\n");
            continue;
        }
        else if (res < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Receive frame failed with code %d.\n", res);
            continue;
        }

        av_log(nullptr, AV_LOG_INFO, "Receive and re-encode frame %d with type %d and pts %ld. ", ++frameCount, decoder.frame->pict_type, decoder.frame->pts);
        if (encoder.encode(decoder.frame->data, decoder.frame->linesize)) {
            av_log(nullptr, AV_LOG_INFO, "Success.\n");
        } else {
            av_log(nullptr, AV_LOG_INFO, "Failed.\n");
        }

        av_frame_unref(decoder.frame);
    }

    // Flush decoder to receive buffered frames(e.g. B frames)
    if (frameCount < decoder.stream->nb_frames) {
        avcodec_send_packet(decoder.videoCodecContext, nullptr);
        while (avcodec_receive_frame(decoder.videoCodecContext, decoder.frame) != AVERROR_EOF) {
            av_log(nullptr, AV_LOG_INFO, "Receive and re-encode frame %d with pts %ld. ", ++frameCount,decoder.frame->pts);
            if (encoder.encode(decoder.frame->data, decoder.frame->linesize)) {
                av_log(nullptr, AV_LOG_INFO, "Success.\n");
            } else {
                av_log(nullptr, AV_LOG_INFO, "Failed.\n");
            }

            av_frame_unref(decoder.frame);
        }
    }

    // Flush encoder to encode buffered frames(e.g. B frames)
    encoder.flush();

    return 0;
}
