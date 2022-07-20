
#pragma once

#include "libav_headers.h"
#include "audio_queue.h"

#include "SDL.h"
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
  }
  ~Player() = default;

public:
  int Open(const AVCodecContext *v_dec_ctx, const AVCodecContext *a_dec_ctx);
  int Close();
  bool Opened() const { return opened_; }

  int PushAudioFrame(AVFrameExtended f);
  int PopAudioData(unsigned char *data, int len);

  int PushVideoFrame(AVFrameExtended f);
  AVFrameExtended PopVideoFrame();
  void ClearVideoFrames();

  void RefreshDisplay(AVFrame *f);

  // in milliseconds
  int CalculateNextFrameInterval(const AVFrameExtended &f); 

  // return false means normal exit, true means by quit event
  bool SDLEventProc();
  void StopSDLEventProc() const;

private:

  /*** video ***/ 
  SDL_Window *window_{nullptr};
  SDL_Renderer *renderer_{nullptr};
  SDL_Texture *texture_{nullptr};

  std::deque<AVFrameExtended> video_frames_;
  mutable std::mutex video_frames_mutex_;
  std::condition_variable video_frames_cv_;
  const int kMaxCacheFrames{50};

  std::pair<int64_t, AVRational> video_clock_{}; // calculated video clock in AVRational (time_base)
  AVRational video_frame_rate_{};
  void SyncVideoUnsafe(const AVFrameExtended &f); // calculate video clock
  std::pair<int64_t, AVRational> video_clock() const;

  // these 3 vars only will be used in CalculateNextFrameInterval, no need multithreading protection
  std::pair<int64_t, AVRational> last_frame_pts_{};
  double last_frame_delay_{0.0}; // seconds
  double frame_timer_{0.0};

  std::atomic_bool video_flushed_{false};

  SDL_TimerID refresh_timer_id_{0}; // timer to trigger refresh event

  int default_refresh_interval_ms_{0};
  /*** video ***/ 

  /*** audio ***/ 
  AudioSamplesQueue audio_queue_;
  std::mutex audio_queue_mutex_;
  std::condition_variable audio_queue_cv_;
  std::atomic_bool audio_flushed_{false};

  SDL_AudioSpec audio_spec_;
  SwrContext *swr_ctx_{nullptr};
  int audio_device_id_{0};
  /*** audio ***/

public:
  const uint32_t kSDLEventProcStopEvent{
      SDL_RegisterEvents(1)}; // register customized event

private:
  const bool enable_video_{false};
  const bool enable_audio_{false};
  std::atomic_bool opened_{false};
  std::atomic_bool stop_{false};    // notify for stopping

#if defined(SAVE_PLAYBACK_AUDIO)
public:
  FILE *audio_file{nullptr};
#endif
};
