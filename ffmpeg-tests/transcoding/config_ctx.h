#pragma once

#include "libav_headers.h"

#include <string>

// Configuration across all processing.
struct ConfigurationContext {
  AVHWDeviceType hwaccel_device_type{AV_HWDEVICE_TYPE_NONE}; // hwaccel device

  // frame in cuda internal memory, don't copy to CPU
  bool hwaccel_output_format_cuda{false};

  // copy frames from decoder to cuda memory for caching.
  // The `No decoder surface left` is very easy to occur when use cuda nvdec
  // hwaccel, suggest to enable it in such case.
  bool enable_cuda_frames_caching{false};

  // How many decoded frames can be cached in memory or GPU memory,
  // 0 means no limitation.
  int max_cache_frames{0};

  std::string hw_encoder_name; // set hardware encoder name if expect to use
};