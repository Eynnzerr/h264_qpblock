#include "parseOptUtils.h"
#include <sstream>

extern "C" {
#include <libavutil/log.h>
}

Regions::Regions(int x1, int y1, int x2, int y2, float qp) {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    this->qp = qp;
}

bool Arguments::parseArguments(int argc, char **argv) {
    // av_log(nullptr, AV_LOG_INFO, "Argument count(except executable path): %d\n", argc);
    int regionCount = 0;

    // set input and output path
    this->inputPath = argv[1];
    av_log(nullptr, AV_LOG_INFO, "Input file path: %s\n", inputPath);
    this->outputPath = argv[2];
    av_log(nullptr, AV_LOG_INFO, "Output file path: %s\n", outputPath);

    this->base_qp = 0;

    // parse base qp option and region qp options
    for (int i = 3; i < argc; ++i) {
        string param = argv[i];
        // av_log(nullptr, AV_LOG_INFO, "%s\n", param.c_str());

        // find base qp
        if (param == "-baseqp") {
            this->base_qp = stof(argv[++i]);
            continue;
        }

        // find target region in format (x1, y1, x2, y2):qp-value
        size_t pos = param.find(':');
        string regionCoords = param.substr(0, pos);
        float region_qp = stof(param.substr(pos + 1));

        istringstream iss(regionCoords);
        string token;

        getline(iss, token, ',');
        int x1 = stoi(token);
        getline(iss, token, ',');
        int y1 = stoi(token);
        getline(iss, token, ',');
        int x2 = stoi(token);
        getline(iss, token, ',');
        int y2 = stoi(token);

        Regions region = Regions(x1 / 16, y1 / 16, x2 / 16, y2 / 16, region_qp);
        this->rois.push_back(region);

        av_log(nullptr, AV_LOG_INFO, "Find %d region: top_left:(%d, %d), bottom_right:(%d, %d), QP:%f\n",
               ++regionCount, x1, y1, x2, y2, region_qp);
    }

    av_log(nullptr, AV_LOG_INFO, "Base QP: %f\n", this->base_qp);

    return true;
}
