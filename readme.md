# h264_qpblock

### What's this repository for?
This library allows you to encode regions of interest(ROIs) with multiple target QP value, instead of frame-level uniform QP adopted in traditional CQP H264.

H264 may achieve different qp for macroblocks by adaptive quantization, however sometimes you may not be satisfied with the decision made by encoder, then you can use this library to modify it.

### What's good point of this repository?
This library is inspired by [h264-roi](https://github.com/ChaoticEnigma/h264-roi), but with more advantages:

- Remove dependency on [libchaos](https://github.com/ChaoticEnigma/libchaos) and use purely C++ standard library instead
- Use latest version of FFmpeg and x264, which means no deprecated API is used
- Input mp4 for output mp4 instead of h264 for h264
- Fix the bug that will lead to frame loss in [h264-roi](https://github.com/ChaoticEnigma/h264-roi)
- Fix the bug that relative path will be parsed as absolute path in [h264-roi](https://github.com/ChaoticEnigma/h264-roi)
- Fix the bug that B frames are not applied

[h264-roi](https://github.com/ChaoticEnigma/h264-roi) is a brilliant work, but the author stopped maintenance long time ago. So this repository is given birth to.

### Dependency
- FFmpeg 5.0.1 or above
- x264 latest
- libraries on which FFmpeg/x264 depend

### Quick start
1. Compile FFmpeg and x264 for headers and static/shared library. Note that FFmpeg should be configured to support x264(--enable-libx264)
2. Create a new empty folder:
```shell
mkdir build
```
3. use `cmake` to compile the project. **Don't forget to edit CMakeLists.txt to change include & link path to your own path**:
```shell
cmake ..
make
```
4. modify permission for the executable if possible:
```shell
chmod 777 h264_qpblock
```
5. set qp for regions in forms like:
```shell
h264_qpblock input_mp4_path output_mp4_path -baseqp 0~51 x1,y1,x2,y2:qp x3,y3,x4,y4:qp ...
```
Example:
```shell
h264_qpblock /home/h264-qpblock/in.mp4 /home/h264-qpblock/out.mp4 -baseqp 5 100,100,800,600:45 250,350,400,700:20
```
Note that regions are denoted by: top_left_x,top_left_y,bottom_right_x,bottom_right_y.
Values of QP range from 0 to 51 as usual.

### Other words
- You can disable log output by setting log level in function `main`.
- Note that because of the rate control of x264, the qp settings are not 100% accurate, but the overall effect is good. In fact, uniform qp in one region is not recommended since it conflicts with adaptive quantization of x264.
- Any input video that has pixel format of yuv420p and is supported by FFmpeg is valid. Other pixel format may be implemented later.
- Currently the frame resolution of output video is the same as that of input video. Custom resolution option may be implemented later.
- This is my first time learning and trying C++. Forgive me for any unreasonable code if I've made ;)
