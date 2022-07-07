
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

    if (audio_samples.Remain() == 0) {
      queue_.pop_front();
    }
  }

  return n;
}

std::pair<int64_t, AVRational> AudioSamplesQueue::front_pts() const {
  std::lock_guard<std::mutex> _(mutex_);
  return std::pair<int64_t, AVRational>{front_pts_, time_base_};
}

bool AudioSamplesQueue::Empty() const { 
    std::lock_guard<std::mutex> _(mutex_); 
    return queue_.empty();
}
