#include <memory>

#include "decoding.h"
#include "encoding.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n",
           argv[0]);
    return -1;
  }
  const char *input_url = argv[1];
  const char *output_url = argv[2];
  av_log(NULL, AV_LOG_INFO, "transcoding task: %s -> %s\n", input_url,
         output_url);

  auto enc = std::make_unique<Encoding>(output_url);

  int64_t total_decoded_video = 0, total_decoded_audio = 0;
  auto data_func = [&total_decoded_video, &total_decoded_audio,
                    &enc](int stream_index, const AVMediaType media_type,
                          AVFrame *f) -> int {
    if (!f) {
      av_log(NULL, AV_LOG_ERROR, "stream %d media_type %s result null frame\n",
             stream_index, av_get_media_type_string(media_type));
    }
    if (!f->buf[0]) {
      av_log(NULL, AV_LOG_INFO,
             "stream %d media_type %s result blank frame for flushing\n",
             stream_index, av_get_media_type_string(media_type));
    } else {
      if (media_type == AVMEDIA_TYPE_VIDEO) {
        av_log(NULL, AV_LOG_DEBUG,
               "stream %d frame pict_type %c, pts %" PRId64 ", pkt_dts %" PRId64
               ", pkt_duration %" PRId64 ", best_effort_timestamp %" PRId64
               ", time_base %d/%d\n",
               stream_index, av_get_picture_type_char(f->pict_type), f->pts,
               f->pkt_dts, f->pkt_duration, f->best_effort_timestamp,
               f->time_base.num, f->time_base.den);
        ++total_decoded_video;
      } else if (media_type == AVMEDIA_TYPE_AUDIO) {
        // av_log(NULL, AV_LOG_INFO, "stream %d frame samples %d\n",
        // stream_index,
        //        f->nb_samples);
        total_decoded_audio += f->nb_samples;
      }
      // ignore other types
    }

    enc->SendFrame(f, media_type);

    return 0;
  };

  auto err_func = [](int err) -> int {
    av_log(NULL, AV_LOG_ERROR, "decoding error %d\n", err);
    return 0;
  };

  auto dec = std::make_unique<Decoding>(input_url, std::move(data_func));
  auto ret = dec->Open();
  if (ret != AVERROR_OK) {
    return ret;
  }
  dec->DumpInputFormat();
  av_log(NULL, AV_LOG_INFO, "\n\n\n");

  ret = enc->Open(dec->CodecContext(AVMEDIA_TYPE_VIDEO), nullptr);
  if (ret != AVERROR_OK) {
    return ret;
  }
  enc->DumpInputFormat();

  enc->RunAsync();
  dec->RunAsync(std::move(err_func));

  dec->Join();
  dec->Close();
  enc->Join();
  enc->Close();

  av_log(NULL, AV_LOG_INFO,
         "total decoded video frames %" PRId64 " audio samples %" PRId64 "\n",
         total_decoded_video, total_decoded_audio);

  return 0;
}