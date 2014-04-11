// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/*  Copyright 2013 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_COMMON_AUTHENTICATION_DETAIL_SAFE_ALLOCATORS_H_
#define MAIDSAFE_COMMON_AUTHENTICATION_DETAIL_SAFE_ALLOCATORS_H_

#ifdef MAIDSAFE_WIN32
#include <Windows.h>
#else
#include <unistd.h>  // for sysconf
#include <sys/mman.h>
#include <climits>  // for PAGESIZE
#endif

#include <string>
#include <memory>
#include <map>
#include <mutex>

namespace maidsafe {

namespace authentication {

namespace detail {

// Thread-safe class to keep track of locked (ie, non-swappable) memory pages.
// Memory locks do not stack, that is, pages which have been locked several times by calls to
// mlock() will be unlocked by a single call to munlock(). This can result in keying material
// ending up in swap when those functions are used naively. This class simulates stacking memory
// locks by keeping a counter per page.
// NOTE: By using a map from each page base address to lock count, this class is optimized for small
// objects that span up to a few pages, mostly smaller than a page. To support large allocations,
// something like an interval tree would be the preferred data structure.
template <typename Locker>
class LockedPageManagerBase {
 public:
  explicit LockedPageManagerBase(size_t page_size)
      : locker_(),
        mutex_(),
        page_size_(page_size),
        page_mask_(~(page_size - 1)),  // bitmask for extracting page from address
        histogram_() {
    assert(!(page_size & (page_size - 1)));  // size must be power of two
  }

  // For all pages in affected range, increase lock count
  void LockRange(void* p, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!size)
      return;
    const size_t base_address = reinterpret_cast<size_t>(p);
    const size_t start_page = base_address & page_mask_;
    const size_t end_page = (base_address + size - 1) & page_mask_;
    for (size_t page = start_page; page <= end_page; page += page_size_) {
      Histogram::iterator it = histogram_.find(page);
      if (it == histogram_.end()) {  // Newly locked page
        locker_.Lock(reinterpret_cast<void*>(page), page_size_);
        histogram_.insert(std::make_pair(page, 1));
      } else {  // Page was already locked; increase counter
        it->second += 1;
      }
    }
  }

  // For all pages in affected range, decrease lock count
  void UnlockRange(void* p, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!size)
      return;
    const size_t base_address = reinterpret_cast<size_t>(p);
    const size_t start_page = base_address & page_mask_;
    const size_t end_page = (base_address + size - 1) & page_mask_;
    for (size_t page = start_page; page <= end_page; page += page_size_) {
      Histogram::iterator it = histogram_.find(page);
      assert(it != histogram_.end());  // Cannot unlock an area that was not locked
      // Decrease counter for page, when it is zero, the page will be unlocked
      it->second -= 1;
      if (it->second == 0) {  // Nothing on the page anymore that keeps it locked
        // Unlock page and remove the count from histogram_
        locker_.Unlock(reinterpret_cast<void*>(page), page_size_);
        histogram_.erase(it);
      }
    }
  }

  // Get number of locked pages for diagnostics
  int GetLockedPageCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return histogram_.size();
  }

 private:
  // map of page base address to lock count
  typedef std::map<size_t, int> Histogram;
  Locker locker_;
  mutable std::mutex mutex_;
  size_t page_size_, page_mask_;
  Histogram histogram_;
};

// Determine system page size in bytes
static inline size_t GetSystemPageSize() {
  size_t page_size;
#if defined(MAIDSAFE_WIN32)
  SYSTEM_INFO sSysInfo;
  GetSystemInfo(&sSysInfo);
  page_size = sSysInfo.dwPageSize;
#elif defined(PAGESIZE)  // defined in limits.h
  page_size = PAGESIZE;
#else  // assume some POSIX OS
  page_size = sysconf(_SC_PAGESIZE);
#endif
  return page_size;
}

// OS-dependent memory page locking/unlocking.
// Defined as policy class to make stubbing for test possible.
class MemoryPageLocker {
 public:
  // Lock memory pages. address and length must be a multiple of the system page size
  bool Lock(const void* address, size_t length) {
#ifdef MAIDSAFE_WIN32
    return VirtualLock(const_cast<void*>(address), length) != 0;
#else
    return mlock(address, length) == 0;
#endif
  }
  // Unlock memory pages. address and length must be a multiple of the system page size
  bool Unlock(const void* address, size_t length) {
#ifdef MAIDSAFE_WIN32
    return VirtualUnlock(const_cast<void*>(address), length) != 0;
#else
    return munlock(address, length) == 0;
#endif
  }
};

// Singleton class to keep track of locked (ie, non-swappable) memory pages, for use in
// std::allocator templates.
class LockedPageManager : public LockedPageManagerBase<MemoryPageLocker> {
 public:
  static LockedPageManager instance;

 private:
  LockedPageManager() : LockedPageManagerBase<MemoryPageLocker>(GetSystemPageSize()) {}
};

// Allocator that locks its contents from being paged out of memory and clears its contents before
// deletion.
template <typename T>
struct safe_allocator : public std::allocator<T> {
  typedef std::allocator<T> base;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type difference_type;
  typedef typename base::pointer pointer;
  typedef typename base::const_pointer const_pointer;
  typedef typename base::reference reference;
  typedef typename base::const_reference const_reference;
  typedef typename base::value_type value_type;

  safe_allocator() {}
  safe_allocator(const safe_allocator& a) : base(a) {}
  template <typename U>
  explicit safe_allocator(const safe_allocator<U>& a)
      : base(a) {}
  ~safe_allocator() {}

  template <typename Other>
  struct rebind {
    typedef safe_allocator<Other> other;
  };

  pointer allocate(size_type count, const void* hint = 0) {
    pointer ptr;
    ptr = std::allocator<value_type>::allocate(count, hint);
    if (ptr)
      LockedPageManager::instance.LockRange(ptr, sizeof(value_type) * count);
    return ptr;
  }

  void deallocate(pointer ptr, size_type count) {
    if (ptr) {
      size_type size(sizeof(value_type) * count);
      volatile char* vptr = reinterpret_cast<volatile char*>(ptr);
      while (size) {
        *vptr = 0;
        vptr++;
        size--;
      }
      LockedPageManager::instance.UnlockRange(ptr, sizeof(value_type) * count);
    }
    std::allocator<value_type>::deallocate(ptr, count);
  }
};

// Allocator that clears its contents before deletion.
template <typename T>
struct zero_after_free_allocator : public std::allocator<T> {
  typedef std::allocator<T> base;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type difference_type;
  typedef typename base::pointer pointer;
  typedef typename base::const_pointer const_pointer;
  typedef typename base::reference reference;
  typedef typename base::const_reference const_reference;
  typedef typename base::value_type value_type;

  zero_after_free_allocator() {}
  zero_after_free_allocator(const zero_after_free_allocator& a) : base(a) {}
  template <typename U>
  explicit zero_after_free_allocator(const zero_after_free_allocator<U>& a)
      : base(a) {}
  ~zero_after_free_allocator() {}

  template <typename Other>
  struct rebind {
    typedef zero_after_free_allocator<Other> other;
  };

  void deallocate(pointer ptr, size_type count) {
    if (ptr) {
      size_type size(sizeof(value_type) * count);
      volatile char* vptr = reinterpret_cast<volatile char*>(ptr);
      while (size) {
        *vptr = 0;
        vptr++;
        size--;
      }
    }
    std::allocator<value_type>::deallocate(ptr, count);
  }
};

}  // namespace detail

}  // namespace authentication

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_AUTHENTICATION_DETAIL_SAFE_ALLOCATORS_H_
