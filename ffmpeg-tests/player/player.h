
#pragma once

#include "SDL.h"
#include "libav_headers.h"
#include "ring_buffer.h"

#include <atomic>
#include <deque>
#include <mutex>

//#define SAVE_PLAYBACK_AUDIO

class Player {
public:
  Player() = delete;
  Player(const Player &) = delete;
  Player(Player &&) = delete;
  Player(bool enable_video, bool enable_audio)
      : enable_video_(enable_video), enable_audio_(enable_audio) {
    audio_buffer_ = std::make_unique<RingBuffer>(kAudioBufferSizeInBytes);
  }
  ~Player() = default;

public:
  int Open(const AVCodecContext *v_dec_ctx, const AVCodecContext *a_dec_ctx);
  int Close();
  bool Opened() const { return opened_; }

  int PushAudioFrame(AVFrame *f);
  int PushAudioData(unsigned char *data, int len);
  int PopAudioData(unsigned char *data, int len);

private:
  SDL_Window *window_{nullptr};
  SDL_Renderer *renderer_{nullptr};
  SDL_Texture *texture_{nullptr};


  /*** audio ***/ 
  std::unique_ptr<RingBuffer> audio_buffer_;
  std::mutex audio_buffer_mutex_;
  std::condition_variable audio_buffer_cv_;
  const int kAudioBufferSizeInBytes = 1024000; // 1MB
  std::atomic_bool audio_flushed_{false};

  uint8_t *audio_resample_buffer_ {nullptr};
  const int kMaxSamplesPerResample{1024 * 100}; // max 1024*100 per resample

  SDL_AudioSpec audio_spec_;
  SwrContext *swr_ctx_{nullptr};
  int audio_device_id_{0};
  /*** audio ***/

private:
  const bool enable_video_{false};
  const bool enable_audio_{false};
  std::atomic_bool opened_{false};

#if defined(SAVE_PLAYBACK_AUDIO)
public:
  FILE *audio_file{nullptr};
#endif
};
