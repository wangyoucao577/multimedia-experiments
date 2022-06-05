
#include "player.h"

void sdl_audio_callback(void *userdata, Uint8 *stream, int len) {
  auto player = (Player *)userdata;
  if (!player->Opened()) {
    return;
  }

  auto n = player->PopAudioData(stream, len);
  if (n != len) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "audio playback requires %d bytes but only got %d", len, n);
  }

#if defined(SAVE_PLAYBACK_AUDIO)
  fwrite(stream, len, 1, player->audio_file);
#endif
}

int Player::PopAudioData(unsigned char *data, int len) {
  if (!opened) {
    return 0;
  }

  auto n = audio_buffer_->Read(data, len);
  audio_buffer_cv_.notify_all();
  return n;
}

int Player::PushAudioData(unsigned char *data, int len) {
  if (!opened) {
    return 0;
  }

  int pushed_n = 0;
  while (opened) {
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
  opened = false;

  SDL_CloseAudio();

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

#if defined(SAVE_PLAYBACK_AUDIO)
  fflush(audio_file);
  fclose(audio_file);
  audio_file = nullptr;
#endif
  return 0;
}

int Player::Open() {
  if (opened) {
    return 0;
  }

  Uint32 sdl_flags = SDL_INIT_TIMER;
  if (enable_video_) {
    sdl_flags |= SDL_INIT_VIDEO;
  }
  if (enable_audio_) {
    sdl_flags |= SDL_INIT_AUDIO;
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
    SDL_AudioSpec wanted_spec, spec;
    wanted_spec.freq = 48000;          // TODO: get from codec
    wanted_spec.format = AUDIO_S16SYS; // TODO: get from codec
    wanted_spec.channels = 2;          // TODO: get from codec
    wanted_spec.silence = 0;
    wanted_spec.samples = 2048;
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = this;

    ret = SDL_OpenAudio(&wanted_spec, &spec);
    if (ret < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_OpenAudio failed, err %s",
                   SDL_GetError());
      return -1;
    }
    SDL_PauseAudio(0);
  }

#if defined(SAVE_PLAYBACK_AUDIO)
  audio_file = fopen("test_player.pcm", "w+");
#endif

  opened = true;
  return 0;
}
