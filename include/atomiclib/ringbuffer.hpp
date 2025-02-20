#ifndef __LOCKFREELIB_RINGBUFFER_HPP__
#define __LOCKFREELIB_RINGBUFFER_HPP__

#include <atomic>
#include <new>
#include <utility>

namespace lockfreelib {

template <typename T>
class RingBuffer {
 private:
  const uint64_t capacity_;
  T* data_;
  std::atomic<uint64_t> head_, tail_, size_;

 public:
  RingBuffer(uint64_t _capacity_)
      : capacity_(_capacity_), head_(0), tail_(0), size_(0) {
    data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
  }

  ~RingBuffer() { ::operator delete(data_); }

  uint64_t size() const { return size_.load(std::memory_order_relaxed); }

  bool push(T&& entry) {
    if (size_.load(std::memory_order_relaxed) == capacity_) {
      return false;
    }
    uint64_t tail = tail_.load(std::memory_order_relaxed);
    while (tail_.compare_exchange_weak(tail, (tail + 1) % capacity_));
    new (&data_[tail_]) T(std::forward<T>(entry));
    size_.fetch_add(1, std::memory_order_release);
    return true;
  }

  template <typename... Args>
  bool emplace(Args&&... args) {
    if (size_.load(std::memory_order_relaxed) == capacity_) {
      return false;
    }
    uint64_t tail = tail_.load(std::memory_order_relaxed);
    while (tail_.compare_exchange_weak(tail, (tail + 1) % capacity_));
    new (&data_[tail]) T(std::forward<Args>(args)...);
    size_.fetch_add(1, std::memory_order_release);
    return true;
  }

  bool pop(T& entry) {
    if (size_.load(std::memory_order_relaxed) == 0) {
      return false;
    }
    uint64_t head = head_.load(std::memory_order_relaxed);
    while (head_.compare_exchange_weak(head, (head + 1) % capacity_));
    entry = data_[head];
    data_[head].~T();
    size_.fetch_sub(1, std::memory_order_release);
    return true;
  }
};  // RingBuffer

}  // namespace lockfreelib

#endif