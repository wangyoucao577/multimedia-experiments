
#pragma once

#include <mutex>

class RingBuffer {
public:
  RingBuffer(int capacity);
  RingBuffer() = delete;
  RingBuffer(const RingBuffer &) = delete;
  RingBuffer(RingBuffer &&) = delete;
  ~RingBuffer() = default;

public:
  int Write(unsigned char *buf, int len);
  int Read(unsigned char *buf, int len);
  int Size() const;
  int Capacity() const;
  void Clear();

private:
  const int capacity_{0}; // buffer capacity in bytes, non-changable
  std::unique_ptr<unsigned char[]> buffer_;
  int read_index_{0};
  int write_index_{0};
  int size_{0}; // be able to check whether full or empty when 'read_index_ ==
                // write_index_', so that capacity can be fully used
  mutable std::mutex mutex_;
};
