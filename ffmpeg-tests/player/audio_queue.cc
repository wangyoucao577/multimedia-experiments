
#include "audio_queue.h"

bool AudioSamplesQueue::Write(AudioSamples &&audio_samples) {
  std::lock_guard<std::mutex> _(mutex_);
  queue_.emplace_back(std::move(audio_samples));
  return true;
}

int AudioSamplesQueue::Read(unsigned char *buf, int len) {
  std::lock_guard<std::mutex> _(mutex_);

  int n = 0;
  while (!queue_.empty() && n < len) {

    auto &audio_samples = queue_.front();
    n += audio_samples.Read(buf + n, len - n);

    // update pts
    front_pts_ = audio_samples.pts;
    time_base_ = audio_samples.time_base;
    if (audio_clock_ == 0) {    // initialize audio_clock_
      audio_clock_ = front_pts_;
    }

    if (audio_samples.Remain() == 0) {
      queue_.pop_front();

      // sync audio: calculate audio clock
      SyncAudioUnsafe(audio_samples);
    }
  }

  return n;
}

int64_t AudioSamplesQueue::front_pts() const {
  std::lock_guard<std::mutex> _(mutex_);
  return front_pts_;
}

AVRational AudioSamplesQueue::time_base() const { 
  std::lock_guard<std::mutex> _(mutex_);
  return time_base_;
}

std::pair<int64_t, AVRational> AudioSamplesQueue::audio_clock() const {
  std::lock_guard<std::mutex> _(mutex_);
  return {audio_clock_, time_base_};
}

bool AudioSamplesQueue::Empty() const { 
    std::lock_guard<std::mutex> _(mutex_); 
    return queue_.empty();
}

void AudioSamplesQueue::SyncAudioUnsafe(const AudioSamples &audio_samples) {

  // calculate audio_clock
  audio_clock_ = av_add_stable(time_base_, audio_clock_, time_base_,
                               audio_samples.samples);

  // validate
  auto audio_clock_us = av_rescale_q(audio_clock_, time_base_, AV_TIME_BASE_Q);
  auto frame_pts_us = av_rescale_q(front_pts_, time_base_, AV_TIME_BASE_Q);
  //av_log(NULL, AV_LOG_INFO, "audio clock %" PRId64 "ms), latest frame pts %" PRId64 "ms\n",
  //       audio_clock_us/1000, frame_pts_us/1000);

  auto delta_us = audio_clock_us - frame_pts_us;
  if (delta_us < -1000000 || delta_us > 1000000) {
    av_log(NULL, AV_LOG_WARNING,
           "audio clock and latest frame pts delta too big %" PRId64 "\n",
           delta_us);
  }
}
