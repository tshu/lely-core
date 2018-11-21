/**@file
 * This header file is part of the C11 and POSIX compatibility library; it
 * includes `<stdatomic.h>`, if it exists, and defines any missing
 * functionality.
 *
 * @copyright 2013-2018 Lely Industries N.V.
 *
 * @author J. S. Seldenthuis <jseldenthuis@lely.com>
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

#ifndef LELY_LIBC_STDATOMIC_H_
#define LELY_LIBC_STDATOMIC_H_

#include <lely/features.h>

#ifndef LELY_HAVE_STDATOMIC_H
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__) \
		&& __has_include(<stdatomic.h>)
#define LELY_HAVE_STDATOMIC_H 1
#endif
#endif

#if LELY_HAVE_STDATOMIC_H
#include <stdatomic.h>
#else

#undef __STDC_NO_ATOMICS__
#if __has_extension(c_atomic)
#define LELY_HAVE_CLANG_ATOMIC 1
#elif GNUC_PREREQ(4, 7)
#define LELY_HAVE_GNUC_ATOMIC 1
#elif GNUC_PREREQ(4, 1)
#define LELY_HAVE_SYNC_ATOMIC 1
#else
#define __STDC_NO_ATOMICS__ 1
#endif

#ifndef __STDC_NO_ATOMICS__

#include <lely/libc/stdint.h>
#include <lely/libc/uchar.h>

#include <stddef.h>

#ifndef LELY_LIBC_STDATOMIC_INLINE
#define LELY_LIBC_STDATOMIC_INLINE static inline
#endif

#undef __atomic_bool
#ifdef __cplusplus
#define __atomic_bool bool
#else
#define __atomic_bool _Bool
#endif

#ifdef __GCC_ATOMIC_BOOL_LOCK_FREE
#define ATOMIC_BOOL_LOCK_FREE __GCC_ATOMIC_BOOL_LOCK_FREE
#else
#define ATOMIC_BOOL_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_CHAR_LOCK_FREE
#define ATOMIC_CHAR_LOCK_FREE __GCC_ATOMIC_CHAR_LOCK_FREE
#else
#define ATOMIC_CHAR_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_CHAR16_T_LOCK_FREE
#define ATOMIC_CHAR16_T_LOCK_FREE __GCC_ATOMIC_CHAR16_T_LOCK_FREE
#else
#define ATOMIC_CHAR16_T_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_CHAR32_T_LOCK_FREE
#define ATOMIC_CHAR32_T_LOCK_FREE __GCC_ATOMIC_CHAR32_T_LOCK_FREE
#else
#define ATOMIC_CHAR32_T_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_WCHAR_T_LOCK_FREE
#define ATOMIC_WCHAR_T_LOCK_FREE __GCC_ATOMIC_WCHAR_T_LOCK_FREE
#else
#define ATOMIC_WCHAR_T_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_SHORT_LOCK_FREE
#define ATOMIC_SHORT_LOCK_FREE __GCC_ATOMIC_SHORT_LOCK_FREE
#else
#define ATOMIC_SHORT_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_INT_LOCK_FREE
#define ATOMIC_INT_LOCK_FREE __GCC_ATOMIC_INT_LOCK_FREE
#else
#define ATOMIC_INT_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_LONG_LOCK_FREE
#define ATOMIC_LONG_LOCK_FREE __GCC_ATOMIC_LONG_LOCK_FREE
#else
#define ATOMIC_LONG_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_LLONG_LOCK_FREE
#define ATOMIC_LLONG_LOCK_FREE __GCC_ATOMIC_LLONG_LOCK_FREE
#else
#define ATOMIC_LLONG_LOCK_FREE 1
#endif

#ifdef __GCC_ATOMIC_POINTER_LOCK_FREE
#define ATOMIC_POINTER_LOCK_FREE __GCC_ATOMIC_POINTER_LOCK_FREE
#else
#define ATOMIC_POINTER_LOCK_FREE 1
#endif

#ifndef LELY_HAVE_CLANG_ATOMIC
#undef _Atomic
#define _Atomic(T) struct { T volatile __value; }
#endif

typedef _Atomic(__atomic_bool) atomic_bool;
typedef _Atomic(char) atomic_char;
typedef _Atomic(signed char) atomic_schar;
typedef _Atomic(unsigned char) atomic_uchar;
typedef _Atomic(short) atomic_short;
typedef _Atomic(unsigned short) atomic_ushort;
typedef _Atomic(int) atomic_int;
typedef _Atomic(unsigned int) atomic_uint;
typedef _Atomic(long) atomic_long;
typedef _Atomic(unsigned long) atomic_ulong;
#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L
typedef _Atomic(long long) atomic_llong;
typedef _Atomic(unsigned long long) atomic_ullong;
#endif
typedef _Atomic(char16_t) atomic_char16_t;
typedef _Atomic(char32_t) atomic_char32_t;
typedef _Atomic(wchar_t) atomic_wchar_t;
typedef _Atomic(int_least8_t) atomic_int_least8_t;
typedef _Atomic(uint_least8_t) atomic_uint_least8_t;
typedef _Atomic(int_least16_t) atomic_int_least16_t;
typedef _Atomic(uint_least16_t) atomic_uint_least16_t;
typedef _Atomic(int_least32_t) atomic_int_least32_t;
typedef _Atomic(uint_least32_t) atomic_uint_least32_t;
typedef _Atomic(int_least64_t) atomic_int_least64_t;
typedef _Atomic(uint_least64_t) atomic_uint_least64_t;
typedef _Atomic(int_fast8_t) atomic_int_fast8_t;
typedef _Atomic(uint_fast8_t) atomic_uint_fast8_t;
typedef _Atomic(int_fast16_t) atomic_int_fast16_t;
typedef _Atomic(uint_fast16_t) atomic_uint_fast16_t;
typedef _Atomic(int_fast32_t) atomic_int_fast32_t;
typedef _Atomic(uint_fast32_t) atomic_uint_fast32_t;
typedef _Atomic(int_fast64_t) atomic_int_fast64_t;
typedef _Atomic(uint_fast64_t) atomic_uint_fast64_t;
typedef _Atomic(intptr_t) atomic_intptr_t;
typedef _Atomic(uintptr_t) atomic_uintptr_t;
typedef _Atomic(size_t) atomic_size_t;
typedef _Atomic(ptrdiff_t) atomic_ptrdiff_t;
typedef _Atomic(intmax_t) atomic_intmax_t;
typedef _Atomic(uintmax_t) atomic_uintmax_t;

/**
 * The static initializer for an atomic object of a type that is
 * initialization-compatible with <b>value</b>.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define ATOMIC_VAR_INIT(value) (value)
#else
#define ATOMIC_VAR_INIT(value) \
	{ \
		(value) \
	}
#endif

/**
 * Initializes the atomic object at <b>obj</b> with the value <b>value</b>. Note
 * that this function does not avoid data races.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_init(obj, value) __c11_atomic_init(obj, value)
#else
#define atomic_init(obj, value) ((void)((obj)->__value = (value)))
#endif

#ifndef __ATOMIC_RELAXED
#define __ATOMIC_RELAXED 0
#endif
#ifndef __ATOMIC_CONSUME
#define __ATOMIC_CONSUME 1
#endif
#ifndef __ATOMIC_ACQUIRE
#define __ATOMIC_ACQUIRE 2
#endif
#ifndef __ATOMIC_RELEASE
#define __ATOMIC_RELEASE 3
#endif
#ifndef __ATOMIC_ACQ_REL
#define __ATOMIC_ACQ_REL 4
#endif
#ifndef __ATOMIC_SEQ_CST
#define __ATOMIC_SEQ_CST 5
#endif

/// An enumerated type identifying memory constraints.
typedef enum {
	/// No operation orders memory.
	memory_order_relaxed = __ATOMIC_RELAXED,
	/**
	 * A load operation performs a consume operation on the affected memory
	 * location.
	 */
	memory_order_consume = __ATOMIC_CONSUME,
	/**
	 * A load operation performs an acquire operation on the affected memory
	 * location.
	 */
	memory_order_acquire = __ATOMIC_ACQUIRE,
	/**
	 * A store operation performs a release operation on the affected memory
	 * location.
	 */
	memory_order_release = __ATOMIC_RELEASE,
	/**
	 * A load operation performs an acquire operation on the affected memory
	 * location, and a store operation performs a release operation on the
	 * affected memory location.
	 */
	memory_order_acq_rel = __ATOMIC_ACQ_REL,
	/// Enforces a single total order on all affected locations.
	memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

/**
 * Terminates a dependency chain; the argument does not carry a dependency to
 * the return value.
 */
#define kill_dependency(y) \
	__extension__({ \
		__typeof__(y) __y = y; \
		__y; \
	})

#ifdef __cplusplus
extern "C" {
#endif

/// Inserts a fence with semantics according to <b>order</b>.
LELY_LIBC_STDATOMIC_INLINE void atomic_thread_fence(memory_order order);

/**
 * Equivalent to `atomic_thread_fence(order)`, except that the resulting
 * ordering constraints are established only between a thread and a signal
 * handler executing in the same thread.
 */
LELY_LIBC_STDATOMIC_INLINE void atomic_signal_fence(memory_order order);

#ifdef __cplusplus
}
#endif

/**
 * Indicates the lock-free property of integer and address atomic types.
 *
 * @returns 0 if the type is never lock-free, 1 if the type is sometimes
 * lock-free, and 2 if the type is always lock-free.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_is_lock_free(obj) (__c11_atomic_is_lock_free(sizeof(*(obj))))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_is_lock_free(obj) \
	(__atomic_is_lock_free(sizeof((obj)->__value), &(obj)->__value))
#else
#define atomic_is_lock_free(obj) (sizeof((obj)->__value) <= sizeof(void *))
#endif

/**
 * Atomically replaces the value at <b>object</b> with the value of
 * <b>desired</b>. Memory is affected according to <b>order</b>.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_store_explicit(object, desired, order) \
	(__c11_atomic_store((object), (desired), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_store_explicit(object, desired, order) \
	(__atomic_store_n(&(object)->__value, (desired), (order)))
#else
#define atomic_store_explicit(object, desired, order) \
	((void)atomic_exchange_explicit((object), (desired), (order)))
#endif
/**
 * Equivalent to #atomic_store_explicit(object, desired, memory_order_seq_cst).
 */
#define atomic_store(object, desired) \
	(atomic_store_explicit((object), (desired), memory_order_seq_cst))

/**
 * Atomically returns the value at <b>object</b>. Memory is affected according
 * to <b>order</b>.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_load_explicit(object, order) \
	(__c11_atomic_load((object), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_load_explicit(object, order) \
	(__atomic_load_n(&(object)->__value, (order)))
#elif LELY_HAVE_SYNC_ATOMIC
#define atomic_load_explicit(object, order) \
	(__unused_var(order), __sync_fetch_and_add(&(object)->__value, 0))
#endif
/// Equivalent to #atomic_load_explicit(object, memory_order_seq_cst).
#define atomic_load(object) \
	(atomic_load_explicit((object), memory_order_seq_cst))

/**
 * Atomically replaces the value at <b>object</b> with <b>desired</b>. Memory is
 * affected according to <b>order</b>.
 *
 * @returns the value at <b>object</b> immediately before the effects.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_exchange_explicit(object, desired, order) \
	(__c11_atomic_exchange((object), (desired), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_exchange_explicit(object, desired, order) \
	(__atomic_exchange_n(&(object)->__value, (desired), (order)))
#elif LELY_HAVE_SYNC_ATOMIC
#define atomic_exchange_explicit(object, desired, order) \
	__extension__({ \
		__typeof__(object) __object = (object); \
		__typeof__(desired) __desired = (desired); \
		atomic_thread_fence(order); \
		__sync_lock_test_and_set(&(__object)->__value, __desired); \
	})
#endif
/**
 * Equivalent to #atomic_exchange_explicit(object, desired,
 * memory_order_seq_cst).
 */
#define atomic_exchange(object, desired) \
	(atomic_exchange_explicit((object), (desired), memory_order_seq_cst))

/**
 * Atomically compares the value at <b>object</b> for equality with that at
 * <b>expected</b>, and if true, replaces the value at <b>object</b> with
 * <b>desired</b>, and if false, updates the value at <b>expected</b> with the
 * value at <b>object</b>. Further, if the comparison is true, memory is
 * affected according to the value of <b>success</b>, and if the comparison is
 * false, memory is affected according to the value of <b>failure</b>.
 *
 * @returns the result of the comparison.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_compare_exchange_strong_explicit( \
		object, expected, desired, success, failure) \
	(__c11_atomic_compare_exchange_strong((object), (expected), (desired), \
			(success), (failure)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_compare_exchange_strong_explicit( \
		object, expected, desired, success, failure) \
	(__atomic_compare_exchange_n(&(object)->__value, (expected), \
			(desired), 0, (success), (failure)))
#elif LELY_HAVE_SYNC_ATOMIC
// clang-format off
#define atomic_compare_exchange_strong_explicit( \
		object, expected, desired, success, failure) \
	__extension__({ \
		__unused_var(success); \
		__unused_var(failure); \
		__typeof__(*(expected)) __expected = *(expected); \
		(__atomic_bool)((*(expected) = __sync_val_compare_and_swap( \
				&(object)->__value, __expected, (desired))) \
				== __expected); \
	})
// clang-format on
#endif
/**
 * Equivalent to #atomic_compare_exchange_strong_explicit(object, expected,
 * desired, memory_order_seq_cst, memory_order_seq_cst).
 */
#define atomic_compare_exchange_strong(object, expected, desired) \
	(atomic_compare_exchange_strong_explicit((object), (expected), \
			(desired), memory_order_seq_cst, \
			memory_order_seq_cst))

/**
 * Equivalent to atomic_compare_exchange_strong_explicit(), except that a weak
 * compare-and-exchange may fail spuriously. That is, even when the contents of
 * memory referred to by <b>expected</b> and <b>object</b> are equal, it may
 * return zero and store back to <b>expected</b> the same memory contents that
 * were there before.
 *
 * The spurious failure enables implementation of compare-and-exchange on a
 * broader class of machines. When a compare-and-exchange is in a loop, the weak
 * version will yield better performance on some platforms. When a weak
 * compare-and-exchange would require a loop and a strong one would not, the
 * strong one is preferable.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_compare_exchange_weak_explicit( \
		object, expected, desired, success, failure) \
	(__c11_atomic_compare_exchange_weak((object), (expected), (desired), \
			(success), (failure)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_compare_exchange_weak_explicit( \
		object, expected, desired, success, failure) \
	(__atomic_compare_exchange_n(&(object)->__value, (expected), \
			(desired), 1, (success), (failure)))
#else
#define atomic_compare_exchange_weak_explicit( \
		object, expected, desired, success, failure) \
	(atomic_compare_exchange_strong_explicit((object), (expected), \
			(desired), (success), (failure)))
#endif
/**
 * Equivalent to #atomic_compare_exchange_weak_explicit(object, expected,
 * desired, memory_order_seq_cst, memory_order_seq_cst).
 */
#define atomic_compare_exchange_weak(object, expected, desired) \
	(atomic_compare_exchange_weak_explicit((object), (expected), \
			(desired), memory_order_seq_cst, \
			memory_order_seq_cst))

/**
 * Atomically replaces the value at <b>object</b> with `*object + operand`.
 * Memory is affected according to <b>order</b>.
 *
 * @returns the value at <b>object</b> immediately before the effects.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_fetch_add_explicit(object, operand, order) \
	(__c11_atomic_fetch_add((object), (operand), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_fetch_add_explicit(object, operand, order) \
	(__atomic_fetch_add(&(object)->__value, (operand), (order)))
#elif LELY_HAVE_SYNC_ATOMIC
#define atomic_fetch_add_explicit(object, operand, order) \
	(__unused_var(order), \
			__sync_fetch_and_add(&(object)->__value, (operand)))
#endif
/**
 * Equivalent to #atomic_fetch_add_explicit(object, operand,
 * memory_order_seq_cst).
 */
#define atomic_fetch_add(object, operand) \
	(atomic_fetch_add_explicit((object), (operand), memory_order_seq_cst))

/**
 * Atomically replaces the value at <b>object</b> with `*object - operand`.
 * Memory is affected according to <b>order</b>.
 *
 * @returns the value at <b>object</b> immediately before the effects.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_fetch_sub_explicit(object, operand, order) \
	(__c11_atomic_fetch_sub((object), (operand), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_fetch_sub_explicit(object, operand, order) \
	(__atomic_fetch_sub(&(object)->__value, (operand), (order)))
#elif LELY_HAVE_SYNC_ATOMIC
#define atomic_fetch_sub_explicit(object, operand, order) \
	(__unused_var(order), \
			__sync_fetch_and_sub(&(object)->__value, (operand)))
#endif
/**
 * Equivalent to #atomic_fetch_sub_explicit(object, operand,
 * memory_order_seq_cst).
 */
#define atomic_fetch_sub(object, operand) \
	(atomic_fetch_sub_explicit((object), (operand), memory_order_seq_cst))

/**
 * Atomically replaces the value at <b>object</b> with `*object | operand`.
 * Memory is affected according to <b>order</b>.
 *
 * @returns the value at <b>object</b> immediately before the effects.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_fetch_or_explicit(object, operand, order) \
	(__c11_atomic_fetch_or((object), (operand), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_fetch_or_explicit(object, operand, order) \
	(__atomic_fetch_or(&(object)->__value, (operand), (order)))
#elif LELY_HAVE_SYNC_ATOMIC
#define atomic_fetch_or_explicit(object, operand, order) \
	(__unused_var(order), \
			__sync_fetch_and_or(&(object)->__value, (operand)))
#endif
/**
 * Equivalent to #atomic_fetch_or_explicit(object, operand,
 * memory_order_seq_cst).
 */
#define atomic_fetch_or(object, operand) \
	(atomic_fetch_or_explicit((object), (operand), memory_order_seq_cst))

/**
 * Atomically replaces the value at <b>object</b> with `*object ^ operand`.
 * Memory is affected according to <b>order</b>.
 *
 * @returns the value at <b>object</b> immediately before the effects.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_fetch_xor_explicit(object, operand, order) \
	(__c11_atomic_fetch_xor((object), (operand), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_fetch_xor_explicit(object, operand, order) \
	(__atomic_fetch_xor(&(object)->__value, (operand), (order)))
#elif LELY_HAVE_SYNC_ATOMIC
#define atomic_fetch_xor_explicit(object, operand, order) \
	(__unused_var(order), \
			__sync_fetch_and_xor(&(object)->__value, (operand)))
#endif
/**
 * Equivalent to #atomic_fetch_xor_explicit(object, operand,
 * memory_order_seq_cst).
 */
#define atomic_fetch_xor(object, operand) \
	(atomic_fetch_xor_explicit((object), (operand), memory_order_seq_cst))

/**
 * Atomically replaces the value at <b>object</b> with `*object & operand`.
 * Memory is affected according to <b>order</b>.
 *
 * @returns the value at <b>object</b> immediately before the effects.
 */
#if LELY_HAVE_CLANG_ATOMIC
#define atomic_fetch_and_explicit(object, operand, order) \
	(__c11_atomic_fetch_and((object), (operand), (order)))
#elif LELY_HAVE_GNUC_ATOMIC
#define atomic_fetch_and_explicit(object, operand, order) \
	(__atomic_fetch_and(&(object)->__value, (operand), (order)))
#elif LELY_HAVE_SYNC_ATOMIC
#define atomic_fetch_and_explicit(object, operand, order) \
	(__unused_var(order), \
			__sync_fetch_and_and(&(object)->__value, (operand)))
#endif
/**
 * Equivalent to #atomic_fetch_and_explicit(object, operand,
 * memory_order_seq_cst).
 */
#define atomic_fetch_and(object, operand) \
	(atomic_fetch_and_explicit((object), (operand), memory_order_seq_cst))

/**
 * An atomic type providing the classic test-and-set functionality. It has two
 * states, set and clear. Operations on objects of this type are lock free.
 */
typedef struct {
	atomic_bool __value;
} atomic_flag;

/**
 * The static initializer used to initialize an #atomic_flag to the clear state.
 */
#define ATOMIC_FLAG_INIT \
	{ \
		ATOMIC_VAR_INIT(0) \
	}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Atomically sets the value at <b>object</b> to true. Memory is affected
 * according to <b>order</b>.
 *
 * @returns the value at <b>object</b> immediately before the effects.
 */
LELY_LIBC_STDATOMIC_INLINE
__atomic_bool atomic_flag_test_and_set_explicit(
		volatile atomic_flag *object, memory_order order);

/**
 * Equivalent to #atomic_flag_test_and_set_explicit(object,
 * memory_order_seq_cst).
 */
LELY_LIBC_STDATOMIC_INLINE __atomic_bool atomic_flag_test_and_set(
		volatile atomic_flag *object);

/**
 * Atomically sets the value at <b>object</b> to false. Memory is affected
 * according to <b>order</b>.
 */
LELY_LIBC_STDATOMIC_INLINE void atomic_flag_clear_explicit(
		volatile atomic_flag *object, memory_order order);

/**
 * Equivalent to #atomic_flag_test_and_set_explicit(object,
 * memory_order_seq_cst).
 */
LELY_LIBC_STDATOMIC_INLINE void atomic_flag_clear(volatile atomic_flag *object);

inline void
atomic_thread_fence(memory_order order)
{
#if LELY_HAVE_CLANG_ATOMIC
	__c11_atomic_thread_fence(order);
#elif LELY_HAVE_GNUC_ATOMIC
	__atomic_thread_fence(order);
#elif LELY_HAVE_SYNC_ATOMIC
	if (order != memory_order_relaxed)
		__sync_synchronize();
#endif
}

inline void
atomic_signal_fence(memory_order order)
{
#if LELY_HAVE_CLANG_ATOMIC
	__c11_atomic_signal_fence(order);
#elif LELY_HAVE_GNUC_ATOMIC
	__atomic_signal_fence(order);
#elif defined(__GNUC__)
	if (order != memory_order_relaxed)
		__asm volatile("" ::: "memory");
#else
	__unused_var(order);
#endif
}

inline __atomic_bool
atomic_flag_test_and_set_explicit(
		volatile atomic_flag *object, memory_order order)
{
	return atomic_exchange_explicit(&object->__value, 1, order);
}

inline __atomic_bool
atomic_flag_test_and_set(volatile atomic_flag *object)
{
	return atomic_flag_test_and_set_explicit(object, memory_order_seq_cst);
}

inline void
atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order)
{
	atomic_store_explicit(&object->__value, 0, order);
}

inline void
atomic_flag_clear(volatile atomic_flag *object)
{
	atomic_flag_test_and_set_explicit(object, memory_order_seq_cst);
}

#ifdef __cplusplus
}
#endif

#undef __atomic_bool

#endif // !__STDC_NO_ATOMICS__

#endif // LELY_HAVE_STDATOMIC_H

#endif // !LELY_LIBC_STDATOMIC_H_
