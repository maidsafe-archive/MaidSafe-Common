// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MAIDSAFE_COMMON_ALLOCATORS_H_
#define MAIDSAFE_COMMON_ALLOCATORS_H_

// #include <openssl/crypto.h> // for OPENSSL_cleanse()

//  #ifdef WIN32
//  #  ifdef _WIN32_WINNT
//  #    undef _WIN32_WINNT
//  #  endif
//  #  define _WIN32_WINNT 0x0501
//  #  define WIN32_LEAN_AND_MEAN 1
//  #  ifndef NOMINMAX
//  #    define NOMINMAX
//  #  endif
//  #include <windows.h>
//  // This is used to attempt to keep keying material out of swap
//  // Note that VirtualLock does not provide this as a guarantee on Windows,
//  // but, in practice, memory that has been VirtualLock'd almost never gets written to
//  // the pagefile except in rare circumstances where memory is extremely low.
//  #else
#ifndef MAIDSAFE_WIN32
#  include <unistd.h>  // for sysconf
#  include <sys/mman.h>
#  include <climits>  // for PAGESIZE
#endif

#include <string>
#include <memory>
#include <map>
#include <mutex>


namespace maidsafe {

// Thread-safe class to keep track of locked (ie, non-swappable) memory pages.
// Memory locks do not stack, that is, pages which have been locked several times by calls to
// mlock() will be unlocked by a single call to munlock(). This can result in keying material
// ending up in swap when those functions are used naively. This class simulates stacking memory
// locks by keeping a counter per page.
// NOTE: By using a map from each page base address to lock count, this class is optimized for small
// objects that span up to a few pages, mostly smaller than a page. To support large allocations,
// something like an interval tree would be the preferred data structure.
template<typename Locker>
class LockedPageManagerBase {
 public:
  explicit LockedPageManagerBase(size_t page_size) : page_size_(page_size) {
    // Determine bitmask for extracting page from address
    assert(!(page_size_ & (page_size_ - 1)));  // size must be power of two
    page_mask_ = ~(page_size_ - 1);
  }

  // For all pages in affected range, increase lock count
  void LockRange(void *p, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!size)
      return;
    const size_t base_addr = reinterpret_cast<size_t>(p);
    const size_t start_page = base_addr & page_mask_;
    const size_t end_page = (base_addr + size - 1) & page_mask_;
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
  void UnlockRange(void *p, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!size)
      return;
    const size_t base_addr = reinterpret_cast<size_t>(p);
    const size_t start_page = base_addr & page_mask_;
    const size_t end_page = (base_addr + size - 1) & page_mask_;
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
  // Lock memory pages.  addr and len must be a multiple of the system page size
  bool Lock(const void *addr, size_t len) {
#ifdef MAIDSAFE_WIN32
    // return VirtualLock(const_cast<void*>(addr), len);
    return VirtualLock(const_cast<void*>(addr), len) != 0;
#else
    return mlock(addr, len) == 0;
#endif
  }
  // Unlock memory pages.  addr and len must be a multiple of the system page size
  bool Unlock(const void *addr, size_t len) {
#ifdef MAIDSAFE_WIN32
    // return VirtualUnlock(const_cast<void*>(addr), len);
    return VirtualUnlock(const_cast<void*>(addr), len) != 0;
#else
    return munlock(addr, len) == 0;
#endif
  }
};

// Singleton class to keep track of locked (ie, non-swappable) memory pages, for use in
// std::allocator templates.
class LockedPageManager: public LockedPageManagerBase<MemoryPageLocker> {
 public:
  static LockedPageManager instance;  // instantiated in utils.cc (util.cpp)
 private:
  LockedPageManager(): LockedPageManagerBase<MemoryPageLocker>(GetSystemPageSize()) {}
};

// Allocator that locks its contents from being paged out of memory and clears its contents before
// deletion.
template<typename T>
struct secure_allocator : public std::allocator<T> {
  // MSVC8 default copy constructor is broken
  typedef std::allocator<T> base;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type  difference_type;
  typedef typename base::pointer pointer;
  typedef typename base::const_pointer const_pointer;
  typedef typename base::reference reference;
  typedef typename base::const_reference const_reference;
  typedef typename base::value_type value_type;
  secure_allocator() throw() {}
  explicit secure_allocator(const secure_allocator& a) throw() : base(a) {}
  template<typename U>
  explicit secure_allocator(const secure_allocator<U>& a) throw() : base(a) {}
  ~secure_allocator() throw() {}
  template<typename _Other>
  struct rebind {
    typedef secure_allocator<_Other> other;
  };

  T* allocate(std::size_t n, const void *hint = 0) {
    T *p;
    p = std::allocator<T>::allocate(n, hint);
    if (p)
      LockedPageManager::instance.LockRange(p, sizeof(T) * n);
    return p;
  }

  void deallocate(T* p, std::size_t n) {
    if (p) {
#ifdef MAIDSAFE_WIN32
      SecureZeroMemory(p, sizeof(T) * n);
#else
      // OPENSSL_cleanse(p, sizeof(T) * n);
      static unsigned char cleanse_ctr = 0;
      volatile unsigned char* ptr = reinterpret_cast<volatile unsigned char*>(p);
      size_t loop = n, ctr = cleanse_ctr;
      while (loop--) {
        *(ptr++) = (unsigned char)ctr;
        ctr += (17 + ((size_t)ptr & 0xF));
      }
      ptr = (volatile unsigned char*)memchr((void*)p, (unsigned char)ctr, n);
      if (ptr)
        ctr += (63 + (size_t)ptr);
      cleanse_ctr = (unsigned char)ctr;
#endif
      LockedPageManager::instance.UnlockRange(p, sizeof(T) * n);
    }
    std::allocator<T>::deallocate(p, n);
  }
};

// Allocator that clears its contents before deletion.
template<typename T>
struct zero_after_free_allocator : public std::allocator<T> {
  // MSVC8 default copy constructor is broken
  typedef std::allocator<T> base;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type  difference_type;
  typedef typename base::pointer pointer;
  typedef typename base::const_pointer const_pointer;
  typedef typename base::reference reference;
  typedef typename base::const_reference const_reference;
  typedef typename base::value_type value_type;
  zero_after_free_allocator() throw() {}
  zero_after_free_allocator(const zero_after_free_allocator& a) throw() : base(a) {}
  template<typename U>
  zero_after_free_allocator(const zero_after_free_allocator<U>& a) throw() : base(a) {}
  ~zero_after_free_allocator() throw() {}
  template<typename _Other> struct rebind {
    typedef zero_after_free_allocator<_Other> other;
  };

  void deallocate(T* p, std::size_t n) {
    if (p) {
#ifdef MAIDSAFE_WIN32
      SecureZeroMemory(p, sizeof(T) * n);
#else
      // OPENSSL_cleanse(p, sizeof(T) * n);
      static unsigned char cleanse_ctr = 0;
      volatile unsigned char* ptr = reinterpret_cast<volatile unsigned char*>(p);
      size_t loop = n, ctr = cleanse_ctr;
      while (loop--) {
        *(ptr++) = (unsigned char)ctr;
        ctr += (17 + ((size_t)ptr & 0xF));
      }
      ptr = (volatile unsigned char*)memchr((void*)p, (unsigned char)ctr, n);
      if (ptr)
        ctr += (63 + (size_t)ptr);
      cleanse_ctr = (unsigned char)ctr;
#endif
    }
    std::allocator<T>::deallocate(p, n);
  }
};

// This is exactly like std::string, but with a custom allocator.
typedef std::basic_string<char, std::char_traits<char>, secure_allocator<char>> SecureString;

}  // namespace maidsafe

#endif  // MAIDSAFE_COMMON_ALLOCATORS_H_
