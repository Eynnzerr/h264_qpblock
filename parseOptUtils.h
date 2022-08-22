#ifndef H264_QPBLOCK_PARSEOPTUTILS_H
#define H264_QPBLOCK_PARSEOPTUTILS_H

#include <string>
#include <vector>

using namespace std;

class Regions {
public:
    Regions(int x1, int y1, int x2, int y2, float qp);
    // coordinate unit: MacroBlock(pixel / 16)
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    float qp = 0;
};

class Arguments {
public:
    bool parseArguments(int argc, char **argv);
    char *inputPath;
    char *outputPath;
    float base_qp;
    int fps;
    vector<Regions> rois;
};

#endif
