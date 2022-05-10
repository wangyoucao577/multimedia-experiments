
#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <thread>

#include "config_ctx.h"
#include "libav_headers.h"

using DataCallback = int(int stream_index, const AVMediaType media_type,
                         AVFrame *f);
using ErrorCallback = int(int);

class Decoding {
public:
  Decoding() = delete;
  Decoding(const Decoding &) = delete;
  Decoding(Decoding &&) = delete;
  Decoding(const std::string &input_file,
           std::function<DataCallback> data_callback,
           const std::shared_ptr<ConfigurationContext> config_ctx)
      : input_file_(input_file), data_callback_(std::move(data_callback)),
        config_ctx_(config_ctx) {}
  ~Decoding();

public:
  int Open();
  void Close();

  int Run();
  int RunAsync(std::function<ErrorCallback> error_callback);
  void Join();

  void DumpInputFormat() const;
  const AVFormatContext *InputContext() const { return ifmt_ctx_; }
  const AVCodecContext *CodecContext(AVMediaType media_type) const;

private:
  void release();

  int run();

  // receive all frames on a stream
  int receive_frames(int stream_index);

private:
  struct DecodingContext {
    AVCodecContext *codec_ctx;
    AVFrame *frame;

    int in_count;
    int out_count;
  };

private:
  AVFormatContext *ifmt_ctx_{nullptr};

  int nb_streams_{0};
  DecodingContext *dec_ctx_ = {
      nullptr}; // ctx per stream, length depends on `nb_streams_`

  AVPacket *pkt_{nullptr};

  // for HWAccel
  AVPixelFormat hw_pix_fmt_{AV_PIX_FMT_NONE};
  AVBufferRef *hw_device_ctx_{nullptr};
  AVBufferRef *hw_frames_ctx_{nullptr}; // for av_frame_get_buffer/transfer_data
  enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                   const enum AVPixelFormat *pix_fmts);
  int hw_decoder_init(const AVCodec *dec, AVCodecContext *ctx,
                      const enum AVHWDeviceType type);

private:
  bool opened{false};
  std::thread t_;

  std::function<DataCallback> data_callback_ = nullptr;
  std::function<ErrorCallback> error_callback_ = nullptr;

  const std::string input_file_;

  const std::shared_ptr<ConfigurationContext> config_ctx_{nullptr};
};