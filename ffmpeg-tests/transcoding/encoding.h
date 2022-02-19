
#include "libav_headers.h"
#include <string>

class Encoding {
public:
  Encoding() = delete;
  Encoding(const Encoding &) = delete;
  Encoding(Encoding &&) = delete;
  Encoding(const std::string &output_file) : output_file_(output_file) {}
  ~Encoding();

public:
  int Open(const AVCodecContext *v_dec_ctx, const AVCodecContext *a_dec_ctx);
  void Close();

  int Run();
  //   int RunAsync(std::function<ErrorCallback> error_callback);
  //   void Join();

  void DumpInputFormat() const;

private:
  void release();

  //  int run();

private:
  struct EncodingContext {
    AVCodecContext *codec_ctx;
    AVPacket *pkt;

    int in_count;
    int out_count;
  };

private:
  AVFormatContext *ofmt_ctx_{nullptr};

  const static int kMaxStreams = 2; // video & audio
  int nb_streams_{0};
  EncodingContext *enc_ctx_ = {
      nullptr}; // ctx per stream, length depends on `nb_streams_`

  AVFrame *frame_{nullptr};

private:
  bool opened{false};
  //   std::thread t_;

  //   std::function<DataCallback> data_callback_ = nullptr;
  //   std::function<ErrorCallback> error_callback_ = nullptr;

  const std::string output_file_;
};