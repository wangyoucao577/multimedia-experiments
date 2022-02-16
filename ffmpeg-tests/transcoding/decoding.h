
#pragma once

#include <cassert>
#include <functional>
#include <string>

#include "libav_headers.h"

using DataCallback = int(int stream_index, const AVMediaType media_type,
                         AVFrame *f);

class Decoding {
public:
  Decoding() = delete;
  Decoding(const Decoding &) = delete;
  Decoding(Decoding &&) = delete;
  Decoding(const std::string &input_file,
           std::function<DataCallback> data_callback)
      : input_file_(input_file), data_callback_(std::move(data_callback)) {}
  ~Decoding();

public:
  int Open();
  void Close();
  int Run();

  void DumpInputFormat() const;

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

private:
  bool opened{false};

  std::function<DataCallback> data_callback_ = nullptr;

  const std::string input_file_;
};