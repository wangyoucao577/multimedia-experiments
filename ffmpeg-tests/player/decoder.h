
#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <thread>

#include "libav_headers.h"

using DataCallback = int(int stream_index, const AVMediaType media_type,
                         AVFrame *f);
using ErrorCallback = int(int);

class Decoder {
public:
  Decoder() = delete;
  Decoder(const Decoder &) = delete;
  Decoder(Decoder &&) = delete;
  Decoder(const std::string &input_file,
          std::function<DataCallback> data_callback)
      : input_file_(input_file), data_callback_(std::move(data_callback)) {}
  ~Decoder() = default;

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

private:
  bool opened{false};
  std::thread t_;

  std::function<DataCallback> data_callback_ = nullptr;
  std::function<ErrorCallback> error_callback_ = nullptr;

  const std::string input_file_;
};