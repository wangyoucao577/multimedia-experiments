

#if defined(_WIN32)
#define SDL_MAIN_HANDLED
#endif

#include "decoder.h"
#include "player.h"

int main(int argc, char *argv[]) {

  av_log_set_level(AV_LOG_INFO);
  if (argc < 2) {
    av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file>\n", argv[0]);
    return -1;
  }
  const char *input_url = argv[1];

  auto player = std::make_unique<Player>(true, false);

  int64_t total_decoded_video = 0, total_decoded_audio = 0;
  auto data_func = [&total_decoded_video, &total_decoded_audio,
                    &player](int stream_index, const AVMediaType media_type,
                             AVFrame *f) -> int {
    assert(f);
    if (media_type == AVMEDIA_TYPE_VIDEO) {
      if (!f->buf[0]) {
        av_log(
            NULL, AV_LOG_INFO,
            "decoded callback stream %d media_type %s result blank frame for "
            "flushing\n",
            stream_index, av_get_media_type_string(media_type));
      } else {
        av_log(NULL, AV_LOG_VERBOSE,
               "decoded callback stream %d frame pict_type %c, pts %" PRId64
               ", pkt_dts %" PRId64 ", pkt_duration %" PRId64
               ", best_effort_timestamp %" PRId64 ", time_base %d/%d\n",
               stream_index, av_get_picture_type_char(f->pict_type), f->pts,
               f->pkt_dts, f->pkt_duration, f->best_effort_timestamp,
#if LIBAVUTIL_VERSION_MAJOR >= 57 && LIBAVUTIL_VERSION_MINOR >= 10
               f->time_base.num, f->time_base.den
#else
               0, 0
#endif
        );
        ++total_decoded_video;
      }
      player->PushVideoFrame(f);

    } else if (media_type == AVMEDIA_TYPE_AUDIO) {
      if (!f->buf[0]) {
        av_log(
            NULL, AV_LOG_INFO,
            "decoded callback stream %d media_type %s result blank frame for "
            "flushing\n",
            stream_index, av_get_media_type_string(media_type));
      } else {
        av_log(NULL, AV_LOG_VERBOSE,
               "decoded callback stream %d frame samples %d\n", stream_index,
               f->nb_samples);
        total_decoded_audio += f->nb_samples;
      }
      player->PushAudioFrame(f);
    }
    // ignore other types

    return 0;
  };

  auto err_func = [](int err) -> int {
    av_log(NULL, AV_LOG_ERROR, "decoding error %d\n", err);
    return 0;
  };

  auto dec = std::make_unique<Decoder>(input_url, std::move(data_func));
  auto ret = dec->Open();
  if (ret != AVERROR_OK) {
    return ret;
  }
  dec->DumpInputFormat();
  av_log(NULL, AV_LOG_INFO, "\n\n\n");

  ret = player->Open(dec->CodecContext(AVMEDIA_TYPE_VIDEO),
                     dec->CodecContext(AVMEDIA_TYPE_AUDIO));
  if (ret != AVERROR_OK) {
    return ret;
  }
  dec->RunAsync(std::move(err_func));

  dec->Join();
  dec->Close();
  player->Close();

  av_log(NULL, AV_LOG_INFO,
         "playing done, total decoded video frames %" PRId64
         " audio samples %" PRId64 "\n",
         total_decoded_video, total_decoded_audio);

  return 0;
}
