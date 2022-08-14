#include "encoder.h"

X264Encoder::X264Encoder(char *outputPath, int inWidth, int inHeight, int fps) {
    if (inWidth <= 0 || inHeight <= 0 || fps <= 0) {
        av_log(nullptr, AV_LOG_INFO, "Invalid parameter for frame w/h and fps. Force to use default value.\n");
        width = 1280;
        height = 720;
        fps = 30;
    }
    width = inWidth;
    height = inHeight;

    sws = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR,
                         nullptr, nullptr, nullptr);
    if (!sws) {
        av_log(nullptr, AV_LOG_INFO, "Cannot create sws context.\n");
    }

    // open writing stream of output file
    /*outputStream.open(outputPath, ios::out | ios::trunc);
    if (outputStream.fail()) {
        av_log(nullptr, AV_LOG_ERROR, "Failed to open output file!\n");
    }*/
    outputFile = fopen(outputPath, "wb");
    if (!outputFile) {
        av_log(nullptr, AV_LOG_ERROR, "Failed to open output file!\n");
    }

    // allocate qp array
    //mbQp = new float[width*height];
    mbQp = nullptr;

    // create and set x264 params
    // inFrame = nullptr;
    // outFrame = nullptr;
    nals = nullptr;
    nalCount = 0;
    if (!x264Param) delete x264Param;
    x264Param = new x264_param_t;
    int ret = x264_param_default_preset(x264Param, "fast", "zerolatency");
    if(ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Failed to set preset parameter!\n");
    }
    x264Param->i_threads = 1;
    x264Param->i_width = width;
    x264Param->i_height = height;
    x264Param->i_fps_num = fps;
    x264Param->i_fps_den = 1;

    // set input frame buffer
    x264_picture_alloc(&inFrame, X264_CSP_I420, width, height);
    inFrame.param = x264Param;

    // set QP in macroBlock level
    x264Param->rc.i_aq_mode = X264_AQ_VARIANCE;
    // inFrame.prop.quant_offsets = mbQp;

    encoder = x264_encoder_open(x264Param);
    if(!encoder) {
        av_log(nullptr, AV_LOG_ERROR, "Failed to open x264 encoder!\n");
    }

    // Write headers to file
    int header_size = x264_encoder_headers(encoder, &nals, &nalCount);
    if(header_size < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Error when calling x264_encoder_headers()!\n");
    }
    // outputStream << nals[0].p_payload;
    if (!fwrite(nals[0].p_payload, sizeof(uint8_t), header_size, outputFile)) {
        av_log(nullptr, AV_LOG_ERROR, "Failed to write header!\n");
    }
}

X264Encoder::~X264Encoder() {
    if (encoder) {
        x264_picture_clean(&inFrame);
        x264_encoder_close(encoder);
        encoder = nullptr;
    }
    /*if (outputStream.is_open()) {
        outputStream.close();
    }*/
    if (outputFile) {
        fclose(outputFile);
        outputFile = nullptr;
    }
    if (sws) {
        sws_freeContext(sws);
        sws = nullptr;
    }
    delete mbQp;
}

// return whether encoding of current frame is successful
bool X264Encoder::encode(uint8_t **data, const int *linesize) {
    int scaleHeight = sws_scale(sws, data, linesize, 0, height, inFrame.img.plane, inFrame.img.i_stride);
    if(scaleHeight != height) {
        av_log(nullptr, AV_LOG_ERROR, "Failed to scale frame!\n");
        return false;
    }

    int frame_size = x264_encoder_encode(encoder, &nals, &nalCount, &inFrame, &outFrame);
    if(frame_size < 0){
        av_log(nullptr, AV_LOG_ERROR, "Failed to encode frame!\n");
        return false;
    }

    if(!fwrite(nals[0].p_payload, sizeof(uint8_t), frame_size, outputFile)){
        av_log(nullptr, AV_LOG_ERROR, "Failed to write NAL!\n");
        return false;
    }

    return true;
}

bool X264Encoder::setBlockQp(const vector<Regions>& rois, float baseQp) {
    // allocate mbqp: array of width*height in macroBlock level
    int xblocks = width / 16 + (width % 16 ? 1 : 0);
    int yblocks = height / 16 + (height % 16 ? 1 : 0);

    if (!mbQp) delete mbQp;
    mbQp = new float[xblocks*yblocks];
    inFrame.prop.quant_offsets = mbQp;

    for (int i = 0; i < xblocks * yblocks; ++i) {
        mbQp[i] = baseQp;
    }

    unsigned roiCount = rois.size();
    av_log(nullptr, AV_LOG_INFO, "Totally setting QP for %d regions of interest.\n", roiCount);
    for (int i = 0; i < roiCount; ++i) {
        av_log(nullptr, AV_LOG_INFO, "Setting QP %f for %d region of interest.\n", rois[i].qp, i+1);
        for (int y = rois[i].y1; y <= rois[i].y2 && y < yblocks ; ++y) {
            for (int x = rois[i].x1; x < rois[i].x2 && x < xblocks; ++x) {
                mbQp[x + y * xblocks] = rois[i].qp;
            }
        }
    }
    av_log(nullptr, AV_LOG_INFO, "ROI QP setting is done.\n");

    return true;
}
