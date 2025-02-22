#ifndef __LOCKFREELIB_RINGBUFFER_HPP__
#define __LOCKFREELIB_RINGBUFFER_HPP__

#include <atomic>
#include <new>
#include <string>
#include <utility>

namespace atomiclib {

template <typename T>
class RingBuffer {
 private:
  const uint64_t capacity_;
  T* data_;
  uint8_t *push_mask_, *pop_mask_;
  std::atomic<uint64_t> head_, tail_;

 public:
  RingBuffer(uint64_t _capacity_) : capacity_(_capacity_), head_(0), tail_(0) {
    data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));

    push_mask_ =
        static_cast<uint8_t*>(::operator new(sizeof(uint8_t) * capacity_));
    memset(push_mask_, 0, sizeof(uint8_t) * capacity_);

    pop_mask_ =
        static_cast<uint8_t*>(::operator new(sizeof(uint8_t) * capacity_));
    memset(pop_mask_, 0, sizeof(uint8_t) * capacity_);
  }

  ~RingBuffer() { ::operator delete(data_); }

  void push(T& entry) {
    uint64_t idx = tail_.load(std::memory_order_acquire);
    while ((idx + 1) % capacity_ == head_.load(std::memory_order_acquire) ||
           pop_mask_[(idx + 1) % capacity_] == 1 ||
           !tail_.compare_exchange_weak(idx, (idx + 1) % capacity_,
                                        std::memory_order_release,
                                        std::memory_order_relaxed));
    new (&data_[idx]) T(entry);
    push_mask_[idx] = 1;
  }

  // template <typename... Args>
  // void emplace(Args&&... args) {
  //   uint64_t tail = tail_.load(std::memory_order_relaxed);
  //   while ((tail + 1) % capacity_ == head_.load(std::memory_order_relaxed) ||
  //          !tail_.compare_exchange_weak(tail, (tail + 1) % capacity_));
  //   new (&data_[tail]) T(std::forward<Args>(args)...);
  // }

  void pop(T& entry) {
    uint64_t idx = head_.load(std::memory_order_acquire);
    while (idx == tail_.load(std::memory_order_acquire) ||
           push_mask_[idx] == 0 ||
           !head_.compare_exchange_weak(idx, (idx + 1) % capacity_,
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed));
    entry = data_[idx];
    data_[idx].~T();
    pop_mask_[idx] = 0;
  }
};  // RingBuffer

}  // namespace atomiclib

#endif