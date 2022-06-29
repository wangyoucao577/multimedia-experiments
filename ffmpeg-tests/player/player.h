
#pragma once

#include "SDL.h"
#include "libav_headers.h"
#include "ring_buffer.h"

#include <atomic>
#include <deque>
#include <mutex>
#include <thread>

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

  int PushVideoFrame(AVFrame *f);
  AVFrame* PopVideoFrame();
  void ClearVideoFrames();

private:

  /*** video ***/ 
  SDL_Window *window_{nullptr};
  SDL_Renderer *renderer_{nullptr};
  SDL_Texture *texture_{nullptr};

  std::deque<AVFrame *> video_frames_;
  mutable std::mutex video_frames_mutex_;
  std::condition_variable video_frames_cv_;
  const int kMaxCacheFrames{50};

  std::atomic_bool video_flushed_{false};

  SDL_TimerID refresh_timer_id_{0}; // timer to trigger refresh event

  std::thread t_;   // thread to respond SDL events, mandantory for video

  void sdlEventProc();
  void refreshDisplay(AVFrame *f);
  /*** video ***/ 

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

public:
  const uint32_t sdl_refresh_event{
      SDL_RegisterEvents(1)}; // register refresh event

private:
  const bool enable_video_{false};
  const bool enable_audio_{false};
  std::atomic_bool opened_{false};

#if defined(SAVE_PLAYBACK_AUDIO)
public:
  FILE *audio_file{nullptr};
#endif
};
