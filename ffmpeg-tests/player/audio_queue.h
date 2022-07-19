#pragma once

#include "libav_headers.h"
#include <mutex>
#include <deque>
#include <cassert>
#include <utility>


struct AudioSamples {
  AudioSamples() = default;
  ~AudioSamples() {
    if (data) {
      av_freep(&data);
    }
  }
  AudioSamples(const AudioSamples &&) = delete;
  AudioSamples(AudioSamples &&other) {
    data = other.data;
    other.data = nullptr;

    len = other.len;
    read_offset = other.read_offset;
    pts = other.pts;
    time_base = other.time_base;
    samples = other.samples;
  }

  int Read(unsigned char *buf, int len) {
    auto n = Remain() >= len ? len : Remain();
    memcpy(buf, data + read_offset, n);
    read_offset += n;
    return n;
  }

  int Remain() const {  // how many bytes available to read
    return len - read_offset;
  }

  uint8_t *data{nullptr};
  int len{0};
  int read_offset{0};   // how many bytes have been read already

  int64_t pts{0};       // start pts of current samples
  AVRational time_base{0};
  int64_t samples{0};   // how many samples
};

class AudioSamplesQueue {
public:
  AudioSamplesQueue() = default;
  AudioSamplesQueue(const AudioSamplesQueue &) = delete;
  AudioSamplesQueue(AudioSamplesQueue &&) = delete;
  ~AudioSamplesQueue() = default;

public:
  bool Write(AudioSamples &&audio_samples);
  int Read(unsigned char *buf, int len);

  bool Empty() const;
  int64_t front_pts() const;
  AVRational time_base() const;
  std::pair<int64_t, AVRational> audio_clock() const;

private:
  void SyncAudioUnsafe(const AudioSamples &audio_samples); // calculate audio clock

private:
  std::deque<AudioSamples> queue_;
  mutable std::mutex mutex_;

  int64_t front_pts_{0};
  AVRational time_base_{0};
  int64_t audio_clock_{0};  // calculated, use same time_base_
};
