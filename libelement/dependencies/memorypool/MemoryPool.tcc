/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MEMORY_BLOCK_TCC
#define MEMORY_BLOCK_TCC



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::padPointer(data_pointer_ p, size_type align)
const noexcept
{
  uintptr_t result = reinterpret_cast<uintptr_t>(p);
  return ((align - result) % align);
}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool()
noexcept
{
  currentBlock_ = nullptr;
  currentAndLastSlots_.store({nullptr, nullptr});
  freeSlots_.store(nullptr);
}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool)
noexcept :
MemoryPool()
{}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool&& memoryPool)
noexcept
{
  currentBlock_ = memoryPool.currentBlock_;
  memoryPool.currentBlock_ = nullptr;
  currentAndLastSlots_.store(memoryPool.currentAndLastSlots_.load());
  freeSlots_.store(memoryPool.freeSlots_.load());
}


template <typename T, size_t BlockSize>
template<class U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U>& memoryPool)
noexcept :
MemoryPool()
{}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>&
MemoryPool<T, BlockSize>::operator=(MemoryPool&& memoryPool)
noexcept
{
  if (this != &memoryPool)
  {
    std::swap(currentBlock_, memoryPool.currentBlock_);
    currentAndLastSlots_.store(memoryPool.currentAndLastSlots_.load());
    freeSlots_.store(memoryPool.freeSlots_.load());
  }
  return *this;
}



template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool()
noexcept
{
  std::lock_guard<std::mutex> lock(blockAllocMutex_);
  slot_pointer_ curr = currentBlock_;
  while (curr != nullptr) {
    slot_pointer_ prev = curr->next;
    operator delete(reinterpret_cast<void*>(curr));
    curr = prev;
  }
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::address(reference x)
const noexcept
{
  return &x;
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::const_pointer
MemoryPool<T, BlockSize>::address(const_reference x)
const noexcept
{
  return &x;
}



template <typename T, size_t BlockSize>
void
MemoryPool<T, BlockSize>::allocateBlock(slot_pointer_ currentLastSlot)
{
  std::lock_guard<std::mutex> lock(blockAllocMutex_);
  // Now we are locked, check if our last slot is still the same (i.e. we haven't allocated a new block)
  // If it is different, we've hit this twice in succession and a new block has already been allocated
  // When this happens, bomb back out to allocate; this iteration will inevitably fail, but it can try again
  if (currentAndLastSlots_.load().last != currentLastSlot)
    return;
  // Allocate space for the new block and store a pointer to the previous one
  data_pointer_ newBlock = reinterpret_cast<data_pointer_>
                           (operator new(BlockSize));
  reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
  currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
  // Pad block body to staisfy the alignment requirements for elements
  data_pointer_ body = newBlock + sizeof(slot_pointer_);
  size_type bodyPadding = padPointer(body, alignof(slot_type_));
  // Now store the new block into the current slot info
  currentAndLastSlots_.store({
    reinterpret_cast<slot_pointer_>(body + bodyPadding),
    reinterpret_cast<slot_pointer_>(newBlock + BlockSize - sizeof(slot_type_) + 1)});
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::allocate(size_type n, const_pointer hint)
{
  slot_pointer_ lFreeSlots = freeSlots_.load();
  while (lFreeSlots != nullptr) {
    // If true, we successfully updated freeSlots_ and can return our new object
    // If false, we failed to update, but the current value of freeSlots_ is now in lFreeSlots
    // So, iterate until we succeed or run out of free slots
    if (freeSlots_.compare_exchange_weak(lFreeSlots, lFreeSlots->next))
      return reinterpret_cast<pointer>(lFreeSlots);
  }
  // Get the current and last slots and see if we're out of space
  slot_pair_ lCurrentAndLast = currentAndLastSlots_.load();
  for(;;) {
    // If we're out of available slots, attempt to allocate a new block
    // Note that this may fail (if another thread has already done it)
    // In this case we continue, knowing that the compare/exchange will fail
    // This will still result in lCurrentAndLast being updated to the new block's values
    if (lCurrentAndLast.current >= lCurrentAndLast.last) {
      allocateBlock(lCurrentAndLast.last);
    }
    // Increment the current value
    slot_pair_ newCurrentAndLast = {lCurrentAndLast.current + 1, lCurrentAndLast.last};
    // Attempt to store the updated value back
    // If true, we successfully updated currentAndLastSlots_ and can return our new object
    // If false, we failed to update, but the new value of currentAndLastSlots_ is now in lCurrentAndLast
    if (currentAndLastSlots_.compare_exchange_weak(lCurrentAndLast, newCurrentAndLast)) {
      return reinterpret_cast<pointer>(lCurrentAndLast.current);
    }
  }
}



template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deallocate(pointer p, size_type n)
{
  if (p != nullptr) {
    slot_pointer_ lFreeSlots = freeSlots_.load();
    for (;;) {
      // Store this slot's next pointer as the current head
      reinterpret_cast<slot_pointer_>(p)->next = lFreeSlots;
      // If true, the head is unchanged and we are able to replace it (and thus we are done)
      // If false, the head has changed and we must iterate again to complete the operation
      if (freeSlots_.compare_exchange_weak(lFreeSlots, reinterpret_cast<slot_pointer_>(p)))
        return;
    }
  }
}



template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::max_size()
const noexcept
{
  const size_type maxBlocks = -1 / BlockSize;
  return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
}



template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void
MemoryPool<T, BlockSize>::construct(U* p, Args&&... args)
{
  new (p) U (std::forward<Args>(args)...);
}



template <typename T, size_t BlockSize>
template <class U>
inline void
MemoryPool<T, BlockSize>::destroy(U* p)
{
  p->~U();
}



template <typename T, size_t BlockSize>
template <class... Args>
inline typename MemoryPool<T, BlockSize>::pointer
MemoryPool<T, BlockSize>::newElement(Args&&... args)
{
  pointer result = allocate();
  construct<value_type>(result, std::forward<Args>(args)...);
  return result;
}



template <typename T, size_t BlockSize>
inline void
MemoryPool<T, BlockSize>::deleteElement(pointer p)
{
  if (p != nullptr) {
    p->~value_type();
    deallocate(p);
  }
}



#endif // MEMORY_BLOCK_TCC
