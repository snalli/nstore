#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <mutex>
#include "utils.h"

namespace storage {

// Logging

#define DEBUG(...)
#define ASSERT(cnd)
#define ASSERTinfo(cnd, info)
#define ASSERTeq(lhs, rhs)
#define ASSERTne(lhs, rhs)

#define FATALSYS(...)\
  fatal(errno, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define FATAL(...)\
  fatal(0, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define USAGE(...)\
  usage(Usage, __VA_ARGS__)

/*
 #define DEBUG(...)\
  debug(__FILE__, __LINE__, __func__, __VA_ARGS__)
 // assert a condition is true
 #define ASSERT(cnd)\
  ((void)((cnd) || (fatal(0, __FILE__, __LINE__, __func__,\
  "assertion failure: %s", #cnd), 0)))
 // assertion with extra info printed if assertion fails
 #define ASSERTinfo(cnd, info) \
  ((void)((cnd) || (fatal(0, __FILE__, __LINE__, __func__,\
  "assertion failure: %s (%s = %s)", #cnd, #info, info), 0)))
 // assert two integer values are equal
 #define ASSERTeq(lhs, rhs)\
  ((void)(((lhs) == (rhs)) || (fatal(0, __FILE__, __LINE__, __func__,\
  "assertion failure: %s (%d) == %s (%d)", #lhs,\
  (lhs), #rhs, (rhs)), 0)))
 // assert two integer values are not equal
 #define ASSERTne(lhs, rhs)\
  ((void)(((lhs) != (rhs)) || (fatal(0, __FILE__, __LINE__, __func__,\
  "assertion failure: %s (%d) != %s (%d)", #lhs,\
  (lhs), #rhs, (rhs)), 0)))
 */

// size of the static area returned by pmem_static_area()
#define PMEM_STATIC_SIZE 4096

/* definitions used internally by this implementation */
#define PMEM_SIGNATURE "*PMEMALLOC_POOL"
#define PMEM_PAGE_SIZE 4096 /* size of next three sections */
#define PMEM_NULL_OFFSET 0  /* offset of NULL page (unmapped) */
#define PMEM_STATIC_OFFSET 4096 /* offset of static area */
#define PMEM_RED_OFFSET 8192  /* offset of red zone page (unmapped) */
#define PMEM_HDR_OFFSET 12288 /* offset of pool header */
#define PMEM_CLUMP_OFFSET 16384 /* offset of first clump */
#define PMEM_MIN_POOL_SIZE (1024 * 1024)
#define PMEM_CHUNK_SIZE 64  /* alignment/granularity for all allocations */
#define PMEM_STATE_MASK 63  /* for storing state in size lower bits */
#define PMEM_STATE_FREE 0 /* free clump */
#define PMEM_STATE_RESERVED 1 /* reserved clump */
#define PMEM_STATE_ACTIVE 2 /* active (allocated) clump */
#define PMEM_STATE_UNUSED 3 /* must be highest value + 1 */

/* latency in ns */
#define PCOMMIT_LATENCY 100

// number of onactive/onfree values allowed
#define PMEM_NUM_ON 3

#define MAX_PTRS 512

extern void* pmp;

struct static_info {
  unsigned int init;
  unsigned int itr;
  void* ptrs[MAX_PTRS];
};

extern struct static_info* sp;

struct clump {
  size_t size;  // size of the clump
  size_t prevsize;  // size of previous (lower) clump
  struct {
    off_t off;
    void *ptr_;
  } ons[PMEM_NUM_ON];
};


#define ABS_PTR(p) ((decltype(p))(pmp + (uintptr_t)p))
#define REL_PTR(p) ((decltype(p))((uintptr_t)p - (uintptr_t)pmp))

/* 64B cache line size */
#define ALIGN 64
/* To match Mnemosyne and reuse trace processing tools */
#define LIBPM PSEGMENT_RESERVED_REGION_START
#define PMSIZE PSEGMENT_RESERVED_REGION_SIZE

static inline void *
pmem_map(int fd, size_t len) {
  void *base;

  if ((base = mmap((caddr_t) LIBPM, len, PROT_READ | PROT_WRITE,
  MAP_SHARED | MAP_POPULATE,
                   fd, 0)) == MAP_FAILED)
    return NULL;

  return base;
}

static inline void __pmem_flush_cache(void *addr, size_t len,
                                    __attribute((unused)) int flags) {
  uintptr_t uptr = (uintptr_t) addr & ~(ALIGN - 1);
  uintptr_t end = (uintptr_t) addr + len;

  /* loop through 64B-aligned chunks covering the given range */
  for (; uptr < end; uptr += ALIGN) {
    // __builtin_ia32_clflush((void *) uptr);
  }
}


static inline void __pmem_persist(void *addr, size_t len, int flags) {
  __pmem_flush_cache(addr, len, flags);
  PM_FENCE();
  __builtin_ia32_sfence();
}

#define pmem_flush_cache(addr, len, flags)					\
	({									\
  		uintptr_t uptr = (uintptr_t) addr & ~(ALIGN - 1);		\
  		uintptr_t end = (uintptr_t) addr + len;				\
		uintptr_t *paddr;						\
  		for (; uptr < end; uptr += ALIGN) {				\
			paddr = (uintptr_t *) uptr;				\
		        __asm__ __volatile__ ("clflush %0" : : 			\
							  "m"(*(paddr)));  	\
			/*PM_FLUSHOPT(((void*)uptr), ALIGN, ALIGN);*/		\
  		}								\
	})
#define pmem_persist(addr, len, flags)						\
	({/*									\
    		if (!(LIBPM <= (unsigned long long) addr &&			\
			(unsigned long long) addr <= LIBPM + PMSIZE)) 		\
		{								\
			fprintf(stderr, "%s:%d %p !<= %p !<= %p\n", 		\
						__func__,__LINE__,		\
						((void*)LIBPM),			\
						((void*)addr),			\
						((void*)LIBPM+PMSIZE));		\
	    		assert (LIBPM <= (unsigned long long) addr &&		\
			(unsigned long long) addr <= LIBPM + PMSIZE);		\
		}								\
    		if (!(LIBPM <= (unsigned long long) (addr+len) &&		\
			(unsigned long long) (addr+len) <= LIBPM + PMSIZE))	\
		{								\
			fprintf(stderr, "%s:%d %p !<= %p !<= %p\n", 		\
						__func__,__LINE__,		\
						((void*)LIBPM),			\
						((void*)addr+len),		\
						((void*)LIBPM+PMSIZE));		\
    			assert (LIBPM <= (unsigned long long) (addr+len) &&	\
			(unsigned long long) (addr+len) <= LIBPM + PMSIZE);	\
		} */								\
  		pmem_flush_cache(addr, len, flags);				\
		PM_FENCE();							\
	})
		

#define pmemalloc_activate_helper(abs_ptr) 					\
	({									\
		struct clump *clp;						\
  		size_t sz;							\
  		DEBUG("ptr_=%lx", abs_ptr);					\
		/*
    		if (!(LIBPM <= (unsigned long long) abs_ptr &&			\
			(unsigned long long) abs_ptr <= LIBPM + PMSIZE))	\
		{								\
			fprintf(stderr, "%s:%d %p !<= %p !<= %p\n", 		\
						__func__,__LINE__,		\
						((void*)LIBPM),			\
						((void*)abs_ptr),		\
						((void*)LIBPM+PMSIZE));		\
    			assert (LIBPM <= (unsigned long long) abs_ptr &&	\
			(unsigned long long) abs_ptr <= LIBPM + PMSIZE);	\
		}								\
		*/								\
  		clp = (struct clump *) ((uintptr_t) abs_ptr - PMEM_CHUNK_SIZE);	\
  		ASSERTeq(clp->size & PMEM_STATE_MASK, PMEM_STATE_RESERVED);	\
  		sz = clp->size & ~PMEM_STATE_MASK;				\
  		pmem_persist(abs_ptr, clp->size - PMEM_CHUNK_SIZE, 0);		\
  		PM_EQU((clp->size), (sz | PMEM_STATE_ACTIVE));			\
		pmem_persist(clp, sizeof(*clp), 0);				\
	})

#define pmemalloc_activate(abs_ptr)		\
	pmemalloc_activate_helper(abs_ptr)

void debug(const char *file, int line, const char *func, const char *fmt, ...);
void fatal(int err, const char *file, int line, const char *func,
           const char *fmt, ...);

void *pmemalloc_init(const char *path, size_t size);
void *pmemalloc_static_area();
void *pmemalloc_reserve(size_t size);
// void pmemalloc_activate(void *abs_ptr_);
void pmemalloc_free(void *abs_ptr_);
void pmemalloc_check(const char *path);
unsigned int get_next_pp();

}
