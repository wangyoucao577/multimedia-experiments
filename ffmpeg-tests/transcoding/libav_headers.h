
#pragma once

extern "C" {
#define __STDC_CONSTANT_MACROS
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

constexpr static int AVERROR_OK = 0;

const AVRational kFundamentalTimeBase = AVRational{1, 100000}; // 10 ns