cmake_minimum_required(VERSION 3.23)
project(h264_qpblock)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        main.cpp

        parseOptUtils.h
        parseOptUtils.cpp

        decoder.h
        decoder.cpp

        encoder.h
        encoder.cpp
)

INCLUDE_DIRECTORIES(
        /home/eynnzerr/open/FFmpeg/src/ffmpeg-5.0-x86_64/include/
        /home/eynnzerr/open/github/x264/build/include/
)

LINK_DIRECTORIES(
        /home/eynnzerr/open/FFmpeg/src/ffmpeg-5.0-x86_64/lib/
        /home/eynnzerr/open/github/x264/build/lib/
)

add_executable(h264_qpblock ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(h264_qpblock avformat avcodec avdevice avutil swscale swresample x264)
