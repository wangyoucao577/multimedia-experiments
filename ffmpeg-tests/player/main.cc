
#include "decoder.h"
#include "player.h"

int main(int argc, char *argv[]) {

  av_log_set_level(AV_LOG_INFO);
  if (argc < 2) {
    av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file>\n", argv[0]);
    return -1;
  }
  const char *input_url = argv[1];

  // convert AV_SAMPLE_FMT_FLTP to AV_SAMPLE_FMT_S16
  SwrContext *swr_ctx = nullptr;

  auto player = std::make_unique<Player>(false, true);
  // TODO: set codec parameters to player

  int64_t total_decoded_video = 0, total_decoded_audio = 0;
  auto data_func = [&total_decoded_video, &total_decoded_audio, &player,
                    &swr_ctx](int stream_index, const AVMediaType media_type,
                              AVFrame *f) -> int {
    if (!f) {
      av_log(NULL, AV_LOG_ERROR,
             "decoded callback stream %d media_type %s result null frame\n",
             stream_index, av_get_media_type_string(media_type));
    }
    if (!f->buf[0]) {
      av_log(NULL, AV_LOG_INFO,
             "decoded callback stream %d media_type %s result blank frame for "
             "flushing\n",
             stream_index, av_get_media_type_string(media_type));
    } else {
      if (media_type == AVMEDIA_TYPE_VIDEO) {
        av_log(NULL, AV_LOG_VERBOSE,
               "decoded callback stream %d frame pict_type %c, pts %" PRId64
               ", pkt_dts %" PRId64 ", pkt_duration %" PRId64
               ", best_effort_timestamp %" PRId64 ", time_base %d/%d\n",
               stream_index, av_get_picture_type_char(f->pict_type), f->pts,
               f->pkt_dts, f->pkt_duration, f->best_effort_timestamp,
               f->time_base.num, f->time_base.den);
        ++total_decoded_video;
      } else if (media_type == AVMEDIA_TYPE_AUDIO) {
        av_log(NULL, AV_LOG_VERBOSE,
               "decoded callback stream %d frame samples %d\n", stream_index,
               f->nb_samples);
        total_decoded_audio += f->nb_samples;

        if (swr_ctx == nullptr) {
          swr_ctx =
              swr_alloc_set_opts(NULL, // we're allocating a new context
                                 AV_CH_LAYOUT_STEREO,       // out_ch_layout
                                 AV_SAMPLE_FMT_S16,         // out_sample_fmt
                                 48000,                     // out_sample_rate
                                 f->channel_layout,         // in_ch_layout
                                 (AVSampleFormat)f->format, // in_sample_fmt
                                 f->sample_rate,            // in_sample_rate
                                 0,                         // log_offset
                                 NULL);                     // log_ctx
          swr_init(swr_ctx);
        }
        uint8_t *output;
        int out_samples =
            av_rescale_rnd(swr_get_delay(swr_ctx, 48000) + f->nb_samples, 48000,
                           f->sample_rate, AV_ROUND_UP);
        av_samples_alloc(&output, NULL, 2, out_samples, AV_SAMPLE_FMT_S16, 0);
        out_samples = swr_convert(swr_ctx, &output, out_samples,
                                  (const uint8_t **)f->data, f->nb_samples);

        auto len = f->nb_samples * f->channels *
                   2; // TODO: use codec parameters to set sample size
        player->PushAudioData(output, len);

        av_freep(&output);
      }
      // ignore other types
    }

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

  ret = player->Open(); // TODO: codec parameters
  if (ret != AVERROR_OK) {
    return ret;
  }
  dec->RunAsync(std::move(err_func));

  dec->Join();
  dec->Close();
  player->Close();

  if (swr_ctx) {
    swr_free(&swr_ctx);
  }

  av_log(NULL, AV_LOG_INFO,
         "playing done, total decoded video frames %" PRId64
         " audio samples %" PRId64 "\n",
         total_decoded_video, total_decoded_audio);

  return 0;
}
