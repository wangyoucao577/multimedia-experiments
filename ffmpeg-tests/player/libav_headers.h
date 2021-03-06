
#pragma once

extern "C" {
#define __STDC_CONSTANT_MACROS
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/time.h"
#include "libswresample/swresample.h"
}

constexpr static int AVERROR_OK = 0;

struct AVFrameExtended {
  AVFrame *frame;
  AVMediaType media_type;
  AVRational time_base;
};
