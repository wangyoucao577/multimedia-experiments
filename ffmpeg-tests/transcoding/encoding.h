
#include "libav_headers.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>
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

  // int Run();
  int RunAsync();
  void Join();

  int SendFrame(const AVFrame *frame, AVMediaType media_type);

  void DumpInputFormat() const;

private:
  struct EncodingContext {
    AVCodecContext *codec_ctx = nullptr;
    AVPacket *pkt = nullptr;

    int in_count = 0;
    int out_count = 0;
  };

  struct AVFrameWithMediaType {
    AVFrame *frame = nullptr;
    AVMediaType media_type = AVMEDIA_TYPE_UNKNOWN;
  };

private:
  void release();

  int run();

  int pushFrame(const AVFrame *frame, AVMediaType media_type);

  int findEncodingContextIndex(AVMediaType media_type) const;

  // receive all packets on a stream
  int receive_packets(int stream_index, EncodingContext &enc_ctx);

private:
  AVFormatContext *ofmt_ctx_{nullptr};

  const static int kMaxStreams = 2; // video & audio
  int nb_streams_{0};
  EncodingContext *enc_ctx_ = {
      nullptr}; // ctx per stream, length depends on `nb_streams_`
  std::set<AVMediaType> enabled_media_types_;

private:
  bool opened{false};
  std::thread t_;

  std::mutex mtx_;
  std::condition_variable cv_;
  std::queue<AVFrameWithMediaType> frame_queue_;

  //   std::function<DataCallback> data_callback_ = nullptr;
  //   std::function<ErrorCallback> error_callback_ = nullptr;

  const std::string output_file_;
};