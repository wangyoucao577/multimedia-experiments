
#include "player.h"
#include <cassert>

void sdl_audio_callback(void *userdata, Uint8 *stream, int len) {
  auto player = (Player *)userdata;
  if (!player->Opened()) {
    return;
  }

  auto n = player->PopAudioData(stream, len);
  if (n != len) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                 "audio playback requires %d bytes but only got %d", len, n);
  }

#if defined(SAVE_PLAYBACK_AUDIO)
  fwrite(stream, len, 1, player->audio_file);
#endif
}


int Player::PushAudioFrame(AVFrame *f) {
  if (!opened_ || audio_flushed_) {   // don't allow push data again if flushed
    return 0;
  }

  if (!f || !f->data[0]) { // empty frame, no more data will be pushed
    audio_flushed_.store(true);
    return 0;
  }

  if (swr_ctx_ == nullptr) { // use libswresample to convert for SDL playout
    swr_ctx_ = swr_alloc_set_opts(
        NULL, // we're allocating a new context
        av_get_default_channel_layout(audio_spec_.channels), // out_ch_layout
        AV_SAMPLE_FMT_S16,                                   // out_sample_fmt
        audio_spec_.freq,                                    // out_sample_rate
        f->channel_layout,                                   // in_ch_layout
        (AVSampleFormat)f->format,                           // in_sample_fmt
        f->sample_rate,                                      // in_sample_rate
        0,                                                   // log_offset
        NULL);                                               // log_ctx
    swr_init(swr_ctx_);
  }

  uint8_t *output;
  int out_samples =
      av_rescale_rnd(swr_get_delay(swr_ctx_, f->sample_rate) + f->nb_samples,
                     audio_spec_.freq, f->sample_rate, AV_ROUND_UP);
  av_samples_alloc(&output, NULL, audio_spec_.channels, out_samples, AV_SAMPLE_FMT_S16, 0);

  out_samples = swr_convert(swr_ctx_, &output, out_samples,
                            (const uint8_t **)f->data, f->nb_samples);

  auto size = av_samples_get_buffer_size(NULL, audio_spec_.channels, out_samples,
                                        AV_SAMPLE_FMT_S16, 0);  
   
  auto ret = PushAudioData(output, size);

  av_freep(&output);
  return ret;
}


int Player::PopAudioData(unsigned char *data, int len) {
  if (!opened_) {
    return 0;
  }

  auto n = audio_buffer_->Read(data, len);
  audio_buffer_cv_.notify_all();
  return n;
}

int Player::PushAudioData(unsigned char *data, int len) {

  int pushed_n = 0;
  while (opened_) {
    auto n = audio_buffer_->Write(data, len);
    pushed_n += n;
    if (pushed_n == len) { // until all data has been write
      break;
    }

    std::unique_lock<std::mutex> l(audio_buffer_mutex_);
    audio_buffer_cv_.wait(l);
  }

  return pushed_n;
}

int Player::Close() {
  if (!opened_) {
    return 0;
  }

  // wait until all data consumed if flushed
  if (audio_flushed_) {
    while (true) {
      using namespace std::chrono_literals;
      std::unique_lock<std::mutex> l(audio_buffer_mutex_);
      if (audio_buffer_cv_.wait_for(
              l, 10ms, [this] { return audio_buffer_->Size() == 0; })) {
        break;   // if audio buffer is empty
      }
    }
  }

  opened_.store(false);

  if (audio_device_id_ > 0) {
    SDL_CloseAudioDevice(audio_device_id_);
    audio_device_id_ = 0;
  }
  audio_flushed_.store(false);

  if (texture_) {
    SDL_DestroyTexture(texture_);
    texture_ = nullptr;
  }
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
  SDL_Quit();

  if (audio_buffer_) {
    audio_buffer_->Clear();
  }

  if (swr_ctx_) {
    swr_free(&swr_ctx_);
  }


#if defined(SAVE_PLAYBACK_AUDIO)
  fflush(audio_file);
  fclose(audio_file);
  audio_file = nullptr;
#endif
  return 0;
}

int Player::Open(const AVCodecContext *v_dec_ctx,
                 const AVCodecContext *a_dec_ctx) {
  if (opened_) {
    return 0;
  }

  Uint32 sdl_flags = SDL_INIT_TIMER;
  if (enable_video_) {
    assert(v_dec_ctx);
    sdl_flags |= SDL_INIT_VIDEO;
  }
  if (enable_audio_) {
    assert(a_dec_ctx);
    sdl_flags |= SDL_INIT_AUDIO;
#if defined(_WIN32)
    // the SDL_AUDIODRIVER is mandantory on windows, otherwise no voice can be hear
    const char *kSDLAudioDriverName = "SDL_AUDIODRIVER";
    const char *kSDLAudioDriverValue = "directsound";
    SDL_setenv(kSDLAudioDriverName, kSDLAudioDriverValue, 0);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "set env for SDL: %s=%s\n",
                 kSDLAudioDriverName, kSDLAudioDriverValue);
#endif
  }
  auto ret = SDL_Init(sdl_flags);
  if (ret < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed, err %d(%s)",
                 ret, SDL_GetError());
    return ret;
  }

  if (enable_video_) { // video window/renderer/texture
    window_ = SDL_CreateWindow("player", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, 1920,
                               1080, // TODO: get from codec
                               //   SDL_WINDOW_FULLSCREEN |
                               SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_CreateWindow failed, err %s", SDL_GetError());
      return -1;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    if (!renderer_) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_CreateRenderer failed, err %s", SDL_GetError());
      return -1;
    }

    // Allocate a place to put our YUV image on that screen
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV,
                                 SDL_TEXTUREACCESS_STREAMING, 1920,
                                 1080); // TODO: get from codec
    if (!texture_) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_CreateTexture failed, err %s", SDL_GetError());
      return -1;
    }
  }

  if (enable_audio_) {
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = a_dec_ctx->sample_rate;        
    wanted_spec.format = AUDIO_S16SYS; 
    wanted_spec.channels = 2; // a_dec_ctx->channels; // failed with 6(5.1) on windows with directsound, use 2 as default by simple
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024; 
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = this;
    audio_device_id_ = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &audio_spec_,
                              SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audio_device_id_ <= 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_OpenAudio failed, err %s",
                   SDL_GetError());
      return -1;
    }
    if (audio_spec_.format != AUDIO_S16SYS) {
      av_log(NULL, AV_LOG_ERROR,
             "SDL advised audio format %d is not supported!\n", audio_spec_.format);
      return -1;
    }
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "SDL_OpenAudio want (%d channels, %d Hz), got (%d channels, %d Hz)\n",
        wanted_spec.channels, wanted_spec.freq, audio_spec_.channels,
        audio_spec_.freq);

    SDL_PauseAudioDevice(audio_device_id_, 0);
  }

#if defined(SAVE_PLAYBACK_AUDIO)
  audio_file = fopen("test_player.pcm", "w+");
#endif

  opened_ = true;
  return 0;
}
