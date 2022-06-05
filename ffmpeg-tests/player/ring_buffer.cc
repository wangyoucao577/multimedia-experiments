
#include "ring_buffer.h"

RingBuffer::RingBuffer(int capacity)
    : capacity_(capacity), buffer_(std::make_unique<uint8_t[]>(capacity)) {}

int RingBuffer::Capacity() const { return capacity_; }

int RingBuffer::Size() const {
  std::lock_guard _(mutex_);
  return size_;
}

void RingBuffer::Clear() {
  std::lock_guard _(mutex_);
  read_index_ = write_index_ = size_ = 0;
}

int RingBuffer::Write(unsigned char *buf, int len) {
  assert(buf && len > 0);
  auto write_n = 0;

  std::lock_guard _(mutex_);
  if (size_ == capacity_) { // full
    return 0;
  }

  if (write_index_ >= read_index_) {
    auto available = capacity_ - write_index_;
    auto write_size = available >= len ? len : available;

    memcpy(buffer_.get() + write_index_, buf, write_size);
    write_index_ += write_size;
    assert(write_index_ <= capacity_);
    if (write_index_ == capacity_) {
      write_index_ = 0; // ring to begin
    }
    size_ += write_size;
    write_n += write_size;
    assert(write_index_ > read_index_ || write_index_ == 0);
  }
  if (size_ == capacity_) { // full
    return write_n;
  }

  if (write_n < len) {
    assert(write_index_ < read_index_);
    auto available = read_index_ - write_index_;
    auto write_size =
        available >= (len - write_n) ? (len - write_n) : available;

    memcpy(buffer_.get() + write_index_, buf + write_n, write_size);
    write_index_ += write_size;
    size_ += write_size;
    write_n += write_size;
    assert(write_index_ <= read_index_);
  }

  return write_n;
}

int RingBuffer::Read(unsigned char *buf, int len) {
  assert(buf && len > 0);
  auto read_n = 0;

  std::lock_guard _(mutex_);
  if (size_ == 0) { // empty
    return 0;
  }

  if (read_index_ >= write_index_) {
    auto available = capacity_ - read_index_;
    auto read_size = available >= len ? len : available;

    memcpy(buf, buffer_.get() + read_index_, read_size);
    read_index_ += read_size;
    assert(read_index_ <= capacity_);
    if (read_index_ == capacity_) {
      read_index_ = 0; // ring to begin
    }
    size_ -= read_size;
    read_n += read_size;
    assert(read_index_ > write_index_ || read_index_ == 0);
  }
  if (size_ == 0) { // empty
    return read_n;
  }

  if (read_n < len) {
    assert(write_index_ > read_index_);
    auto available = write_index_ - read_index_;
    auto read_size = available >= (len - read_n) ? (len - read_n) : available;

    memcpy(buf + read_n, buffer_.get() + read_index_, read_size);
    read_index_ += read_size;
    size_ -= read_size;
    read_n += read_size;
    assert(write_index_ >= read_index_);
  }

  return read_n;
}
