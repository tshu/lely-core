/*!\file
 * This is the internal header file of the I/O handle declarations.
 *
 * \copyright 2016 Lely Industries N.V.
 *
 * \author J. S. Seldenthuis <jseldenthuis@lely.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LELY_IO_INTERN_HANDLE_H
#define LELY_IO_INTERN_HANDLE_H

#include "io.h"
#ifndef LELY_NO_ATOMICS
#define LELY_NO_ATOMICS	1
#ifndef LELY_NO_THREADS
#include <lely/libc/stdatomic.h>
#ifndef __STDC_NO_ATOMICS__
#undef LELY_NO_ATOMICS
#endif
#endif
#endif

struct io_handle_vtab;

//! An I/O device handle.
struct io_handle {
	//! A pointer to the virtual table.
	const struct io_handle_vtab *vtab;
	//! The reference count.
#ifndef LELY_NO_ATOMICS
	atomic_size_t ref;
#elif !defined(LELY_NO_THREADS) && defined(_WIN32)
	volatile LONG ref;
#else
	atomic_size_t ref;
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

//! The virtual table of an I/O device handle.
struct io_handle_vtab {
	//! The size (in bytes) of the handle struct.
	size_t size;
	//! A pointer to the \a fini method.
	void (*fini)(struct io_handle *handle);
};

/*!
 * Allocates a new I/O device handle from a virtual table. On success, the
 * reference count is initialized to zero.
 *
 * \returns a pointer to a new device handle, or NULL on error.
 *
 * \see io_handle_free().
 */
struct io_handle *io_handle_alloc(const struct io_handle_vtab *vtab);

//! Frees an I/O device handle. \see io_handle_alloc()
void io_handle_free(struct io_handle *handle);

//! Finalizes an I/O device handle by invoking its \a fini method, if available.
void io_handle_fini(struct io_handle *handle);

/*!
 * Destroys an I/O device handle. This function SHOULD never be called directly.
 * Call io_handle_release() instead.
 */
void io_handle_destroy(struct io_handle *handle);

/*!
 * Increments the reference count of an I/O device handle.
 *
 * \returns \a handle.
 *
 * \see io_handle_release()
 */
static inline struct io_handle *io_handle_acquire(struct io_handle *handle);

/*!
 * Decrements the reference count of an I/O device handle. If the count reaches
 * zero, the handle is destroyed.
 *
 * \see io_handle_acquire()
 */
static inline void io_handle_release(struct io_handle *handle);

/*!
 * Returns 1 if there is only a single reference to the specified I/O device
 * handle, and 0 otherwise.
 */
static inline int io_handle_unique(struct io_handle *handle);

static inline struct io_handle *
io_handle_acquire(struct io_handle *handle)
{
#ifndef LELY_NO_ATOMICS
	atomic_fetch_add_explicit(&handle->ref, 1, memory_order_relaxed);
#elif !defined(LELY_NO_THREADS) && defined(_WIN32)
	InterlockedIncrementNoFence(&handle->ref);
#else
	handle->ref++;
#endif
	return handle;
}

static inline void
io_handle_release(struct io_handle *handle)
{
#ifndef LELY_NO_ATOMICS
	if (atomic_fetch_sub_explicit(&handle->ref, 1, memory_order_release)
			== 1) {
		atomic_thread_fence(memory_order_acquire);
#elif !defined(LELY_NO_THREADS) && defined(_WIN32)
	if (!InterlockedDecrementRelease(&handle->ref)) {
		MemoryBarrier();
#else
	if (handle->ref-- == 1) {
#endif
		io_handle_destroy(handle);
	}
}

static inline int
io_handle_unique(struct io_handle *handle)
{
#ifndef LELY_NO_ATOMICS
	return atomic_load(&handle->ref) == 1;
#else
	return handle->ref == 1;
#endif
}

#ifdef __cplusplus
}
#endif

#endif

