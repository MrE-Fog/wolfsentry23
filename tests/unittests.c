/*
 * unittests.c
 *
 * Copyright (C) 2021-2023 wolfSSL Inc.
 *
 * This file is part of wolfSentry.
 *
 * wolfSentry is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSentry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#define _GNU_SOURCE

#define WOLFSENTRY_SOURCE_ID WOLFSENTRY_SOURCE_ID_USER_BASE
#define WOLFSENTRY_ERROR_ID_UNIT_TEST_FAILURE WOLFSENTRY_ERROR_ID_USER_BASE

#include "src/wolfsentry_internal.h"

#ifdef WOLFSENTRY_LWIP

#define iovec iovec /* inhibit lwIP's definition of struct iovec */
#include "lwip/sockets.h"
#undef iovec

/* supply libc-based implementations of lwip_inet_{ntop,pton}, since we're not
 * currently building/linking any lwIP objects for the unit tests.
 */
#undef inet_ntop
const char *inet_ntop(int af, const void *restrict src,
                             char *restrict dst, socklen_t size);
const char *lwip_inet_ntop(int af, const void *restrict src,
                             char *restrict dst, socklen_t size) {
    return inet_ntop(af, src, dst, size);
}
#undef inet_pton
int inet_pton(int af, const char *restrict src, void *restrict dst);
int lwip_inet_pton(int af, const char *restrict src, void *restrict dst) {
    return inet_pton(af, src, dst);
}

#else /* !WOLFSENTRY_LWIP */

#include <sys/socket.h>
#include <netinet/in.h>

#endif /* WOLFSENTRY_LWIP */

#include <stdlib.h>
#include <unistd.h>

#ifdef WOLFSENTRY_NO_STDIO
#define printf(...)
#endif

#ifdef WOLFSENTRY_THREADSAFE

#include <unistd.h>
#include <pthread.h>

#define WOLFSENTRY_EXIT_ON_FAILURE(...) do { wolfsentry_errcode_t _retval = (__VA_ARGS__); if (_retval < 0) { WOLFSENTRY_WARN("%s: " WOLFSENTRY_ERROR_FMT "\n", #__VA_ARGS__, WOLFSENTRY_ERROR_FMT_ARGS(_retval)); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(expected, ...) do { wolfsentry_errcode_t _retval = (__VA_ARGS__); if (! WOLFSENTRY_ERROR_CODE_IS(_retval, expected)) { WOLFSENTRY_WARN("%s: expected %s but got: " WOLFSENTRY_ERROR_FMT "\n", #__VA_ARGS__, #expected, WOLFSENTRY_ERROR_FMT_ARGS(_retval)); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_UNLESS_EXPECTED_SUCCESS(expected, ...) do { wolfsentry_errcode_t _retval = (__VA_ARGS__); if (! WOLFSENTRY_SUCCESS_CODE_IS(_retval, expected)) { WOLFSENTRY_WARN("%s: expected %s but got: " WOLFSENTRY_ERROR_FMT "\n", #__VA_ARGS__, #expected, WOLFSENTRY_ERROR_FMT_ARGS(_retval)); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_SYSFAILURE(...) do { int _retval = (__VA_ARGS__); if (_retval < 0) { perror(#__VA_ARGS__); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_SYSFALSE(...) do { if (! (__VA_ARGS__)) { perror(#__VA_ARGS__); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_SUCCESS(...) do { if ((__VA_ARGS__) >= 0) { WOLFSENTRY_WARN("%s should have failed, but succeeded.\n", #__VA_ARGS__); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_FALSE(...) do { if (! (__VA_ARGS__)) { WOLFSENTRY_WARN("%s should have been true, but was false.\n", #__VA_ARGS__); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_TRUE(...) do { if (__VA_ARGS__) { WOLFSENTRY_WARN("%s should have been false, but was true.\n", #__VA_ARGS__); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(...) do { int _pthread_ret; if ((_pthread_ret = (__VA_ARGS__)) != 0) { WOLFSENTRY_WARN("%s: %s\n", #__VA_ARGS__, strerror(_pthread_ret)); exit(1); }} while(0)

#else /* !WOLFSENTRY_THREADSAFE */

#define WOLFSENTRY_EXIT_ON_FAILURE(...) do { wolfsentry_errcode_t _retval = (__VA_ARGS__); if (_retval < 0) { WOLFSENTRY_WARN("%s: " WOLFSENTRY_ERROR_FMT "\n", #__VA_ARGS__, WOLFSENTRY_ERROR_FMT_ARGS(_retval)); return 1; }} while(0)
#define WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(expected, ...) do { wolfsentry_errcode_t _retval = (__VA_ARGS__); if (! WOLFSENTRY_ERROR_CODE_IS(_retval, expected)) { WOLFSENTRY_WARN("%s: expected %s but got: " WOLFSENTRY_ERROR_FMT "\n", #expected, #__VA_ARGS__, WOLFSENTRY_ERROR_FMT_ARGS(_retval)); return 1; }} while(0)
#define WOLFSENTRY_EXIT_UNLESS_EXPECTED_SUCCESS(expected, ...) do { wolfsentry_errcode_t _retval = (__VA_ARGS__); if (! WOLFSENTRY_SUCCESS_CODE_IS(_retval, expected)) { WOLFSENTRY_WARN("%s: expected %s but got: " WOLFSENTRY_ERROR_FMT "\n", #expected, #__VA_ARGS__, WOLFSENTRY_ERROR_FMT_ARGS(_retval)); return 1; }} while(0)
#define WOLFSENTRY_EXIT_ON_SYSFAILURE(...) do { int _retval = (__VA_ARGS__); if (_retval < 0) { perror(#__VA_ARGS__); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_SYSFALSE(...) do { if (! (__VA_ARGS__)) { perror(#__VA_ARGS__); exit(1); }} while(0)
#define WOLFSENTRY_EXIT_ON_SUCCESS(...) do { if ((__VA_ARGS__) >= 0) { WOLFSENTRY_WARN("%s should have failed, but succeeded.\n", #__VA_ARGS__); return 1; }} while(0)
#define WOLFSENTRY_EXIT_ON_FALSE(...) do { if (! (__VA_ARGS__)) { WOLFSENTRY_WARN("%s should have been true, but was false.\n", #__VA_ARGS__); return 1; }} while(0)
#define WOLFSENTRY_EXIT_ON_TRUE(...) do { if (__VA_ARGS__) { WOLFSENTRY_WARN("%s should have been false, but was true.\n", #__VA_ARGS__); return 1; }} while(0)

#endif /* WOLFSENTRY_THREADSAFE */

/* If not defined use default allocators */
#ifndef WOLFSENTRY_TEST_HPI
#  define WOLFSENTRY_TEST_HPI NULL
#else
extern struct wolfsentry_host_platform_interface* WOLFSENTRY_TEST_HPI;
#endif

#define TEST_SKIP(name) static int name (void) { printf("[  skipping " #name "  ]\n"); return 0; }


#ifdef TEST_INIT

static wolfsentry_errcode_t test_init (void) {
    struct wolfsentry_context *wolfsentry;
#ifdef WOLFSENTRY_HAVE_DESIGNATED_INITIALIZERS
    struct wolfsentry_eventconfig config = { .route_private_data_size = 32, .max_connection_count = 10 };
#else
    struct wolfsentry_eventconfig config = { 32, 0, 10, 0, 0, 0, 0 };
#endif
    wolfsentry_errcode_t ret;
    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    ret = wolfsentry_init(wolfsentry_build_settings,
                          WOLFSENTRY_CONTEXT_ARGS_OUT_EX(WOLFSENTRY_TEST_HPI),
                          &config,
                          &wolfsentry);
    printf("wolfsentry_init() returns " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
    if (ret < 0)
        WOLFSENTRY_ERROR_RERETURN(ret);

    ret = wolfsentry_shutdown(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&wolfsentry));
    printf("wolfsentry_shutdown() returns " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_ERROR_RERETURN(ret);
}

#endif /* TEST_INIT */

#if defined(TEST_RWLOCKS)

#if defined(WOLFSENTRY_THREADSAFE)

#include <signal.h>

struct rwlock_args {
    struct wolfsentry_context *wolfsentry;
    volatile int *measured_sequence;
    volatile int *measured_sequence_i;
    int thread_id;
    struct wolfsentry_rwlock *lock;
    wolfsentry_time_t max_wait;
    pthread_mutex_t thread_phase_lock; /* need to wrap a mutex around thread_phase to blind the thread sanitizer to the spin locks on it. */
    volatile int thread_phase;
};

#define INCREMENT_PHASE(x) do { WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_lock(&(x)->thread_phase_lock)); ++(x)->thread_phase; WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_unlock(&(x)->thread_phase_lock)); } while(0)

static void *rd_routine(struct rwlock_args *args) {
    int i;
    WOLFSENTRY_THREAD_HEADER(WOLFSENTRY_THREAD_FLAG_NONE);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_GET_ERROR);
    INCREMENT_PHASE(args);
    if (args->max_wait >= 0)
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared_timed(args->lock, thread, args->max_wait, WOLFSENTRY_LOCK_FLAG_NONE));
    else
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);
    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id;
    usleep(10000);
    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id + 4;
    INCREMENT_PHASE(args);
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));
    return 0;
}

static void *wr_routine(struct rwlock_args *args) {
    int i;
    WOLFSENTRY_THREAD_HEADER(WOLFSENTRY_THREAD_FLAG_NONE);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_GET_ERROR);
    INCREMENT_PHASE(args);
    if (args->max_wait >= 0)
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex_timed(args->lock, thread, args->max_wait, WOLFSENTRY_LOCK_FLAG_NONE));
    else
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);
    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id;
    usleep(10000);
    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id + 4;
    INCREMENT_PHASE(args);
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));
    return 0;
}

static void *rd2wr_routine(struct rwlock_args *args) {
    int i;
    WOLFSENTRY_THREAD_HEADER(WOLFSENTRY_THREAD_FLAG_NONE);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_GET_ERROR);
    INCREMENT_PHASE(args);
    if (args->max_wait >= 0)
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared_timed(args->lock, thread, args->max_wait, WOLFSENTRY_LOCK_FLAG_NONE));
    else
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id;
    INCREMENT_PHASE(args);
    if (args->max_wait >= 0)
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex_timed(args->lock, thread, args->max_wait, WOLFSENTRY_LOCK_FLAG_NONE));
    else
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);
    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id + 4;
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));
    return 0;
}

static void *rd2wr_reserved_routine(struct rwlock_args *args) {
    int i;
    WOLFSENTRY_THREAD_HEADER(WOLFSENTRY_THREAD_FLAG_NONE);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_GET_ERROR);
    INCREMENT_PHASE(args);
    if (args->max_wait >= 0)
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared_timed(args->lock, thread, args->max_wait, WOLFSENTRY_LOCK_FLAG_NONE));
    else
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE)); // GCOV_EXCL_LINE
    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id;

    INCREMENT_PHASE(args);
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex_reserve(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex_redeem(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));


    INCREMENT_PHASE(args);

    i = WOLFSENTRY_ATOMIC_POSTINCREMENT(*args->measured_sequence_i,1);
    args->measured_sequence[i] = args->thread_id + 4;
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(args->lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    INCREMENT_PHASE(args);
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));
    return 0;
}

#define MAX_WAIT 100000
#define WAIT_FOR_PHASE(x, atleast) do { int cur_phase; WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_lock(&(x).thread_phase_lock)); cur_phase = (x).thread_phase; WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_unlock(&(x).thread_phase_lock)); if (cur_phase >= (atleast)) break; usleep(1000); } while(1)

static int test_rw_locks (void) {
    struct wolfsentry_context *wolfsentry;
    struct wolfsentry_rwlock *lock;
    struct wolfsentry_eventconfig config = { .route_private_data_size = 32, .max_connection_count = 10 };

    volatile int measured_sequence[8], measured_sequence_i = 0;
    int measured_sequence_transposed[8];

    pthread_t thread1, thread2, thread3, thread4;
    struct rwlock_args thread1_args, thread2_args, thread3_args, thread4_args;

    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    (void)alarm(1);

#ifdef WOLFSENTRY_LOCK_SHARED_ERROR_CHECKING
#define test_rw_locks_WOLFSENTRY_INIT_FLAGS WOLFSENTRY_INIT_FLAG_LOCK_SHARED_ERROR_CHECKING
#define test_rw_locks_WOLFSENTRY_LOCK_FLAGS WOLFSENTRY_LOCK_FLAG_SHARED_ERROR_CHECKING
#else
#define test_rw_locks_WOLFSENTRY_INIT_FLAGS WOLFSENTRY_INIT_FLAG_NONE
#define test_rw_locks_WOLFSENTRY_LOCK_FLAGS WOLFSENTRY_LOCK_FLAG_NONE
#endif

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_init_ex(wolfsentry_build_settings,
                                                  WOLFSENTRY_TEST_HPI,
                                                  thread,
                                                  &config,
                                                  &wolfsentry,
                                                  test_rw_locks_WOLFSENTRY_INIT_FLAGS
                                   ));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_alloc(wolfsentry_get_hpi(wolfsentry), thread, &lock,
                                                     test_rw_locks_WOLFSENTRY_LOCK_FLAGS
                                   ));

    {
        struct wolfsentry_thread_context_public uninited_thread_buffer =
            WOLFSENTRY_THREAD_CONTEXT_PUBLIC_INITIALIZER;
        struct wolfsentry_thread_context *uninited_thread =
            (struct wolfsentry_thread_context *)&uninited_thread_buffer;
        wolfsentry_thread_flags_t thread_flags;
        wolfsentry_thread_id_t thread_id;
        struct wolfsentry_thread_context *null_thread = NULL;
        struct wolfsentry_rwlock *null_lock = NULL;
        wolfsentry_lock_flags_t lock_flags;
#define TEST_INVALID_ARGS(x) WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(INVALID_ARG, x)
        TEST_INVALID_ARGS(wolfsentry_get_thread_id(NULL, NULL));
        TEST_INVALID_ARGS(wolfsentry_get_thread_id(uninited_thread, NULL));
        TEST_INVALID_ARGS(wolfsentry_get_thread_id(uninited_thread, &thread_id));
        TEST_INVALID_ARGS(wolfsentry_get_thread_id(thread, NULL));
        TEST_INVALID_ARGS(wolfsentry_get_thread_flags(null_thread, NULL));
        TEST_INVALID_ARGS(wolfsentry_get_thread_flags(uninited_thread, NULL));
        TEST_INVALID_ARGS(wolfsentry_get_thread_flags(uninited_thread, &thread_flags));
        TEST_INVALID_ARGS(wolfsentry_get_thread_flags(thread, NULL));
        TEST_INVALID_ARGS(wolfsentry_destroy_thread_context(null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_destroy_thread_context(uninited_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_free_thread_context(NULL, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_free_thread_context(NULL, &null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_free_thread_context(NULL, &uninited_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_set_deadline_rel_usecs(wolfsentry, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_set_deadline_rel_usecs(wolfsentry, uninited_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_set_deadline_abs(wolfsentry, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_set_deadline_abs(wolfsentry, uninited_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_clear_deadline(wolfsentry, null_thread));
        TEST_INVALID_ARGS(wolfsentry_clear_deadline(wolfsentry, uninited_thread));
        TEST_INVALID_ARGS(wolfsentry_set_thread_readonly(null_thread));
        TEST_INVALID_ARGS(wolfsentry_set_thread_readonly(uninited_thread));
        TEST_INVALID_ARGS(wolfsentry_set_thread_readwrite(null_thread));
        TEST_INVALID_ARGS(wolfsentry_set_thread_readwrite(uninited_thread));
        TEST_INVALID_ARGS(wolfsentry_lock_init(NULL, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_init(NULL, uninited_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_init(NULL, uninited_thread, lock, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_alloc(NULL, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_alloc(NULL, uninited_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_alloc(NULL, uninited_thread, &lock, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_destroy(NULL, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_free(NULL, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_free(&null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared_abstimed(null_lock, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared_abstimed(lock, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared_abstimed(null_lock, thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared_timed(null_lock, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared_timed(lock, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared_timed(null_lock, thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex_abstimed(null_lock, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex_abstimed(null_lock, thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex_timed(null_lock, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex_timed(null_lock, thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex2shared(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex2shared(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_mutex2shared(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_reserve(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_reserve(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_reserve(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem_abstimed(null_lock, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem_abstimed(lock, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem_abstimed(null_lock, thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem_timed(null_lock, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem_timed(lock, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem_timed(null_lock, thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_redeem(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_abandon(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_abandon(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_abandon(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_abstimed(null_lock, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_abstimed(lock, null_thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_abstimed(null_lock, thread, NULL, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_timed(null_lock, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_timed(lock, null_thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex_timed(null_lock, thread, 0, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_shared2mutex(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_unlock(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_unlock(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_shared(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_shared(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_shared(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_mutex(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_mutex(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_either(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_either(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_shared2mutex_reservation(null_lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_shared2mutex_reservation(lock, null_thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_have_shared2mutex_reservation(null_lock, thread, 0));
        TEST_INVALID_ARGS(wolfsentry_lock_get_flags(null_lock, null_thread, &lock_flags));
        TEST_INVALID_ARGS(wolfsentry_lock_get_flags(null_lock, thread, &lock_flags));
        TEST_INVALID_ARGS(wolfsentry_lock_get_flags(lock, thread, NULL));
    }

    memset(&thread1_args, 0, sizeof thread1_args);
    thread1_args.wolfsentry = wolfsentry;
    thread1_args.measured_sequence = measured_sequence;
    thread1_args.measured_sequence_i = &measured_sequence_i;
    thread1_args.lock = lock;
    thread1_args.max_wait = -1;
    thread2_args = thread3_args = thread4_args = thread1_args;

    thread1_args.thread_id = 1;
    thread2_args.thread_id = 2;
    thread3_args.thread_id = 3;
    thread4_args.thread_id = 4;

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_init(&thread1_args.thread_phase_lock, NULL));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_init(&thread2_args.thread_phase_lock, NULL));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_init(&thread3_args.thread_phase_lock, NULL));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_mutex_init(&thread4_args.thread_phase_lock, NULL));


    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex_timed(lock, thread, 0, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread1, 0 /* attr */, (void *(*)(void *))rd_routine, (void *)&thread1_args));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread2, 0 /* attr */, (void *(*)(void *))rd_routine, (void *)&thread2_args));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread3, 0 /* attr */, (void *(*)(void *))wr_routine, (void *)&thread3_args));

    /* go to a lot of trouble to make sure thread 3 has entered _lock_mutex() wait. */
    WAIT_FOR_PHASE(thread3_args, 1);
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_kill(thread3, 0));
    usleep(10000);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread1, 0 /* retval */));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread4, 0 /* attr */, (void *(*)(void *))wr_routine, (void *)&thread4_args));
usleep(10000);
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread4, 0 /* retval */));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread2, 0 /* retval */));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread3, 0 /* retval */));

    /* the first write-locking thread must get the lock first after the parent unlocks,
     * because write lock requests have priority over read lock requests all else equal.
     * the second write-locking thread must get the lock last, because it is launched
     * after the first read lock thread has returned.  there is a race between the other read-locking thread
     * launching and the first write locking thread completing, but the usleep(10000) before the
     * parent unlock relaxes that race.  there is a second race between the first read-locking
     * thread returning and the other read-locking thread activating its read lock -- the second
     * write-locking thread can beat it by getting lock->sem first.  this race is relaxed with the
     * usleep(10000) in rd_routine().  the usleep(10000) in wr_routine() is just to catch lock
     * violations in the measured_sequence.
     *
     * the sequence of the two read-locking threads, sandwiched between the write-locking threads,
     * is undefined, and experimentally does vary.
     *
     */

    if ((measured_sequence[0] != 3) ||
        (measured_sequence[6] != 4) ||
        (measured_sequence[1] != 7) ||
        (measured_sequence[7] != 8)) {
    // GCOV_EXCL_START
        size_t i;
        fprintf(stderr,"wrong sequence at L%d.  should be {3,7,1,2,5,6,4,8} (the middle 4 are safely permutable), but got {", __LINE__);
        for (i = 0; i < sizeof measured_sequence / sizeof measured_sequence[0]; ++i)
            fprintf(stderr,"%d%s",measured_sequence[i], i == (sizeof measured_sequence / sizeof measured_sequence[0]) - 1 ? "}.\n" : ",");
        WOLFSENTRY_ERROR_RETURN(NOT_OK);
    // GCOV_EXCL_STOP
    }

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_destroy(lock, thread, test_rw_locks_WOLFSENTRY_LOCK_FLAGS));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_init(wolfsentry_get_hpi(wolfsentry), thread, lock,
                                                    test_rw_locks_WOLFSENTRY_LOCK_FLAGS));

    /* now a scenario with shared2mutex and mutex2shared in the mix: */

    thread1_args.thread_phase = thread2_args.thread_phase = thread3_args.thread_phase = thread4_args.thread_phase = 0;

    measured_sequence_i = 0;

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    thread1_args.max_wait = MAX_WAIT; /* builtin wolfsentry_time_t is microseconds, same as usleep(). */
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread1, 0 /* attr */, (void *(*)(void *))rd_routine, (void *)&thread1_args));

    WAIT_FOR_PHASE(thread1_args, 1);
    thread2_args.max_wait = MAX_WAIT;
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread2, 0 /* attr */, (void *(*)(void *))rd2wr_routine, (void *)&thread2_args));

    WAIT_FOR_PHASE(thread2_args, 1);

    /* this transition advances thread1 and thread2 to both hold shared locks.
     * non-negligible chance that thread2 goes into shared2mutex wait before
     * thread1 can get a shared lock.
     */
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex2shared(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WAIT_FOR_PHASE(thread2_args, 2);

    /* this thread has to wait until thread2 is done with its shared2mutex sequence. */

/* constraint: thread2 must unlock (6) before thread3 locks (3) */
/* constraint: thread3 lock-unlock (3, 7) must be adjacent */
    thread3_args.max_wait = MAX_WAIT;
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread3, 0 /* attr */, (void *(*)(void *))wr_routine, (void *)&thread3_args));

    WAIT_FOR_PHASE(thread3_args, 1);

    /* this one must fail, because at this point thread2 must be in shared2mutex wait. */
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(BUSY, wolfsentry_lock_shared2mutex(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    /* take the opportunity to test expected failures of the _timed() variants. */
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(BUSY, wolfsentry_lock_mutex_timed(lock, thread, 0, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(BUSY, wolfsentry_lock_mutex_timed(lock, thread, 1000, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(TIMED_OUT, wolfsentry_lock_mutex_timed(lock, NULL /* thread */, 1000, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(INVALID_ARG, wolfsentry_lock_shared_timed(lock, NULL /* thread */, 0, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(ALREADY, wolfsentry_lock_shared_timed(lock, thread, 0, WOLFSENTRY_LOCK_FLAG_NONRECURSIVE_SHARED));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(ALREADY, wolfsentry_lock_shared_timed(lock, thread, 1000, WOLFSENTRY_LOCK_FLAG_NONRECURSIVE_SHARED));

    /* this unlock allows thread2 to finally get its mutex. */
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread1, 0 /* retval */));

/* constraint: thread2 must unlock (6) before thread4 locks (4) */
/* constraint: thread4 lock-unlock (4, 8) must be adjacent */
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread4, 0 /* attr */, (void *(*)(void *))wr_routine, (void *)&thread4_args));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread4, 0 /* retval */));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread2, 0 /* retval */));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread3, 0 /* retval */));

    {
        int i;
        for (i=0; i<8; ++i)
            measured_sequence_transposed[measured_sequence[i] - 1] = i + 1;
    }
#define SEQ(x) measured_sequence_transposed[(x)-1]
    if ((SEQ(6) > SEQ(3)) ||
        (SEQ(7) - SEQ(3) != 1) ||
        (SEQ(6) > SEQ(4)) ||
        (SEQ(8) - SEQ(4) != 1)) {
    // GCOV_EXCL_START
        size_t i;
        fprintf(stderr,"wrong sequence at L%d.  got {", __LINE__);
        for (i = 0; i < sizeof measured_sequence / sizeof measured_sequence[0]; ++i)
            fprintf(stderr,"%d%s",measured_sequence[i], i == (sizeof measured_sequence / sizeof measured_sequence[0]) - 1 ? "}.\n" : ",");
        WOLFSENTRY_ERROR_RETURN(NOT_OK);
    // GCOV_EXCL_STOP
    }


    /* again, using shared2mutex reservation: */

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_destroy(lock, thread, test_rw_locks_WOLFSENTRY_LOCK_FLAGS));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_init(wolfsentry_get_hpi(wolfsentry), thread, lock,
                                                    test_rw_locks_WOLFSENTRY_LOCK_FLAGS));

    thread1_args.thread_phase = thread2_args.thread_phase = thread3_args.thread_phase = thread4_args.thread_phase = 0;

    measured_sequence_i = 0;

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(ALREADY, wolfsentry_lock_shared2mutex_reserve(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(ALREADY, wolfsentry_lock_shared2mutex_redeem(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(INCOMPATIBLE_STATE, wolfsentry_lock_shared2mutex_abandon(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    thread1_args.max_wait = MAX_WAIT; /* builtin wolfsentry_time_t is microseconds, same as usleep(). */
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread1, 0 /* attr */, (void *(*)(void *))rd_routine, (void *)&thread1_args));

    WAIT_FOR_PHASE(thread1_args, 1);

    thread2_args.max_wait = MAX_WAIT;
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread2, 0 /* attr */, (void *(*)(void *))rd2wr_reserved_routine, (void *)&thread2_args));

    WAIT_FOR_PHASE(thread2_args, 1);

    /* this transition advances thread1 and thread2 to both hold shared locks.
     * non-negligible chance that thread2 goes into shared2mutex wait before
     * thread1 can get a shared lock.
     */
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex2shared(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WAIT_FOR_PHASE(thread2_args, 3);

    /* this thread has to wait until thread2 is done with its shared2mutex sequence. */

/* constraint: thread2 must unlock (6) before thread3 locks (3) */
/* constraint: thread3 lock-unlock (3, 7) must be adjacent */
    thread3_args.max_wait = MAX_WAIT;
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread3, 0 /* attr */, (void *(*)(void *))wr_routine, (void *)&thread3_args));

    WAIT_FOR_PHASE(thread3_args, 1);

    /* this one must fail, because at this point thread2 must be in shared2mutex wait. */
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(BUSY, wolfsentry_lock_shared2mutex(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    /* take the opportunity to test expected failures of the _timed() variants. */
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(BUSY, wolfsentry_lock_mutex_timed(lock, thread, 0, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(BUSY, wolfsentry_lock_mutex_timed(lock, thread, 1000, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(TIMED_OUT, wolfsentry_lock_mutex_timed(lock, NULL /* thread */, 1000, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(ALREADY, wolfsentry_lock_shared_timed(lock, thread, 0, WOLFSENTRY_LOCK_FLAG_NONRECURSIVE_SHARED));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(ALREADY, wolfsentry_lock_shared_timed(lock, thread, 1000, WOLFSENTRY_LOCK_FLAG_NONRECURSIVE_SHARED));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_have_shared(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(LACKING_MUTEX, wolfsentry_lock_have_mutex(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    /* this unlock allows thread2 to finally get its mutex. */
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread1, 0 /* retval */));

/* constraint: thread2 must unlock (6) before thread4 locks (4) */
/* constraint: thread4 lock-unlock (4, 8) must be adjacent */
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_create(&thread4, 0 /* attr */, (void *(*)(void *))wr_routine, (void *)&thread4_args));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread4, 0 /* retval */));

    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread2, 0 /* retval */));
    WOLFSENTRY_EXIT_ON_FAILURE_PTHREAD(pthread_join(thread3, 0 /* retval */));

    {
        int i;
        for (i=0; i<8; ++i)
            measured_sequence_transposed[measured_sequence[i] - 1] = i + 1;
    }
#define SEQ(x) measured_sequence_transposed[(x)-1]
    if ((SEQ(6) > SEQ(3)) ||
        (SEQ(7) - SEQ(3) != 1) ||
        (SEQ(6) > SEQ(4)) ||
        (SEQ(8) - SEQ(4) != 1)) {
    // GCOV_EXCL_START
        size_t i;
        fprintf(stderr,"wrong sequence at L%d.  got {", __LINE__);
        for (i = 0; i < sizeof measured_sequence / sizeof measured_sequence[0]; ++i)
            fprintf(stderr,"%d%s",measured_sequence[i], i == (sizeof measured_sequence / sizeof measured_sequence[0]) - 1 ? "}.\n" : ",");
        WOLFSENTRY_ERROR_RETURN(NOT_OK);
    // GCOV_EXCL_STOP
    }

    /* cursory exercise of compound reservation calls. */

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_destroy(lock, thread, test_rw_locks_WOLFSENTRY_LOCK_FLAGS));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_init(wolfsentry_get_hpi(wolfsentry), thread, lock,
                                                    test_rw_locks_WOLFSENTRY_LOCK_FLAGS));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex2shared(lock, thread, WOLFSENTRY_LOCK_FLAG_GET_RESERVATION_TOO));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex_redeem(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared(lock, thread, WOLFSENTRY_LOCK_FLAG_GET_RESERVATION_TOO));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex_redeem(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared_timed(lock, thread, 1000, WOLFSENTRY_LOCK_FLAG_GET_RESERVATION_TOO));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_shared2mutex_redeem_timed(lock, thread, 1000, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    /* cursory exercise of null thread calls. */
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_destroy(lock, NULL /* thread */, test_rw_locks_WOLFSENTRY_LOCK_FLAGS));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_init(wolfsentry_get_hpi(wolfsentry), NULL /* thread */, lock,
                                                    test_rw_locks_WOLFSENTRY_LOCK_FLAGS));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex(lock, NULL /* thread */, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, NULL /* thread */, WOLFSENTRY_LOCK_FLAG_NONE));

    /* exercise interrupt-handler-style lock cycle. */
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex_timed(lock, thread, 0, WOLFSENTRY_LOCK_FLAG_RETAIN_SEMAPHORE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_mutex_timed(lock, thread, 0, WOLFSENTRY_LOCK_FLAG_RETAIN_SEMAPHORE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_unlock(lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_lock_free(&lock, thread, WOLFSENTRY_LOCK_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_shutdown(&wolfsentry, thread));

    (void)alarm(0);

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_RETURN_OK;
}

#else

TEST_SKIP(test_rw_locks)

#endif /* WOLFSENTRY_THREADSAFE */

#endif /* TEST_RWLOCKS */

#ifdef TEST_STATIC_ROUTES

#define PRIVATE_DATA_SIZE 32
#ifndef PRIVATE_DATA_ALIGNMENT
#define PRIVATE_DATA_ALIGNMENT 16
#endif

static int test_static_routes (void) {

    struct wolfsentry_context *wolfsentry;
    wolfsentry_action_res_t action_results;
    int n_deleted;
    wolfsentry_ent_id_t id;
    wolfsentry_route_flags_t inexact_matches;

    struct {
        struct wolfsentry_sockaddr sa;
        byte addr_buf[4];
    } remote, local, remote_wildcard, local_wildcard;

    struct wolfsentry_eventconfig config = {
#ifdef WOLFSENTRY_HAVE_DESIGNATED_INITIALIZERS
        .route_private_data_size = PRIVATE_DATA_SIZE,
        .route_private_data_alignment = PRIVATE_DATA_ALIGNMENT,
        .max_connection_count = 10,
        .derogatory_threshold_for_penaltybox = 4,
        .penaltybox_duration = 1, /* denominated in seconds when passing to wolfsentry_init(). */
        .route_idle_time_for_purge = 0,
        .flags = WOLFSENTRY_EVENTCONFIG_FLAG_NONE
#else
        PRIVATE_DATA_SIZE,
        PRIVATE_DATA_ALIGNMENT,
        10,
        4,
        1, /* denominated in seconds when passing to wolfsentry_init(). */
        0,
        WOLFSENTRY_EVENTCONFIG_FLAG_NONE
#endif
    };

    wolfsentry_route_flags_t flags = WOLFSENTRY_ROUTE_FLAG_TCPLIKE_PORT_NUMBERS, flags_wildcard;

    struct wolfsentry_route_table *main_routes;
    struct wolfsentry_route *route_ref;
    wolfsentry_ent_id_t route_id;
    int prefixlen;
    byte *private_data;
    size_t private_data_size;

    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_init_ex(
            wolfsentry_build_settings,
            WOLFSENTRY_CONTEXT_ARGS_OUT_EX(WOLFSENTRY_TEST_HPI),
            &config,
            &wolfsentry,
            WOLFSENTRY_INIT_FLAG_NONE));

    remote.sa.sa_family = local.sa.sa_family = AF_INET;
    remote.sa.sa_proto = local.sa.sa_proto = IPPROTO_TCP;
    remote.sa.sa_port = 12345;
    local.sa.sa_port = 443;
    remote.sa.addr_len = local.sa.addr_len = sizeof remote.addr_buf * BITS_PER_BYTE;
    remote.sa.interface = local.sa.interface = 1;
    memcpy(remote.sa.addr,"\0\1\2\3",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\377\376\375\374",sizeof local.addr_buf);

    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    memcpy(remote.sa.addr,"\4\5\6\7",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

#if 0
    puts("table after first 2 inserts:");
    for (struct wolfsentry_route *i = (struct wolfsentry_route *)wolfsentry->routes.header.head;
         i;
         i = (struct wolfsentry_route *)(i->header.next))
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_render(i, stdout));
    putchar('\n');
#endif

    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);

    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));

    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);

    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));

    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_GREENLISTED);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_PENALTYBOXED);
    memcpy(remote.sa.addr,"\3\4\5\6",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_GREENLISTED);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_PENALTYBOXED);
    memcpy(remote.sa.addr,"\2\3\4\5",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);


    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    flags |= WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN;
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_lock_shared(WOLFSENTRY_CONTEXT_ARGS_OUT));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_main_table(WOLFSENTRY_CONTEXT_ARGS_OUT, &main_routes));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_reference(
                                 WOLFSENTRY_CONTEXT_ARGS_OUT,
                                 main_routes,
                                 &remote.sa,
                                 &local.sa,
                                 flags,
                                 0 /* event_label_len */,
                                 0 /* event_label */,
                                 1 /* exact_p */,
                                 &inexact_matches,
                                 &route_ref));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_unlock(WOLFSENTRY_CONTEXT_ARGS_OUT));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_private_data(
                                 WOLFSENTRY_CONTEXT_ARGS_OUT,
                                 route_ref,
                                 (void **)&private_data,
                                 &private_data_size));

    if (private_data_size < PRIVATE_DATA_SIZE) {
        printf("private_data_size is " SIZET_FMT " but expected %d.\n",private_data_size,PRIVATE_DATA_SIZE);
        WOLFSENTRY_ERROR_RETURN(NOT_OK);
    }
    if ((PRIVATE_DATA_ALIGNMENT > 0) && ((uintptr_t)private_data % (uintptr_t)PRIVATE_DATA_ALIGNMENT)) {
        printf("private_data (%p) is not aligned to %d.\n",private_data,PRIVATE_DATA_ALIGNMENT);
        WOLFSENTRY_ERROR_RETURN(NOT_OK);
    }

    {
        byte *i, *i_end;
        for (i = private_data, i_end = private_data + private_data_size; i < i_end; ++i)
            *i = 'x';
    }

#if 0
    puts("table after deleting 4.5.6.7 and inserting 3 more:");
    for (struct wolfsentry_route *i = (struct wolfsentry_route *)wolfsentry->routes.header.head;
         i;
         i = (struct wolfsentry_route *)(i->header.next))
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_render(i, stdout));
#endif

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_drop_reference(WOLFSENTRY_CONTEXT_ARGS_OUT, route_ref, &action_results));
    WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_DEALLOCATED));

    /* now test basic eventless dispatch using exact-match ents in the static table. */

    WOLFSENTRY_CLEAR_ALL_BITS(action_results);

    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);
    memcpy(remote.sa.addr,"\3\4\5\6",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));

    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
    WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
    WOLFSENTRY_EXIT_ON_FALSE(inexact_matches == 0);

    flags |= WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN;
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);

    WOLFSENTRY_EXIT_ON_FALSE(
        WOLFSENTRY_SUCCESS_CODE_IS(
            wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                            &route_id, &inexact_matches, &action_results),
            USED_FALLBACK));
    WOLFSENTRY_EXIT_ON_FALSE(inexact_matches == (WOLFSENTRY_ROUTE_WILDCARD_FLAGS | WOLFSENTRY_ROUTE_FLAG_PARENT_EVENT_WILDCARD));

    memcpy(remote.sa.addr,"\2\3\4\5",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));

    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
    WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
    WOLFSENTRY_EXIT_ON_FALSE(inexact_matches == 0);

    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));

    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
    WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
    WOLFSENTRY_EXIT_ON_FALSE(inexact_matches == 0);

    memcpy(remote.sa.addr,"\0\1\2\3",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\377\376\375\374",sizeof local.addr_buf);
    WOLFSENTRY_EXIT_ON_FALSE(
        WOLFSENTRY_SUCCESS_CODE_IS(
            wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                            &route_id, &inexact_matches, &action_results),
            USED_FALLBACK));
    WOLFSENTRY_EXIT_ON_FALSE(inexact_matches == (WOLFSENTRY_ROUTE_WILDCARD_FLAGS | WOLFSENTRY_ROUTE_FLAG_PARENT_EVENT_WILDCARD));

    flags |= WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN;
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));

    WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
    WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
    WOLFSENTRY_EXIT_ON_FALSE(inexact_matches == 0);


    /* now test eventless dispatch using wildcard/prefix matches in the static table. */


    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_GREENLISTED);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_PENALTYBOXED);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);
    memcpy(remote.sa.addr,"\4\5\6\7",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    for (prefixlen = sizeof remote.addr_buf * BITS_PER_BYTE;
         prefixlen >= 8;
         --prefixlen) {
        remote.sa.addr_len = (wolfsentry_addr_bits_t)prefixlen;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

        remote.sa.addr_len = sizeof remote.addr_buf * BITS_PER_BYTE;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));

        WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
        WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
        WOLFSENTRY_EXIT_ON_TRUE(prefixlen < (int)(sizeof remote.addr_buf * BITS_PER_BYTE) ? ! WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_ADDR_WILDCARD) : WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_ADDR_WILDCARD));

        remote.sa.addr_len = (wolfsentry_addr_bits_t)prefixlen;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
        WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


        local.sa.addr_len = (wolfsentry_addr_bits_t)prefixlen;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

        local.sa.addr_len = sizeof remote.addr_buf * BITS_PER_BYTE;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));

        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
        WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
        WOLFSENTRY_EXIT_ON_TRUE(prefixlen < (int)(sizeof local.addr_buf * BITS_PER_BYTE) ? ! WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_ADDR_WILDCARD) : WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_ADDR_WILDCARD));

        local.sa.addr_len = (wolfsentry_addr_bits_t)prefixlen;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
        WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);

    }


    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    remote_wildcard.sa.sa_port = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_PORT_WILDCARD);

    {
        struct wolfsentry_route *new_route;

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert_and_check_out(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &new_route, &action_results));

        id = wolfsentry_get_object_id(new_route);
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_drop_reference(WOLFSENTRY_CONTEXT_ARGS_OUT, new_route, &action_results));

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                                   &route_id, &inexact_matches, &action_results));
        WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    }

    local.sa.sa_port = 8765;
    WOLFSENTRY_EXIT_ON_FALSE(
        WOLFSENTRY_SUCCESS_CODE_IS(
            wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                            &route_id, &inexact_matches, &action_results),
            USED_FALLBACK));
    WOLFSENTRY_EXIT_ON_FALSE(inexact_matches == (WOLFSENTRY_ROUTE_WILDCARD_FLAGS | WOLFSENTRY_ROUTE_FLAG_PARENT_EVENT_WILDCARD));

    local.sa.sa_port = local_wildcard.sa.sa_port;

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete_by_id(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, route_id, NULL /* event_label */, 0 /* event_label_len */, &action_results));

    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_DEALLOCATED));

    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    local_wildcard.sa.sa_port = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_PORT_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_PORT_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    remote_wildcard.sa.sa_port = local_wildcard.sa.sa_port = 0;
    remote_wildcard.sa.sa_proto = local_wildcard.sa.sa_proto = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_PROTO_WILDCARD);
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_PORT_WILDCARD);
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_PORT_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_PROTO_WILDCARD));
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_PORT_WILDCARD));
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_PORT_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    local_wildcard.sa.addr_len = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_ADDR_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_ADDR_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    remote_wildcard.sa.sa_port = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_PORT_WILDCARD);
    local_wildcard.sa.addr_len = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_ADDR_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_PORT_WILDCARD));
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_ADDR_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    remote_wildcard.sa.addr_len = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_ADDR_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_ADDR_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    local_wildcard.sa.interface = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_LOCAL_INTERFACE_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_LOCAL_INTERFACE_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    remote_wildcard.sa.interface = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_REMOTE_INTERFACE_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_REMOTE_INTERFACE_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);


    local.sa.interface = 2;
    remote_wildcard = remote;
    local_wildcard = local;
    flags_wildcard = flags;

    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_FAMILY_WILDCARD);
    local_wildcard.sa.sa_family = remote_wildcard.sa.sa_family = 0;
    local_wildcard.sa.addr_len = remote_wildcard.sa.addr_len = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_ADDR_WILDCARD);
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_ADDR_WILDCARD);
    remote_wildcard.sa.sa_port = local_wildcard.sa.sa_port = 0;
    remote_wildcard.sa.sa_proto = local_wildcard.sa.sa_proto = 0;
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_PROTO_WILDCARD);
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_REMOTE_PORT_WILDCARD);
    WOLFSENTRY_SET_BITS(flags_wildcard, WOLFSENTRY_ROUTE_FLAG_SA_LOCAL_PORT_WILDCARD);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                           &route_id, &inexact_matches, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(route_id == id);
    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_SA_FAMILY_WILDCARD));
    WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(inexact_matches, WOLFSENTRY_ROUTE_FLAG_LOCAL_INTERFACE_WILDCARD));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);
    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote_wildcard.sa, &local_wildcard.sa, flags_wildcard, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));


#ifndef WOLFSENTRY_NO_STDIO
    {
        wolfsentry_errcode_t ret;
        struct wolfsentry_cursor *cursor;
        struct wolfsentry_route *route;
        struct wolfsentry_route_exports route_exports;
        wolfsentry_hitcount_t n_seen = 0;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_lock_shared(WOLFSENTRY_CONTEXT_ARGS_OUT));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_table_iterate_start(WOLFSENTRY_CONTEXT_ARGS_OUT, main_routes, &cursor));
        for (ret = wolfsentry_route_table_iterate_current(main_routes, cursor, &route);
             ret >= 0;
             ret = wolfsentry_route_table_iterate_next(main_routes, cursor, &route)) {
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_export(WOLFSENTRY_CONTEXT_ARGS_OUT, route, &route_exports));
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_exports_render(WOLFSENTRY_CONTEXT_ARGS_OUT, &route_exports, stdout));
            ++n_seen;
        }
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_table_iterate_end(WOLFSENTRY_CONTEXT_ARGS_OUT, main_routes, &cursor));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_unlock(WOLFSENTRY_CONTEXT_ARGS_OUT));
        WOLFSENTRY_EXIT_ON_FALSE(n_seen == wolfsentry->routes->header.n_ents);
    }
#endif

    remote.sa.sa_family = local.sa.sa_family = AF_INET;
    remote.sa.sa_proto = local.sa.sa_proto = IPPROTO_TCP;
    remote.sa.sa_port = 12345;
    local.sa.sa_port = 443;
    remote.sa.addr_len = local.sa.addr_len = sizeof remote.addr_buf * BITS_PER_BYTE;
    remote.sa.interface = local.sa.interface = 1;
    memcpy(remote.sa.addr,"\0\1\2\3",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\377\376\375\374",sizeof local.addr_buf);

    WOLFSENTRY_CLEAR_ALL_BITS(flags);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_TCPLIKE_PORT_NUMBERS|WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);
    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));

    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_GREENLISTED);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_PENALTYBOXED);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);
    memcpy(remote.sa.addr,"\2\3\4\5",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);
    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));

    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);
    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));

    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_GREENLISTED);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_PENALTYBOXED);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN);
    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_DIRECTION_OUT);
    memcpy(remote.sa.addr,"\3\4\5\6",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));
    WOLFSENTRY_EXIT_ON_FALSE(n_deleted == 1);
    WOLFSENTRY_EXIT_ON_SUCCESS(wolfsentry_route_delete(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &action_results, &n_deleted));

    WOLFSENTRY_EXIT_ON_FALSE(wolfsentry->routes->header.n_ents == 0);


    /* finally, test config.derogatory_threshold_for_penaltybox */

    WOLFSENTRY_SET_BITS(flags, WOLFSENTRY_ROUTE_FLAG_GREENLISTED);
    WOLFSENTRY_CLEAR_BITS(flags, WOLFSENTRY_ROUTE_FLAG_PENALTYBOXED);
    memcpy(remote.sa.addr,"\3\4\5\6",sizeof remote.addr_buf);
    memcpy(local.sa.addr,"\373\372\371\370",sizeof local.addr_buf);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_insert(WOLFSENTRY_CONTEXT_ARGS_OUT, NULL /* caller_arg */, &remote.sa, &local.sa, flags, 0 /* event_label_len */, 0 /* event_label */, &id, &action_results));

    {
        wolfsentry_hitcount_t i;
        for (i=1; i <= config.derogatory_threshold_for_penaltybox + 1; ++i) {
            WOLFSENTRY_CLEAR_ALL_BITS(action_results);
            WOLFSENTRY_SET_BITS(action_results, WOLFSENTRY_ACTION_RES_DEROGATORY);
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch_with_inited_result(WOLFSENTRY_CONTEXT_ARGS_OUT, &remote.sa, &local.sa, flags, NULL /* event_label */, 0 /* event_label_len */, NULL /* caller_arg */,
                                                                                          &route_id, &inexact_matches, &action_results));
            if (i == config.derogatory_threshold_for_penaltybox) {
                WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
                WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
                printf("sleeping for %ld seconds to test penaltybox timeout...", (long int)(config.penaltybox_duration + 1));
                fflush(stdout);
                sleep((unsigned int)config.penaltybox_duration + 1);
                printf(" done.\n");
                fflush(stdout);
            } else {
                WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
                WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
            }
        }
    }



    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_reference(
                                 WOLFSENTRY_CONTEXT_ARGS_OUT,
                                 main_routes,
                                 &remote.sa,
                                 &local.sa,
                                 flags,
                                 0 /* event_label_len */,
                                 0 /* event_label */,
                                 1 /* exact_p */,
                                 &inexact_matches,
                                 &route_ref));


    {
        int old_derogatory_count;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_reset_derogatory_count(
                                       WOLFSENTRY_CONTEXT_ARGS_OUT,
                                       route_ref,
                                       &old_derogatory_count));
        /* 1 left from final iteration above. */
        WOLFSENTRY_EXIT_ON_FALSE(old_derogatory_count == 1);
    }

    {
        int new_derogatory_count;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_increment_derogatory_count(
                                       WOLFSENTRY_CONTEXT_ARGS_OUT,
                                       route_ref,
                                       123,
                                       &new_derogatory_count));
        WOLFSENTRY_EXIT_ON_FALSE(new_derogatory_count == 123);
    }

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        OVERFLOW_AVERTED,
        wolfsentry_route_increment_derogatory_count(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            route_ref,
            -124,
            NULL));

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        OVERFLOW_AVERTED,
        wolfsentry_route_increment_derogatory_count(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            route_ref,
            65536 - 123,
            NULL));

    {
        int new_derogatory_count;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_increment_derogatory_count(
                                       WOLFSENTRY_CONTEXT_ARGS_OUT,
                                       route_ref,
                                       1,
                                       &new_derogatory_count));
        WOLFSENTRY_EXIT_ON_FALSE(new_derogatory_count == 124);
    }

    {
        int old_commendable_count;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_reset_commendable_count(
                                       WOLFSENTRY_CONTEXT_ARGS_OUT,
                                       route_ref,
                                       &old_commendable_count));
        WOLFSENTRY_EXIT_ON_FALSE(old_commendable_count == 0);
    }

    {
        int new_commendable_count;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_increment_commendable_count(
                                       WOLFSENTRY_CONTEXT_ARGS_OUT,
                                       route_ref,
                                       123,
                                       &new_commendable_count));
        WOLFSENTRY_EXIT_ON_FALSE(new_commendable_count == 123);
    }

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        OVERFLOW_AVERTED,
        wolfsentry_route_increment_commendable_count(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            route_ref,
            -124,
            NULL));

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        OVERFLOW_AVERTED,
        wolfsentry_route_increment_commendable_count(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            route_ref,
            65536 - 123,
            NULL));

    {
        int new_commendable_count;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_increment_commendable_count(
                                       WOLFSENTRY_CONTEXT_ARGS_OUT,
                                       route_ref,
                                       1,
                                       &new_commendable_count));
        WOLFSENTRY_EXIT_ON_FALSE(new_commendable_count == 124);
    }

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_drop_reference(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   route_ref,
                                   NULL /* action_results */));

    /* leave the route in the table, to be cleaned up by wolfsentry_shutdown(). */

    printf("all subtests succeeded -- %d distinct ents inserted and deleted.\n",wolfsentry->mk_id_cb_state.id_counter);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_shutdown(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&wolfsentry)));

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_RETURN_OK;
}

#undef PRIVATE_DATA_SIZE
#undef PRIVATE_DATA_ALIGNMENT

#endif /* TEST_STATIC_ROUTES */

#ifdef TEST_DYNAMIC_RULES

#define PRIVATE_DATA_SIZE 32
#define PRIVATE_DATA_ALIGNMENT 8

static wolfsentry_errcode_t wolfsentry_action_dummy_callback(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    const struct wolfsentry_action *action,
    void *handler_context,
    void *caller_arg,
    const struct wolfsentry_event *event,
    wolfsentry_action_type_t action_type,
    const struct wolfsentry_route *target_route,
    struct wolfsentry_route_table *route_table,
    struct wolfsentry_route *rule_route,
    wolfsentry_action_res_t *action_results)
{
    WOLFSENTRY_CONTEXT_ARGS_NOT_USED;
    (void)action;
    (void)handler_context;
    (void)caller_arg;
    (void)event;
    (void)action_type;
    (void)target_route;
    (void)route_table;
    (void)rule_route;
    (void)action_results;

    WOLFSENTRY_RETURN_OK;
}


static int test_dynamic_rules (void) {

    struct wolfsentry_context *wolfsentry;
#if 0
    wolfsentry_action_res_t action_results;
    int n_deleted;
    int ret;
    struct {
        struct wolfsentry_sockaddr sa;
        byte addr_buf[4];
    } src, dst;
#endif

    wolfsentry_ent_id_t id;

#ifdef WOLFSENTRY_HAVE_DESIGNATED_INITIALIZERS
    struct wolfsentry_eventconfig config = { .route_private_data_size = PRIVATE_DATA_SIZE, .route_private_data_alignment = PRIVATE_DATA_ALIGNMENT, .max_connection_count = 10 };
    struct wolfsentry_eventconfig config2 = { .route_private_data_size = PRIVATE_DATA_SIZE * 2, .route_private_data_alignment = PRIVATE_DATA_ALIGNMENT * 2, .max_connection_count = 15 };
#else
    struct wolfsentry_eventconfig config = { PRIVATE_DATA_SIZE, PRIVATE_DATA_ALIGNMENT, 10, 0, 0, 0, 0 };
    struct wolfsentry_eventconfig config2 = { PRIVATE_DATA_SIZE * 2, PRIVATE_DATA_ALIGNMENT * 2, 15, 0, 0, 0, 0 };
#endif

    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_init_ex(
            wolfsentry_build_settings,
            WOLFSENTRY_CONTEXT_ARGS_OUT_EX(WOLFSENTRY_TEST_HPI),
            &config,
            &wolfsentry,
            WOLFSENTRY_INIT_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "connect",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    /* track port scanning */
    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "connection_refused",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "disconnect",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "authentication_succeeded",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "authentication_failed",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "negotiation_abandoned",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "insertion_side_effect_demo",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "match_side_effect_demo",
            -1 /* label_len */,
            10,
            NULL /* config */,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "deletion_side_effect_demo",
            -1 /* label_len */,
            10,
            &config2,
            WOLFSENTRY_EVENT_FLAG_NONE,
            &id));

    {
        struct wolfsentry_eventconfig eventconfig;

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_event_get_config(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "deletion_side_effect_demo",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &eventconfig));

        WOLFSENTRY_EXIT_ON_FALSE((eventconfig.route_private_data_size == config2.route_private_data_size) &&
                                 (eventconfig.route_private_data_alignment == config2.route_private_data_alignment) &&
                                 (eventconfig.max_connection_count == config2.max_connection_count));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_event_update_config(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "deletion_side_effect_demo",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &config));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_event_get_config(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "deletion_side_effect_demo",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &eventconfig));

        WOLFSENTRY_EXIT_ON_FALSE((eventconfig.route_private_data_size == config.route_private_data_size) &&
                                 (eventconfig.route_private_data_alignment == config.route_private_data_alignment) &&
                                 (eventconfig.max_connection_count == config.max_connection_count));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_event_delete(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "deletion_side_effect_demo",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                NULL /* action_results */));

        WOLFSENTRY_EXIT_ON_SUCCESS(
            wolfsentry_event_delete(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "deletion_side_effect_demo",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                NULL /* action_results */));
    }

#if 0
int wolfsentry_event_set_subevent(
    struct wolfsentry_context *WOLFSENTRY_CONTEXT_ARGS_OUT,
    const char *event_label,
    int event_label_len,
    wolfsentry_action_type_t subevent_type,
    const char *subevent_label,
    int subevent_label_len);
#endif

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "insert_always",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "insert_alway",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    {
        static char too_long_label[WOLFSENTRY_MAX_LABEL_BYTES + 2];
        memset(too_long_label, 'x', sizeof too_long_label - 1);

        too_long_label[sizeof too_long_label - 1] = 0;

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            STRING_ARG_TOO_LONG,
            wolfsentry_action_insert(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                -1 /* label_len */,
                WOLFSENTRY_ACTION_FLAG_NONE,
                wolfsentry_action_dummy_callback,
                NULL /* handler_context */,
                &id));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            STRING_ARG_TOO_LONG,
            wolfsentry_action_insert(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                sizeof too_long_label - 1,
                WOLFSENTRY_ACTION_FLAG_NONE,
                wolfsentry_action_dummy_callback,
                NULL /* handler_context */,
                &id));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_action_insert(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                sizeof too_long_label - 2,
                WOLFSENTRY_ACTION_FLAG_NONE,
                wolfsentry_action_dummy_callback,
                NULL /* handler_context */,
                &id));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            STRING_ARG_TOO_LONG,
            wolfsentry_action_delete(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                sizeof too_long_label - 1,
                NULL /* action_results */));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            STRING_ARG_TOO_LONG,
            wolfsentry_action_delete(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                -1,
                NULL /* action_results */));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            INVALID_ARG,
            wolfsentry_action_delete(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                NULL,
                -1,
                NULL /* action_results */));

        too_long_label[sizeof too_long_label - 2] = 0;

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            ITEM_ALREADY_PRESENT,
            wolfsentry_action_insert(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                -1 /* label_len */,
                WOLFSENTRY_ACTION_FLAG_NONE,
                wolfsentry_action_dummy_callback,
                NULL /* handler_context */,
                &id));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            INVALID_ARG,
            wolfsentry_action_insert(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                NULL,
                -1 /* label_len */,
                WOLFSENTRY_ACTION_FLAG_NONE,
                wolfsentry_action_dummy_callback,
                NULL /* handler_context */,
                &id));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            INVALID_ARG,
            wolfsentry_action_insert(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                0 /* label_len */,
                WOLFSENTRY_ACTION_FLAG_NONE,
                wolfsentry_action_dummy_callback,
                NULL /* handler_context */,
                &id));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_action_delete(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                too_long_label,
                -1 /* label_len */,
                NULL /* action_results */));
    }

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "set_connect_wildcards",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "set_connectionreset_wildcards",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "increment_derogatory",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "increment_commendable",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "check_counts",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    {
        struct wolfsentry_action *action;
        wolfsentry_action_flags_t flags;

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            ITEM_NOT_FOUND,
            wolfsentry_action_get_reference(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "checXXXounts",
                -1 /* label_len */,
                &action));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            INVALID_ARG,
            wolfsentry_action_get_reference(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "checXXXounts",
                0 /* label_len */,
                &action));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
            STRING_ARG_TOO_LONG,
            wolfsentry_action_get_reference(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "checXXXounts",
                WOLFSENTRY_MAX_LABEL_BYTES + 1 /* label_len */,
                &action));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_action_get_reference(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "check_counts",
                -1 /* label_len */,
                &action));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_action_get_flags(
                action,
                &flags));
        WOLFSENTRY_EXIT_ON_FALSE(flags == WOLFSENTRY_ACTION_FLAG_NONE);

        {
            wolfsentry_action_flags_t flags_before, flags_after;
            WOLFSENTRY_EXIT_ON_FAILURE(
                wolfsentry_action_update_flags(
                    action,
                    WOLFSENTRY_ACTION_FLAG_DISABLED,
                    WOLFSENTRY_ACTION_FLAG_NONE,
                    &flags_before,
                    &flags_after));
            WOLFSENTRY_EXIT_ON_FALSE(flags_before == WOLFSENTRY_ACTION_FLAG_NONE);
            WOLFSENTRY_EXIT_ON_FALSE(flags_after == WOLFSENTRY_ACTION_FLAG_DISABLED);
        }

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_action_get_flags(
                action,
                &flags));
        WOLFSENTRY_EXIT_ON_FALSE(flags == WOLFSENTRY_ACTION_FLAG_DISABLED);

        {
            wolfsentry_action_flags_t flags_before, flags_after;
            WOLFSENTRY_EXIT_ON_FAILURE(
                wolfsentry_action_update_flags(
                    action,
                    WOLFSENTRY_ACTION_FLAG_NONE,
                    WOLFSENTRY_ACTION_FLAG_DISABLED,
                    &flags_before,
                    &flags_after));
            WOLFSENTRY_EXIT_ON_FALSE(flags_before == WOLFSENTRY_ACTION_FLAG_DISABLED);
            WOLFSENTRY_EXIT_ON_FALSE(flags_after == WOLFSENTRY_ACTION_FLAG_NONE);
        }

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_action_get_flags(
                action,
                &flags));
        WOLFSENTRY_EXIT_ON_FALSE(flags == WOLFSENTRY_ACTION_FLAG_NONE);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_action_drop_reference(WOLFSENTRY_CONTEXT_ARGS_OUT, action, NULL));
    }

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "add_to_greenlist",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_action_insert(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "del_from_greenlist",
            -1 /* label_len */,
            WOLFSENTRY_ACTION_FLAG_NONE,
            wolfsentry_action_dummy_callback,
            NULL /* handler_context */,
            &id));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_action_append(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "match_side_effect_demo",
            -1,
            "del_from_greenlist",
            -1));

    WOLFSENTRY_EXIT_ON_SUCCESS(
        wolfsentry_event_action_prepend(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "match_side_effect_demo",
            -1,
            "del_from_greenlist",
            -1));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_action_prepend(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "match_side_effect_demo",
            -1,
            "add_to_greenlist",
            -1));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_action_insert_after(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "match_side_effect_demo",
            -1,
            "check_counts",
            -1,
            "add_to_greenlist",
            -1));

    {
        struct wolfsentry_action_list_ent *cursor;
        const char *action_label;
        int action_label_len;
        static const char *labels[] = { "add_to_greenlist", "check_counts", "del_from_greenlist" };
        int label_i = 0;

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_lock_shared(WOLFSENTRY_CONTEXT_ARGS_OUT));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_event_action_list_start(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "match_side_effect_demo",
                -1,
                &cursor));

        while (wolfsentry_event_action_list_next(
                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                   &cursor,
                   &action_label,
                   &action_label_len) >= 0) {
            WOLFSENTRY_EXIT_ON_TRUE((size_t)label_i >= sizeof labels / sizeof labels[0]);
            WOLFSENTRY_EXIT_ON_FALSE(strcmp(action_label, labels[label_i++]) == 0);
        }

        WOLFSENTRY_EXIT_ON_FALSE(label_i == sizeof labels / sizeof labels[0]);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_event_action_list_done(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &cursor));

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_unlock(WOLFSENTRY_CONTEXT_ARGS_OUT));
    }

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_event_action_delete(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "match_side_effect_demo",
            -1,
            "del_from_greenlist",
            -1));


    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_shutdown(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&wolfsentry)));

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_RETURN_OK;
}

#undef PRIVATE_DATA_SIZE
#undef PRIVATE_DATA_ALIGNMENT

#endif /* TEST_DYNAMIC_RULES */

#ifdef TEST_USER_VALUES

#include <math.h>

static wolfsentry_errcode_t test_kv_validator(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    struct wolfsentry_kv_pair *kv)
{
    WOLFSENTRY_CONTEXT_ARGS_NOT_USED;
    switch (WOLFSENTRY_KV_TYPE(kv)) {
    case WOLFSENTRY_KV_NONE:
    case WOLFSENTRY_KV_NULL:
    case WOLFSENTRY_KV_TRUE:
    case WOLFSENTRY_KV_FALSE:
        WOLFSENTRY_RETURN_OK;
    case WOLFSENTRY_KV_UINT:
        if (WOLFSENTRY_KV_V_UINT(kv) == 12345678UL)
            WOLFSENTRY_ERROR_RETURN(BAD_VALUE);
        else
            WOLFSENTRY_RETURN_OK;
    case WOLFSENTRY_KV_SINT:
        if (WOLFSENTRY_KV_V_SINT(kv) == -12345678L)
            WOLFSENTRY_ERROR_RETURN(BAD_VALUE);
        else
            WOLFSENTRY_RETURN_OK;
    case WOLFSENTRY_KV_FLOAT:
        if (WOLFSENTRY_KV_V_FLOAT(kv) > 100.0)
            WOLFSENTRY_ERROR_RETURN(BAD_VALUE);
        else
            WOLFSENTRY_RETURN_OK;
    case WOLFSENTRY_KV_STRING:
        if (WOLFSENTRY_KV_V_STRING_LEN(kv) != 8)
            WOLFSENTRY_RETURN_OK;
        if (strncmp(WOLFSENTRY_KV_V_STRING(kv), "deadbeef", WOLFSENTRY_KV_V_STRING_LEN(kv)) == 0)
            WOLFSENTRY_ERROR_RETURN(BAD_VALUE);
        else
            WOLFSENTRY_RETURN_OK;
    case WOLFSENTRY_KV_BYTES:
        if (WOLFSENTRY_KV_V_BYTES_LEN(kv) != 10)
            WOLFSENTRY_RETURN_OK;
        if (memcmp(WOLFSENTRY_KV_V_STRING(kv), "abcdefghij", WOLFSENTRY_KV_V_BYTES_LEN(kv)) == 0)
            WOLFSENTRY_ERROR_RETURN(BAD_VALUE);
        else
            WOLFSENTRY_RETURN_OK;
    }
    WOLFSENTRY_ERROR_RETURN(WRONG_TYPE);
}

static int test_user_values (void) {
    struct wolfsentry_context *wolfsentry;
    wolfsentry_action_res_t action_results;

    wolfsentry_kv_type_t kv_type;
    struct wolfsentry_kv_pair_internal *kv_ref;

    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_init_ex(
            wolfsentry_build_settings,
            WOLFSENTRY_CONTEXT_ARGS_OUT_EX(WOLFSENTRY_TEST_HPI),
            NULL /* config */,
            &wolfsentry,
            WOLFSENTRY_INIT_FLAG_NONE));

    action_results = WOLFSENTRY_ACTION_RES_NONE;
    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_user_value_set_validator(WOLFSENTRY_CONTEXT_ARGS_OUT, test_kv_validator, &action_results));
    WOLFSENTRY_EXIT_ON_FALSE(action_results == WOLFSENTRY_ACTION_RES_NONE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_store_null(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_null",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            0));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_store_bool(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            WOLFSENTRY_KV_TRUE,
            0));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_get_bool(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            &kv_type));

    WOLFSENTRY_EXIT_ON_FALSE(kv_type == WOLFSENTRY_KV_TRUE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_store_bool(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            WOLFSENTRY_KV_FALSE,
            1));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_get_bool(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            &kv_type));

    WOLFSENTRY_EXIT_ON_FALSE(kv_type == WOLFSENTRY_KV_FALSE);

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        WRONG_TYPE,
        wolfsentry_user_value_store_bool(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            WOLFSENTRY_KV_NONE,
            1));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_get_type(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            &kv_type));

    WOLFSENTRY_EXIT_ON_FALSE(kv_type == WOLFSENTRY_KV_FALSE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_get_type(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            strlen("test_bool"),
            &kv_type));

    WOLFSENTRY_EXIT_ON_FALSE(kv_type == WOLFSENTRY_KV_FALSE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_delete(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED));

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        ITEM_NOT_FOUND,
        wolfsentry_user_value_get_type(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_bool",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            &kv_type));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_store_uint(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_uint",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            123UL,
            0));

    {
        uint64_t value = 0;
        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_uint(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_uint",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value));

        WOLFSENTRY_EXIT_ON_FALSE(value == 123UL);
    }

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        BAD_VALUE,
        wolfsentry_user_value_store_uint(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "bad_uint",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            12345678UL,
            0));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_store_sint(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_sint",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            -123L,
            0));

    {
        int64_t value = 0;
        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_sint(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_sint",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value));

        WOLFSENTRY_EXIT_ON_FALSE(value == -123L);
    }

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        BAD_VALUE,
        wolfsentry_user_value_store_sint(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "bad_sint",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            -12345678L,
            0));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_user_value_store_double(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "test_float",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            1.234,
            0));

    {
        double value = 0.0;
        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_float(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_float",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value));

        WOLFSENTRY_EXIT_ON_FALSE(fabs(value - 1.234) < 0.000001);
    }

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        BAD_VALUE,
        wolfsentry_user_value_store_double(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "bad_float",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            123.45678,
            0));

    {
        static const char test_string[] = "abc123";
        const char *value = NULL;
        int value_len = -1;
        int mutable = -1;
        wolfsentry_errcode_t ret;

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_store_string(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                test_string,
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                0));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_mutability(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &mutable));

        WOLFSENTRY_EXIT_ON_FALSE(mutable == 1);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_set_mutability(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                0));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_mutability(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &mutable));

        WOLFSENTRY_EXIT_ON_FALSE(mutable == 0);

        WOLFSENTRY_EXIT_ON_SUCCESS(
            ret = wolfsentry_user_value_delete(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED));

        WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(NOT_PERMITTED, ret);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_string(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value,
                &value_len,
                &kv_ref));

        WOLFSENTRY_EXIT_ON_FALSE(value_len == (int)strlen(test_string));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(value, test_string) == 0);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_release_record(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &kv_ref));

    }

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        BAD_VALUE,
        wolfsentry_user_value_store_string(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "bad_string",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            "deadbeef",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            0));

    {
        static const byte test_bytes[] = { 0, 1, 2, 3, 4 };
        const byte *value = NULL;
        int value_len = -1;

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_store_bytes(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_bytes",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                test_bytes,
                sizeof test_bytes,
                0));

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_bytes(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "test_bytes",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value,
                &value_len,
                &kv_ref));

        WOLFSENTRY_EXIT_ON_FALSE(value_len == (int)sizeof test_bytes);
        WOLFSENTRY_EXIT_ON_FALSE(memcmp(value, test_bytes, (size_t)value_len) == 0);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_release_record(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &kv_ref));
    }

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        BAD_VALUE,
        wolfsentry_user_value_store_bytes(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "bad_bytes",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            (const byte *)"abcdefghij",
            10,
            0));

#ifndef WOLFSENTRY_NO_STDIO
    {
        wolfsentry_errcode_t ret;
        struct wolfsentry_cursor *cursor;
        const struct wolfsentry_kv_pair *kv_exports;
        const char *val_type;
        char val_buf[256];
        int val_buf_space;
        wolfsentry_hitcount_t n_seen = 0;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_lock_shared(WOLFSENTRY_CONTEXT_ARGS_OUT));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_user_values_iterate_start(WOLFSENTRY_CONTEXT_ARGS_OUT, &cursor));
        for (ret = wolfsentry_user_values_iterate_current(WOLFSENTRY_CONTEXT_ARGS_OUT, cursor, &kv_ref);
             ret >= 0;
             ret = wolfsentry_user_values_iterate_next(WOLFSENTRY_CONTEXT_ARGS_OUT, cursor, &kv_ref)) {
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_kv_pair_export(WOLFSENTRY_CONTEXT_ARGS_OUT, kv_ref, &kv_exports));
            val_buf_space = sizeof val_buf;
            if (wolfsentry_kv_type_to_string(WOLFSENTRY_KV_TYPE(kv_exports), &val_type) < 0)
                val_type = "?";
            if (wolfsentry_kv_render_value(WOLFSENTRY_CONTEXT_ARGS_OUT, kv_exports, val_buf, &val_buf_space) < 0)
                strcpy(val_buf,"?");
            printf("{ \"%.*s\" : { \"type\" : \"%s\", \"value\" : %s } }\n",
                   (int)WOLFSENTRY_KV_KEY_LEN(kv_exports),
                   WOLFSENTRY_KV_KEY(kv_exports),
                   val_type,
                   val_buf);
            ++n_seen;
        }
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_user_values_iterate_end(WOLFSENTRY_CONTEXT_ARGS_OUT, &cursor));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_unlock(WOLFSENTRY_CONTEXT_ARGS_OUT));
        WOLFSENTRY_EXIT_ON_FALSE(n_seen == wolfsentry->user_values->header.n_ents);
        WOLFSENTRY_EXIT_ON_FALSE(n_seen == 6);
    }
#endif

    {
        static const struct {
            const char *q;
            const char *a;
        } base64_qna[] = {
            { "", "" },
            { "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu", "Many hands make light work." },
            { "bGlnaHQgd29yay4=", "light work." },
            { "bGlnaHQgd29yay4", "light work." },
            { "bGlnaHQgd29yaw==", "light work" },
            { "bGlnaHQgd29yaw", "light work" },
            { "bGlnaHQgd29y", "light wor" },
            { "bGlnaHQgd28=", "light wo" },
            { "bGlnaHQgdw==", "light w" },
            { "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXpBQkNERUZHSElKS0xNTk9QUVJTVFVWV"
                "1hZWjAxMjM0NTY3ODkhQCMkJV4mKigpXy0rPXxcYH5bXXt9OzonIiw8Lj4vPw==",
              "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
                                          "!@#$%^&*()_-+=|\\`~[]{};:'\",<.>/?" },
            { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/+",
              "\x69\xb7\x1d\x79\xf8\x21\x8a\x39\x25\x9a\x7a\x29\xaa\xbb\x2d\xba"
              "\xfc\x31\xcb\x30\x01\x08\x31\x05\x18\x72\x09\x28\xb3\x0d\x38\xf4"
              "\x11\x49\x35\x15\x59\x76\x19\xd3\x5d\xb7\xe3\x9e\xbb\xf3\xdf\xfe"
            }
        };

        int i;
        byte outbuf[256];
        size_t outbuf_spc;

        for (i=0; i < (int)(sizeof base64_qna / sizeof base64_qna[0]); ++i) {
            outbuf_spc = sizeof outbuf;
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_base64_decode(base64_qna[i].q, strlen(base64_qna[i].q), outbuf, &outbuf_spc, 0 /* ignore_junk_p */));
            WOLFSENTRY_EXIT_ON_FALSE((outbuf_spc == strlen(base64_qna[i].a)) && (memcmp(outbuf, base64_qna[i].a, outbuf_spc) == 0));
        }
    }

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_shutdown(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&wolfsentry)));

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_RETURN_OK;
}

#endif /* TEST_USER_VALUES */

#if defined(TEST_USER_ADDR_FAMILIES) || defined(TEST_JSON)

static wolfsentry_errcode_t my_addr_family_parser(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    const char *addr_text,
    const int addr_text_len,
    byte *addr_internal,
    wolfsentry_addr_bits_t *addr_internal_len)
{
    uint32_t a[3];
    char abuf[32];
    int n_octets, parsed_len = 0, i;

    WOLFSENTRY_CONTEXT_ARGS_NOT_USED;

    if (snprintf(abuf,sizeof abuf,"%.*s",addr_text_len,addr_text) >= (int)sizeof abuf)
        WOLFSENTRY_ERROR_RETURN(STRING_ARG_TOO_LONG);
    if ((n_octets = sscanf(abuf,"%o/%o/%o%n",&a[0],&a[1],&a[2],&parsed_len)) < 1)
        WOLFSENTRY_ERROR_RETURN(CONFIG_INVALID_VALUE);
    if (parsed_len != addr_text_len) {
        if ((n_octets = sscanf(abuf,"%o/%o/%n",&a[0],&a[1],&parsed_len)) < 1)
            WOLFSENTRY_ERROR_RETURN(CONFIG_INVALID_VALUE);
    }
    if (parsed_len != addr_text_len) {
        if ((n_octets = sscanf(abuf,"%o/%n",&a[0],&parsed_len)) < 1)
            WOLFSENTRY_ERROR_RETURN(CONFIG_INVALID_VALUE);
    }
    if (parsed_len != addr_text_len)
        WOLFSENTRY_ERROR_RETURN(CONFIG_INVALID_VALUE);
    for (i = 0; i < n_octets; ++i) {
        if (a[i] > MAX_UINT_OF(byte))
            WOLFSENTRY_ERROR_RETURN(CONFIG_INVALID_VALUE);
        addr_internal[i] = (byte)a[i];
    }
    *addr_internal_len = (wolfsentry_addr_bits_t)(n_octets * 8);
    WOLFSENTRY_RETURN_OK;
}

static wolfsentry_errcode_t my_addr_family_formatter(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    const byte *addr_internal,
    const unsigned int addr_internal_len,
    char *addr_text,
    int *addr_text_len)
{
    int out_len;
    int ret;

    WOLFSENTRY_CONTEXT_ARGS_NOT_USED;

    if (addr_internal_len <= 8)
        out_len = snprintf(addr_text, (size_t)*addr_text_len, "%03o/",(unsigned int)addr_internal[0]);
    else if (addr_internal_len <= 16)
        out_len = snprintf(addr_text, (size_t)*addr_text_len, "%03o/%03o/",(unsigned int)addr_internal[0],(unsigned int)addr_internal[1]);
    else
        out_len = snprintf(addr_text, (size_t)*addr_text_len, "%03o/%03o/%03o",(unsigned int)addr_internal[0],(unsigned int)addr_internal[1],(unsigned int)addr_internal[2]);
    if (out_len >= *addr_text_len)
        ret = WOLFSENTRY_ERROR_ENCODE(BUFFER_TOO_SMALL);
    else
        ret = WOLFSENTRY_ERROR_ENCODE(OK);
    *addr_text_len = out_len;
    WOLFSENTRY_ERROR_RERETURN(ret);
}

#endif /* TEST_USER_ADDR_FAMILIES || TEST_JSON */

#ifdef TEST_USER_ADDR_FAMILIES

static int test_user_addr_families (void) {

    struct wolfsentry_context *wolfsentry;
    wolfsentry_action_res_t action_results;

    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_init_ex(
            wolfsentry_build_settings,
            WOLFSENTRY_CONTEXT_ARGS_OUT_EX(WOLFSENTRY_TEST_HPI),
            NULL /* config */,
            &wolfsentry,
            WOLFSENTRY_INIT_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_addr_family_handler_install(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            WOLFSENTRY_AF_USER_OFFSET,
            "my_AF",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            my_addr_family_parser,
            my_addr_family_formatter,
            24 /* max_addr_bits */));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_addr_family_handler_remove_bynumber(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            WOLFSENTRY_AF_USER_OFFSET,
            &action_results));

    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_DEALLOCATED));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_addr_family_handler_install(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            WOLFSENTRY_AF_USER_OFFSET,
            "my_AF",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            my_addr_family_parser,
            my_addr_family_formatter,
            24 /* max_addr_bits */));

    action_results = 0;

    /* exercise the plugins to disambiguate failures in the plugins from
     * JSON-specific failures.
     */
    {
        byte addr_internal[3];
        wolfsentry_addr_bits_t addr_internal_len;
        char addr_text[13];
        int addr_text_len;

        addr_internal_len = (wolfsentry_addr_bits_t)(sizeof addr_internal * 8);
        WOLFSENTRY_EXIT_ON_FAILURE(
            my_addr_family_parser(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "377/377/377",
                strlen("377/377/377"),
                addr_internal,
                &addr_internal_len));
        WOLFSENTRY_EXIT_ON_FALSE(addr_internal_len == (wolfsentry_addr_bits_t)(sizeof addr_internal * 8));
        WOLFSENTRY_EXIT_ON_FALSE(memcmp(addr_internal, "\377\377\377", sizeof addr_internal) == 0);

        addr_text_len = (int)sizeof addr_text;
        WOLFSENTRY_EXIT_ON_FAILURE(
            my_addr_family_formatter(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                addr_internal,
                (int)(sizeof addr_internal * 8),
                addr_text,
                &addr_text_len));
        WOLFSENTRY_EXIT_ON_FALSE(addr_text_len == strlen("377/377/377"));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(addr_text, "377/377/377") == 0);

        addr_internal_len = (wolfsentry_addr_bits_t)(sizeof addr_internal * 8);
        WOLFSENTRY_EXIT_ON_FAILURE(
            my_addr_family_parser(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "0/0/0",
                strlen("0/0/0"),
                addr_internal,
                &addr_internal_len));
        WOLFSENTRY_EXIT_ON_FALSE(addr_internal_len == (wolfsentry_addr_bits_t)(sizeof addr_internal * 8));
        WOLFSENTRY_EXIT_ON_FALSE(memcmp(addr_internal, "\0\0\0", sizeof addr_internal) == 0);

        addr_internal_len = (wolfsentry_addr_bits_t)(sizeof addr_internal * 8);
        WOLFSENTRY_EXIT_ON_FAILURE(
            my_addr_family_parser(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "377/377/",
                strlen("377/377/"),
                addr_internal,
                &addr_internal_len));
        WOLFSENTRY_EXIT_ON_FALSE(addr_internal_len == 16);
        WOLFSENTRY_EXIT_ON_FALSE(memcmp(addr_internal, "\377\377", 2) == 0);

        addr_text_len = (int)sizeof addr_text;
        WOLFSENTRY_EXIT_ON_FAILURE(
            my_addr_family_formatter(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                addr_internal,
                16,
                addr_text,
                &addr_text_len));
        WOLFSENTRY_EXIT_ON_FALSE(addr_text_len == strlen("377/377/"));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(addr_text, "377/377/") == 0);

        addr_internal_len = (wolfsentry_addr_bits_t)(sizeof addr_internal * 8);
        WOLFSENTRY_EXIT_ON_FAILURE(
            my_addr_family_parser(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "377/",
                strlen("377/"),
                addr_internal,
                &addr_internal_len));
        WOLFSENTRY_EXIT_ON_FALSE(addr_internal_len == 8);
        WOLFSENTRY_EXIT_ON_FALSE(memcmp(addr_internal, "\377", 1) == 0);

        addr_text_len = (int)sizeof addr_text;
        WOLFSENTRY_EXIT_ON_FAILURE(
            my_addr_family_formatter(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                addr_internal,
                8,
                addr_text,
                &addr_text_len));
        WOLFSENTRY_EXIT_ON_FALSE(addr_text_len == strlen("377/"));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(addr_text, "377/") == 0);
    }

#ifdef WOLFSENTRY_PROTOCOL_NAMES

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_addr_family_handler_remove_byname(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "my_AF",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            &action_results));

    WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_DEALLOCATED));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_addr_family_handler_install(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            WOLFSENTRY_AF_USER_OFFSET,
            "my_AF",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            my_addr_family_parser,
            my_addr_family_formatter,
            24 /* max_addr_bits */));

    {
        wolfsentry_addr_family_t family_number;

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_addr_family_pton(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "my_AF",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            &family_number));
    WOLFSENTRY_EXIT_ON_FALSE(family_number == WOLFSENTRY_AF_USER_OFFSET);

    WOLFSENTRY_EXIT_UNLESS_EXPECTED_FAILURE(
        ITEM_NOT_FOUND,
        wolfsentry_addr_family_pton(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            "no_such_AF",
            WOLFSENTRY_LENGTH_NULL_TERMINATED, &family_number));
    }

    {
        struct wolfsentry_addr_family_bynumber *addr_family = NULL;
        const char *family_name;

        WOLFSENTRY_EXIT_ON_FAILURE(
            (wolfsentry_addr_family_ntop(
                  WOLFSENTRY_CONTEXT_ARGS_OUT,
                  WOLFSENTRY_AF_USER_OFFSET,
                  &addr_family,
                  &family_name)));
        WOLFSENTRY_EXIT_ON_FALSE((family_name != NULL) &&
                                 (! strcmp(family_name,"my_AF")));

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_addr_family_drop_reference(WOLFSENTRY_CONTEXT_ARGS_OUT, addr_family, &action_results));
        WOLFSENTRY_EXIT_ON_TRUE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_DEALLOCATED));
    }

#endif /* WOLFSENTRY_PROTOCOL_NAMES */

    {
        wolfsentry_addr_family_parser_t parser;
        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_addr_family_get_parser(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                WOLFSENTRY_AF_USER_OFFSET,
                &parser));
        WOLFSENTRY_EXIT_ON_FALSE(parser == my_addr_family_parser);
    }
    {
        wolfsentry_addr_family_formatter_t formatter;
        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_addr_family_get_formatter(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                WOLFSENTRY_AF_USER_OFFSET,
                &formatter));
        WOLFSENTRY_EXIT_ON_FALSE(formatter == my_addr_family_formatter);
    }

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_shutdown(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&wolfsentry)));

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_RETURN_OK;
}

#endif /* TEST_USER_ADDR_FAMILIES */

#ifdef TEST_JSON

#include "wolfsentry/wolfsentry_json.h"
#ifdef WOLFSENTRY_HAVE_JSON_DOM
#include <wolfsentry/centijson_dom.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define PRIVATE_DATA_SIZE 32
#define PRIVATE_DATA_ALIGNMENT 16

static wolfsentry_errcode_t test_action(
    WOLFSENTRY_CONTEXT_ARGS_IN,
    const struct wolfsentry_action *action,
    void *handler_arg,
    void *caller_arg,
    const struct wolfsentry_event *trigger_event,
    wolfsentry_action_type_t action_type,
    const struct wolfsentry_route *target_route,
    struct wolfsentry_route_table *route_table,
    struct wolfsentry_route *rule_route,
    wolfsentry_action_res_t *action_results)
{
    const struct wolfsentry_event *parent_event;

    WOLFSENTRY_CONTEXT_ARGS_NOT_USED;

    (void)handler_arg;
    (void)route_table;
    (void)action_results;

    if (rule_route == NULL) {
        printf("null rule_route, target_route=%p\n", (void *)target_route);
        WOLFSENTRY_RETURN_OK;
    }

    parent_event = wolfsentry_route_parent_event(rule_route);
    printf("action callback: target_route=%p  a=\"%s\" parent_event=\"%s\" trigger=\"%s\" t=%u r_id=%u caller_arg=%p\n",
           (void *)target_route,
           wolfsentry_action_get_label(action),
           wolfsentry_event_get_label(parent_event),
           wolfsentry_event_get_label(trigger_event),
           action_type,
           wolfsentry_get_object_id(rule_route),
           caller_arg);
    WOLFSENTRY_RETURN_OK;
}

static wolfsentry_errcode_t json_feed_file(WOLFSENTRY_CONTEXT_ARGS_IN, const char *fname, wolfsentry_config_load_flags_t flags, int verbose) {
    wolfsentry_errcode_t ret;
    struct wolfsentry_json_process_state *jps;
    FILE *f;
    unsigned char buf[512];
    char err_buf[512];
    int fini_ret;

    if (strcmp(fname,"-"))
        f = fopen(fname, "r");
    else
        f = stdin;
    if (! f) {
        fprintf(stderr, "fopen(%s): %s\n",fname,strerror(errno));
        WOLFSENTRY_ERROR_RETURN(UNIT_TEST_FAILURE);
    }

    ret = wolfsentry_config_json_init(WOLFSENTRY_CONTEXT_ARGS_OUT, flags, &jps);
    WOLFSENTRY_RERETURN_IF_ERROR(ret);

    for (;;) {
        size_t n = fread(buf, 1, sizeof buf, f);
        if ((n < sizeof buf) && ferror(f)) {
            fprintf(stderr,"fread(%s): %s\n",fname, strerror(errno));
            ret = WOLFSENTRY_ERROR_ENCODE(UNIT_TEST_FAILURE);
            goto out;
        }

        ret = wolfsentry_config_json_feed(jps, buf, n, err_buf, sizeof err_buf);
        if (ret < 0) {
            if (verbose)
                fprintf(stderr, "%.*s\n", (int)sizeof err_buf, err_buf);
            goto out;
        }
        if ((n < sizeof buf) && feof(f))
            break;
    }

  out:

    fini_ret = wolfsentry_config_json_fini(&jps, err_buf, sizeof err_buf);
    if (fini_ret < 0) {
        if (verbose)
            fprintf(stderr, "%.*s\n", (int)sizeof err_buf, err_buf);
    }
    if (WOLFSENTRY_ERROR_CODE_IS(ret, OK))
        ret = fini_ret;
    if (WOLFSENTRY_ERROR_CODE_IS(ret, OK))
        ret = WOLFSENTRY_ERROR_ENCODE(OK);

    if (f != stdin)
        fclose(f);

    if ((ret < 0) && verbose)
        fprintf(stderr,"error processing file %s\n",fname);

    WOLFSENTRY_ERROR_RERETURN(ret);
}


static int test_json(const char *fname, const char *extra_fname) {
    wolfsentry_errcode_t ret;
    struct wolfsentry_context *wolfsentry;
    wolfsentry_ent_id_t id;

#ifdef WOLFSENTRY_HAVE_DESIGNATED_INITIALIZERS
    struct wolfsentry_eventconfig config = { .route_private_data_size = PRIVATE_DATA_SIZE, .route_private_data_alignment = PRIVATE_DATA_ALIGNMENT };
#else
    struct wolfsentry_eventconfig config = { PRIVATE_DATA_SIZE, PRIVATE_DATA_ALIGNMENT, 0, 0, 0, 0, 0 };
#endif

    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_init_ex(
            wolfsentry_build_settings,
            WOLFSENTRY_CONTEXT_ARGS_OUT_EX(WOLFSENTRY_TEST_HPI),
            &config,
            &wolfsentry,
            WOLFSENTRY_INIT_FLAG_NONE));

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_addr_family_handler_install(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            WOLFSENTRY_AF_USER_OFFSET,
            "my_AF",
            WOLFSENTRY_LENGTH_NULL_TERMINATED,
            my_addr_family_parser,
            my_addr_family_formatter,
            24 /* max_addr_bits */));

    WOLFSENTRY_EXIT_ON_FAILURE(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT, fname, WOLFSENTRY_CONFIG_LOAD_FLAG_NO_ROUTES_OR_EVENTS, 1));

    {
        static const char test_string[] = "hello";
        const char *value = NULL;
        int value_len = -1;
        struct wolfsentry_kv_pair_internal *kv_ref;

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_string(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "user-string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value,
                &value_len,
                &kv_ref));

        WOLFSENTRY_EXIT_ON_FALSE(value_len == (int)strlen(test_string));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(value, test_string) == 0);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_release_record(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &kv_ref));
    }

    {
        static const char user_cert_string[] =
            "-----BEGIN CERTIFICATE-----\n"
            "MIIDnzCCAyWgAwIBAgICEAEwCgYIKoZIzj0EAwMwgZcxCzAJBgNVBAYTAlVTMRMw\n"
            "EQYDVQQIDApXYXNoaW5ndG9uMRAwDgYDVQQHDAdTZWF0dGxlMRAwDgYDVQQKDAd3\n"
            "b2xmU1NMMRQwEgYDVQQLDAtEZXZlbG9wbWVudDEYMBYGA1UEAwwPd3d3LndvbGZz\n"
            "c2wuY29tMR8wHQYJKoZIhvcNAQkBFhBpbmZvQHdvbGZzc2wuY29tMCAXDTIyMDIx\n"
            "NTEyNTAyNFoYDzIwNTIwMjA4MTI1MDI0WjCBlTELMAkGA1UEBhMCVVMxEzARBgNV\n"
            "BAgMCldhc2hpbmd0b24xEDAOBgNVBAcMB1NlYXR0bGUxEDAOBgNVBAoMB0VsaXB0\n"
            "aWMxEjAQBgNVBAsMCUVDQzM4NFNydjEYMBYGA1UEAwwPd3d3LndvbGZzc2wuY29t\n"
            "MR8wHQYJKoZIhvcNAQkBFhBpbmZvQHdvbGZzc2wuY29tMHYwEAYHKoZIzj0CAQYF\n"
            "K4EEACIDYgAE6s+TTywJuzkUD1Zkw0C03w5jruVxSwDMBJf/4ek4lrtfkbJqzLU5\n"
            "X49wWfEB9lorAWxoC89VJa9tmEgKqHTJqRegDMP70yNo/gQ8Y1CIO7lPfGc09zup\n"
            "c+cbw1FeIhjso4IBQDCCATwwCQYDVR0TBAIwADARBglghkgBhvhCAQEEBAMCBkAw\n"
            "HQYDVR0OBBYEFII78mUv87QAxrwG/XlCdUtl0c68MIHXBgNVHSMEgc8wgcyAFKvg\n"
            "wyZMGNRyu9KEjJwKBZKAElNSoYGdpIGaMIGXMQswCQYDVQQGEwJVUzETMBEGA1UE\n"
            "CAwKV2FzaGluZ3RvbjEQMA4GA1UEBwwHU2VhdHRsZTEQMA4GA1UECgwHd29sZlNT\n"
            "TDEUMBIGA1UECwwLRGV2ZWxvcG1lbnQxGDAWBgNVBAMMD3d3dy53b2xmc3NsLmNv\n"
            "bTEfMB0GCSqGSIb3DQEJARYQaW5mb0B3b2xmc3NsLmNvbYIUaiYbTQIaM//CRxT5\n"
            "51VgWi5/ESkwDgYDVR0PAQH/BAQDAgOoMBMGA1UdJQQMMAoGCCsGAQUFBwMBMAoG\n"
            "CCqGSM49BAMDA2gAMGUCMQCEPZBU/y/EetTYGOKzLbtCN0CmHwmD3rwEeoLcVRdC\n"
            "XBeqB0LcyPZQzRS3Bhk5HyQCMBNiS5/JoIzSac8WToa9nik4ROlKOmOgZjiV4n3j\n"
            "F+yUIbg9aV7K5ISc2mF9G1G/0Q==\n"
            "-----END CERTIFICATE-----\n";
        const char *value = NULL;
        int value_len = -1;
        struct wolfsentry_kv_pair_internal *kv_ref;

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_string(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "user-cert-string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value,
                &value_len,
                &kv_ref));

        WOLFSENTRY_EXIT_ON_FALSE(value_len == (int)strlen(user_cert_string));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(value, user_cert_string) == 0);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_release_record(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &kv_ref));
    }

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_action_insert(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   "handle-insert",
                                   WOLFSENTRY_LENGTH_NULL_TERMINATED,
                                   WOLFSENTRY_ACTION_FLAG_NONE,
                                   test_action,
                                   NULL,
                                   &id));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_action_insert(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   "handle-delete",
                                   WOLFSENTRY_LENGTH_NULL_TERMINATED,
                                   WOLFSENTRY_ACTION_FLAG_NONE,
                                   test_action,
                                   NULL,
                                   &id));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_action_insert(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   "handle-match",
                                   WOLFSENTRY_LENGTH_NULL_TERMINATED,
                                   WOLFSENTRY_ACTION_FLAG_NONE,
                                   test_action,
                                   NULL,
                                   &id));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_action_insert(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   "handle-update",
                                   WOLFSENTRY_LENGTH_NULL_TERMINATED,
                                   WOLFSENTRY_ACTION_FLAG_NONE,
                                   test_action,
                                   NULL,
                                   &id));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_action_insert(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   "notify-on-decision",
                                   WOLFSENTRY_LENGTH_NULL_TERMINATED,
                                   WOLFSENTRY_ACTION_FLAG_NONE,
                                   test_action,
                                   NULL,
                                   &id));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_action_insert(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   "handle-connect",
                                   WOLFSENTRY_LENGTH_NULL_TERMINATED,
                                   WOLFSENTRY_ACTION_FLAG_NONE,
                                   test_action,
                                   NULL,
                                   &id));

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_action_insert(
                                   WOLFSENTRY_CONTEXT_ARGS_OUT,
                                   "handle-connect2",
                                   WOLFSENTRY_LENGTH_NULL_TERMINATED,
                                   WOLFSENTRY_ACTION_FLAG_NONE,
                                   test_action,
                                   NULL,
                                   &id));

    WOLFSENTRY_EXIT_ON_FAILURE(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT, fname, WOLFSENTRY_CONFIG_LOAD_FLAG_DRY_RUN, 1));

    WOLFSENTRY_EXIT_ON_FAILURE(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT, fname, WOLFSENTRY_CONFIG_LOAD_FLAG_LOAD_THEN_COMMIT, 1));

    WOLFSENTRY_EXIT_ON_SUCCESS(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT, fname, WOLFSENTRY_CONFIG_LOAD_FLAG_NO_FLUSH | WOLFSENTRY_CONFIG_LOAD_FLAG_LOAD_THEN_COMMIT, 0));

    {
        static const char *bad_json[] = {
            "{ \"wolfsentry-config-version\" : 1, \"user-values\" : { \"user-string\" : \"should collide\" } }",
            "{ \"wolfsentry-config-version\" : 2, \"user-values\" : { \"ok-user-string\" : \"shouldn't collide\" } }",
            "{ \"wolfsentry-config-version\" : 1, \"not-user-values\" : { \"ok-user-string\" : \"shouldn't collide\" } }",
            "{ \"wolfsentry-config-version\" : 1, \"user-values\" : { \"too-long-user-string-123456789-abcdefghi\" : \"x\" } }",
        };
        const char **bad_json_i;
        for (bad_json_i = bad_json; bad_json_i < &bad_json[length_of_array(bad_json)]; ++bad_json_i) {
            WOLFSENTRY_EXIT_ON_SUCCESS(
                wolfsentry_config_json_oneshot(
                    WOLFSENTRY_CONTEXT_ARGS_OUT,
                    (const unsigned char *)*bad_json_i,
                    strlen(*bad_json_i),
                    WOLFSENTRY_CONFIG_LOAD_FLAG_NO_FLUSH | WOLFSENTRY_CONFIG_LOAD_FLAG_LOAD_THEN_COMMIT,
                    NULL,
                    0));
        }
    }

    {
        const char *value = NULL;
        int value_len = -1;
        struct wolfsentry_kv_pair_internal *kv_ref;
        struct {
            struct wolfsentry_sockaddr sa;
            byte addr_buf[4];
        } remote, local;
        wolfsentry_ent_id_t route_id;
        wolfsentry_route_flags_t inexact_matches;
        wolfsentry_action_res_t action_results;

        if (extra_fname) {
            remote.sa.sa_family = local.sa.sa_family = AF_INET;
            remote.sa.sa_proto = local.sa.sa_proto = IPPROTO_TCP;
            remote.sa.sa_port = 12345;
            local.sa.sa_port = 13579;
            remote.sa.addr_len = local.sa.addr_len = sizeof remote.addr_buf * BITS_PER_BYTE;
            remote.sa.interface = local.sa.interface = 1;

            memcpy(remote.sa.addr,"\12\24\36\50",sizeof remote.addr_buf);
            memcpy(local.sa.addr,"\62\74\106\120",sizeof local.addr_buf);

            WOLFSENTRY_EXIT_ON_FAILURE(
                wolfsentry_route_event_dispatch(
                    WOLFSENTRY_CONTEXT_ARGS_OUT,
                    &remote.sa,
                    &local.sa,
                    WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN,
                    NULL /* event_label */,
                    0 /* event_label_len */,
                    NULL /* caller_arg */,
                    &route_id,
                    &inexact_matches,
                    &action_results));
            WOLFSENTRY_EXIT_ON_FALSE(route_id == WOLFSENTRY_ENT_ID_NONE);
            WOLFSENTRY_EXIT_ON_FALSE(action_results == WOLFSENTRY_ACTION_RES_REJECT);

            WOLFSENTRY_EXIT_ON_FAILURE(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT, extra_fname, WOLFSENTRY_CONFIG_LOAD_FLAG_NO_FLUSH | WOLFSENTRY_CONFIG_LOAD_FLAG_LOAD_THEN_COMMIT, 1));

            WOLFSENTRY_EXIT_ON_FAILURE(
                wolfsentry_route_event_dispatch(
                    WOLFSENTRY_CONTEXT_ARGS_OUT,
                    &remote.sa,
                    &local.sa,
                    WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN,
                    NULL /* event_label */,
                    0 /* event_label_len */,
                    NULL /* caller_arg */,
                    &route_id,
                    &inexact_matches,
                    &action_results));
            WOLFSENTRY_EXIT_ON_TRUE(route_id == WOLFSENTRY_ENT_ID_NONE);
            WOLFSENTRY_EXIT_ON_FALSE(action_results == WOLFSENTRY_ACTION_RES_ACCEPT);

            memcpy(remote.sa.addr,"\13\24\36\50",sizeof remote.addr_buf);

            WOLFSENTRY_EXIT_ON_FAILURE(
                wolfsentry_route_event_dispatch(
                    WOLFSENTRY_CONTEXT_ARGS_OUT,
                    &remote.sa,
                    &local.sa,
                    WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN,
                    NULL /* event_label */,
                    0 /* event_label_len */,
                    NULL /* caller_arg */,
                    &route_id,
                    &inexact_matches,
                    &action_results));
            WOLFSENTRY_EXIT_ON_FALSE(route_id == WOLFSENTRY_ENT_ID_NONE);
            WOLFSENTRY_EXIT_ON_FALSE(action_results == WOLFSENTRY_ACTION_RES_REJECT);
        } else {
            static const char *trivial_test_json = "{ \"wolfsentry-config-version\" : 1, \"user-values\" : { \"extra-user-string\" : \"extra hello\" } }";
            WOLFSENTRY_EXIT_ON_FAILURE(
                wolfsentry_config_json_oneshot(
                    WOLFSENTRY_CONTEXT_ARGS_OUT,
                    (const unsigned char *)trivial_test_json,
                    strlen(trivial_test_json),
                    WOLFSENTRY_CONFIG_LOAD_FLAG_NO_FLUSH | WOLFSENTRY_CONFIG_LOAD_FLAG_LOAD_THEN_COMMIT,
                    NULL,
                    0));
        }

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_string(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "user-string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value,
                &value_len,
                &kv_ref));

        WOLFSENTRY_EXIT_ON_FALSE(value_len == (int)strlen("hello"));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(value, "hello") == 0);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_release_record(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &kv_ref));


        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_get_string(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                "extra-user-string",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                &value,
                &value_len,
                &kv_ref));

        WOLFSENTRY_EXIT_ON_FALSE(value_len == (int)strlen("extra hello"));
        WOLFSENTRY_EXIT_ON_FALSE(strcmp(value, "extra hello") == 0);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_user_value_release_record(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &kv_ref));
    }

    WOLFSENTRY_EXIT_ON_FAILURE(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT, fname, WOLFSENTRY_CONFIG_LOAD_FLAG_NONE, 1));

    {
        struct wolfsentry_route_table *main_routes;
        struct wolfsentry_cursor *cursor;
        WOLFSENTRY_BYTE_STREAM_DECLARE_STACK(json_out, 8192);
        WOLFSENTRY_BYTE_STREAM_DECLARE_HEAP(json_out2, 8192);
        wolfsentry_hitcount_t n_seen = 0;
        char err_buf[512];

        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_BYTE_STREAM_INIT_HEAP(json_out2) != NULL);

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_lock_shared(WOLFSENTRY_CONTEXT_ARGS_OUT));

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_main_table(WOLFSENTRY_CONTEXT_ARGS_OUT, &main_routes));

        WOLFSENTRY_BYTE_STREAM_RESET(json_out);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_route_table_dump_json_start(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                main_routes,
                &cursor,
                WOLFSENTRY_BYTE_STREAM_PTR(json_out),
                WOLFSENTRY_BYTE_STREAM_SPC(json_out),
                WOLFSENTRY_FORMAT_FLAG_NONE));

        for (;;) {
            ret = wolfsentry_route_table_dump_json_next(
                 WOLFSENTRY_CONTEXT_ARGS_OUT,
                 main_routes,
                 cursor,
                 WOLFSENTRY_BYTE_STREAM_PTR(json_out),
                 WOLFSENTRY_BYTE_STREAM_SPC(json_out),
                 WOLFSENTRY_FORMAT_FLAG_NONE);
            if (ret < 0) {
                WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_ERROR_CODE_IS(ret, ITEM_NOT_FOUND));
                WOLFSENTRY_EXIT_ON_FAILURE(
                    wolfsentry_route_table_dump_json_end(
                        WOLFSENTRY_CONTEXT_ARGS_OUT,
                        main_routes,
                        &cursor,
                        WOLFSENTRY_BYTE_STREAM_PTR(json_out),
                        WOLFSENTRY_BYTE_STREAM_SPC(json_out),
                        WOLFSENTRY_FORMAT_FLAG_NONE));
            } else
                ++n_seen;
            if (ret < 0)
                break;
        }

        WOLFSENTRY_EXIT_ON_FALSE(n_seen == wolfsentry->routes->header.n_ents);

        ret = wolfsentry_config_json_oneshot(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            WOLFSENTRY_BYTE_STREAM_HEAD(json_out),
            WOLFSENTRY_BYTE_STREAM_LEN(json_out),
            WOLFSENTRY_CONFIG_LOAD_FLAG_LOAD_THEN_COMMIT | WOLFSENTRY_CONFIG_LOAD_FLAG_FLUSH_ONLY_ROUTES,
            err_buf,
            sizeof err_buf);
        if (ret < 0) {
            fprintf(stderr, "%.*s\n", (int)sizeof err_buf, err_buf);
            WOLFSENTRY_EXIT_ON_FAILURE(ret);
        }

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_main_table(WOLFSENTRY_CONTEXT_ARGS_OUT, &main_routes));

        WOLFSENTRY_BYTE_STREAM_RESET(json_out2);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_route_table_dump_json_start(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                main_routes,
                &cursor,
                WOLFSENTRY_BYTE_STREAM_PTR(json_out2),
                WOLFSENTRY_BYTE_STREAM_SPC(json_out2),
                WOLFSENTRY_FORMAT_FLAG_NONE));

        for (;;) {
            ret = wolfsentry_route_table_dump_json_next(
                 WOLFSENTRY_CONTEXT_ARGS_OUT,
                 main_routes,
                 cursor,
                 WOLFSENTRY_BYTE_STREAM_PTR(json_out2),
                 WOLFSENTRY_BYTE_STREAM_SPC(json_out2),
                 WOLFSENTRY_FORMAT_FLAG_NONE);
            if (ret < 0) {
                WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_ERROR_CODE_IS(ret, ITEM_NOT_FOUND));
                WOLFSENTRY_EXIT_ON_FAILURE(
                    wolfsentry_route_table_dump_json_end(
                        WOLFSENTRY_CONTEXT_ARGS_OUT,
                        main_routes,
                        &cursor,
                        WOLFSENTRY_BYTE_STREAM_PTR(json_out2),
                        WOLFSENTRY_BYTE_STREAM_SPC(json_out2),
                        WOLFSENTRY_FORMAT_FLAG_NONE));
            }
            if (ret < 0)
                break;
        }

        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_BYTE_STREAM_LEN(json_out2) == WOLFSENTRY_BYTE_STREAM_LEN(json_out));
        WOLFSENTRY_EXIT_ON_FALSE(memcmp(json_out, json_out2, WOLFSENTRY_BYTE_STREAM_LEN(json_out)) == 0);

        ret = wolfsentry_config_json_oneshot(
            WOLFSENTRY_CONTEXT_ARGS_OUT,
            WOLFSENTRY_BYTE_STREAM_HEAD(json_out),
            WOLFSENTRY_BYTE_STREAM_LEN(json_out),
            WOLFSENTRY_CONFIG_LOAD_FLAG_FLUSH_ONLY_ROUTES,
            err_buf,
            sizeof err_buf);
        if (ret < 0) {
            fprintf(stderr, "%.*s\n", (int)sizeof err_buf, err_buf);
            WOLFSENTRY_EXIT_ON_FAILURE(ret);
        }

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_main_table(WOLFSENTRY_CONTEXT_ARGS_OUT, &main_routes));

        WOLFSENTRY_BYTE_STREAM_RESET(json_out2);

        WOLFSENTRY_EXIT_ON_FAILURE(
            wolfsentry_route_table_dump_json_start(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                main_routes,
                &cursor,
                WOLFSENTRY_BYTE_STREAM_PTR(json_out2),
                WOLFSENTRY_BYTE_STREAM_SPC(json_out2),
                WOLFSENTRY_FORMAT_FLAG_NONE));

        for (;;) {
            ret = wolfsentry_route_table_dump_json_next(
                 WOLFSENTRY_CONTEXT_ARGS_OUT,
                 main_routes,
                 cursor,
                 WOLFSENTRY_BYTE_STREAM_PTR(json_out2),
                 WOLFSENTRY_BYTE_STREAM_SPC(json_out2),
                 WOLFSENTRY_FORMAT_FLAG_NONE);
            if (ret < 0) {
                WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_ERROR_CODE_IS(ret, ITEM_NOT_FOUND));
                WOLFSENTRY_EXIT_ON_FAILURE(
                    wolfsentry_route_table_dump_json_end(
                        WOLFSENTRY_CONTEXT_ARGS_OUT,
                        main_routes,
                        &cursor,
                        WOLFSENTRY_BYTE_STREAM_PTR(json_out2),
                        WOLFSENTRY_BYTE_STREAM_SPC(json_out2),
                        WOLFSENTRY_FORMAT_FLAG_NONE));
            }
            if (ret < 0)
                break;
        }

        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_BYTE_STREAM_LEN(json_out2) == WOLFSENTRY_BYTE_STREAM_LEN(json_out));
        WOLFSENTRY_EXIT_ON_FALSE(memcmp(json_out, json_out2, WOLFSENTRY_BYTE_STREAM_LEN(json_out)) == 0);

        WOLFSENTRY_EXIT_ON_FALSE(n_seen == wolfsentry->routes->header.n_ents);

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_unlock(WOLFSENTRY_CONTEXT_ARGS_OUT));

        WOLFSENTRY_BYTE_STREAM_FREE_HEAP(json_out2);
    }

    WOLFSENTRY_EXIT_ON_FAILURE(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT, fname, WOLFSENTRY_CONFIG_LOAD_FLAG_NONE, 1));

    {
        struct wolfsentry_context *ctx_clone;

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_clone(WOLFSENTRY_CONTEXT_ARGS_OUT, &ctx_clone, WOLFSENTRY_CLONE_FLAG_AS_AT_CREATION));
        WOLFSENTRY_EXIT_ON_FAILURE(json_feed_file(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(ctx_clone), fname, WOLFSENTRY_CONFIG_LOAD_FLAG_NONE, 1));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_exchange(WOLFSENTRY_CONTEXT_ARGS_OUT, ctx_clone));

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_free(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&ctx_clone)));
    }

    {
        struct wolfsentry_context *ctx_clone;

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_clone(WOLFSENTRY_CONTEXT_ARGS_OUT, &ctx_clone, WOLFSENTRY_CLONE_FLAG_NONE));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_free(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&ctx_clone)));
    }

    {
        struct wolfsentry_cursor *cursor;
        struct wolfsentry_route *route;
        struct wolfsentry_route_exports route_exports;
        struct wolfsentry_route_table *main_routes;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_lock_shared(WOLFSENTRY_CONTEXT_ARGS_OUT));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_get_main_table(WOLFSENTRY_CONTEXT_ARGS_OUT, &main_routes));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_table_iterate_start(WOLFSENTRY_CONTEXT_ARGS_OUT, main_routes, &cursor));
        for (ret = wolfsentry_route_table_iterate_current(main_routes, cursor, &route);
             ret >= 0;
             ret = wolfsentry_route_table_iterate_next(main_routes, cursor, &route)) {
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_export(WOLFSENTRY_CONTEXT_ARGS_OUT, route, &route_exports));
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_exports_render(WOLFSENTRY_CONTEXT_ARGS_OUT, &route_exports, stdout));
        }
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_table_iterate_end(WOLFSENTRY_CONTEXT_ARGS_OUT, main_routes, &cursor));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_unlock(WOLFSENTRY_CONTEXT_ARGS_OUT));
    }

#ifndef WOLFSENTRY_NO_STDIO
    {
        struct wolfsentry_kv_pair_internal *kv_ref;
        struct wolfsentry_cursor *cursor;
        const struct wolfsentry_kv_pair *kv_exports;
        const char *val_type;
        char val_buf[1024];
        int val_buf_space;
        wolfsentry_hitcount_t n_seen = 0;
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_lock_shared(WOLFSENTRY_CONTEXT_ARGS_OUT));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_user_values_iterate_start(WOLFSENTRY_CONTEXT_ARGS_OUT, &cursor));
        for (ret = wolfsentry_user_values_iterate_current(WOLFSENTRY_CONTEXT_ARGS_OUT, cursor, &kv_ref);
             ret >= 0;
             ret = wolfsentry_user_values_iterate_next(WOLFSENTRY_CONTEXT_ARGS_OUT, cursor, &kv_ref)) {
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_kv_pair_export(WOLFSENTRY_CONTEXT_ARGS_OUT, kv_ref, &kv_exports));
            val_buf_space = sizeof val_buf;
            if (wolfsentry_kv_type_to_string(WOLFSENTRY_KV_TYPE(kv_exports), &val_type) < 0)
                val_type = "?";
            if (wolfsentry_kv_render_value(WOLFSENTRY_CONTEXT_ARGS_OUT, kv_exports, val_buf, &val_buf_space) < 0) {
                if (WOLFSENTRY_KV_TYPE(kv_exports) == WOLFSENTRY_KV_BYTES)
                    snprintf(val_buf, sizeof val_buf, "\"%.*s\"", (int)WOLFSENTRY_KV_V_BYTES_LEN(kv_exports), WOLFSENTRY_KV_V_BYTES(kv_exports));
                else
                    strcpy(val_buf,"?");
            }
            printf("{ \"%.*s\" : { \"type\" : \"%s\", \"value\" : %s } }\n",
                   (int)WOLFSENTRY_KV_KEY_LEN(kv_exports),
                   WOLFSENTRY_KV_KEY(kv_exports),
                   val_type,
                   val_buf);
            ++n_seen;
        }
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_user_values_iterate_end(WOLFSENTRY_CONTEXT_ARGS_OUT, &cursor));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_context_unlock(WOLFSENTRY_CONTEXT_ARGS_OUT));
        WOLFSENTRY_EXIT_ON_FALSE(n_seen == wolfsentry->user_values->header.n_ents);
    }
#endif

    {
        struct {
            struct wolfsentry_sockaddr sa;
            byte addr_buf[4];
        } remote, local;
        wolfsentry_route_flags_t inexact_matches;
        wolfsentry_action_res_t action_results;

        remote.sa.sa_family = local.sa.sa_family = AF_INET;
        remote.sa.sa_proto = local.sa.sa_proto = IPPROTO_TCP;
        remote.sa.sa_port = 12345;
        local.sa.sa_port = 443;
        remote.sa.addr_len = local.sa.addr_len = sizeof remote.addr_buf * BITS_PER_BYTE;
        remote.sa.interface = local.sa.interface = 1;
        memcpy(remote.sa.addr,"\177\0\0\1",sizeof remote.addr_buf);
        memcpy(local.sa.addr,"\177\0\0\1",sizeof local.addr_buf);

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &remote.sa,
                &local.sa,
                WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN,
                "call-in-from-unit-test",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                (void *)0x12345678 /* caller_arg */,
                &id,
                &inexact_matches, &action_results));

        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_ACCEPT));
    }

    {
        struct {
            struct wolfsentry_sockaddr sa;
            byte addr_buf[4];
        } remote, local;
        wolfsentry_route_flags_t inexact_matches;
        wolfsentry_action_res_t action_results;

        remote.sa.sa_family = local.sa.sa_family = AF_INET;
        remote.sa.sa_proto = local.sa.sa_proto = IPPROTO_TCP;
        remote.sa.sa_port = 0;
        local.sa.sa_port = 13579;
        remote.sa.addr_len = local.sa.addr_len = sizeof remote.addr_buf * BITS_PER_BYTE;
        remote.sa.interface = local.sa.interface = 0;
        memcpy(remote.sa.addr,"\1\2\3\4",sizeof remote.addr_buf);
        memcpy(local.sa.addr,"\0\0\0\0",sizeof local.addr_buf);

        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_route_event_dispatch(
                WOLFSENTRY_CONTEXT_ARGS_OUT,
                &remote.sa,
                &local.sa,
                WOLFSENTRY_ROUTE_FLAG_DIRECTION_IN,
                "call-in-from-unit-test",
                WOLFSENTRY_LENGTH_NULL_TERMINATED,
                (void *)0x12345678 /* caller_arg */,
                &id,
                &inexact_matches, &action_results));

        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_REJECT));
        WOLFSENTRY_EXIT_ON_FALSE(WOLFSENTRY_CHECK_BITS(action_results, WOLFSENTRY_ACTION_RES_PORT_RESET));
    }


#ifdef WOLFSENTRY_HAVE_JSON_DOM
    {
        unsigned char *test_json_document = NULL;
        int fd = -1;
        JSON_VALUE p_root;
        JSON_VALUE *v1 = NULL, *v2 = NULL, *v3 = NULL;
        struct stat st;
        static const JSON_CONFIG centijson_config = {
            65536,  /* max_total_len */
            1000,  /* max_total_values */
            20,  /* max_number_len */
            WOLFSENTRY_KV_MAX_VALUE_BYTES,  /* max_string_len */
            WOLFSENTRY_MAX_LABEL_BYTES,  /* max_key_len */
            10,  /* max_nesting_level */
            JSON_NOSCALARROOT   /* flags */
        };
        JSON_INPUT_POS json_pos;
        const unsigned char *s;
        size_t alen, i;

        WOLFSENTRY_EXIT_ON_SYSFAILURE(fd = open(fname, O_RDONLY));
        WOLFSENTRY_EXIT_ON_SYSFAILURE(fstat(fd, &st));
        WOLFSENTRY_EXIT_ON_SYSFALSE((test_json_document = (unsigned char *)malloc((size_t)st.st_size)) != NULL);
        WOLFSENTRY_EXIT_ON_SYSFALSE(read(fd, test_json_document, (size_t)st.st_size) == st.st_size);

        if ((ret = json_dom_parse(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), test_json_document, (size_t)st.st_size, &centijson_config,
                                  0 /* dom_flags */, &p_root, &json_pos)) < 0) {
            void *p = memchr((const char *)(test_json_document + json_pos.offset), '\n', (size_t)st.st_size - json_pos.offset);
            int linelen = p ? ((int)((unsigned char *)p - (test_json_document + json_pos.offset)) + (int)json_pos.column_number - 1) :
                ((int)((int)st.st_size - (int)json_pos.offset) + (int)json_pos.column_number - 1);
            if (WOLFSENTRY_ERROR_DECODE_SOURCE_ID(ret) == WOLFSENTRY_SOURCE_ID_UNSET)
                fprintf(stderr, "json_dom_parse failed at offset " SIZET_FMT ", L%u, col %u, with centijson code %d: %s\n", json_pos.offset,json_pos.line_number, json_pos.column_number, ret, json_dom_error_str(ret));
            else
                fprintf(stderr, "json_dom_parse failed at offset " SIZET_FMT ", L%u, col %u, with " WOLFSENTRY_ERROR_FMT "\n", json_pos.offset,json_pos.line_number, json_pos.column_number, WOLFSENTRY_ERROR_FMT_ARGS(ret));
            fprintf(stderr,"%.*s\n", linelen, test_json_document + json_pos.offset - json_pos.column_number + 1);
            exit(1);
        }

        WOLFSENTRY_EXIT_ON_TRUE((v1 = json_value_path(&p_root, "wolfsentry-config-version")) == NULL);
        WOLFSENTRY_EXIT_ON_FALSE(json_value_uint32(v1) == 1U);
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v1)));

        WOLFSENTRY_EXIT_ON_TRUE((v1 = json_value_path(&p_root, "default-policies")) == NULL);
        WOLFSENTRY_EXIT_ON_TRUE((v2 = json_value_path(v1, "default-policy")) == NULL);
        WOLFSENTRY_EXIT_ON_TRUE((s = json_value_string(v2)) == NULL);
        WOLFSENTRY_EXIT_ON_FALSE(strcmp((const char *)s, "reject") == 0);
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v2)));

        WOLFSENTRY_EXIT_ON_TRUE((v2 = json_value_path(v1, "default-event")) == NULL);
        WOLFSENTRY_EXIT_ON_TRUE((s = json_value_string(v2)) == NULL);
        WOLFSENTRY_EXIT_ON_FALSE(strcmp((const char *)s, "static-route-parent") == 0);
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v2)));
        v2 = NULL;

        WOLFSENTRY_EXIT_ON_TRUE((v1 = json_value_path(&p_root, "static-routes-insert")) == NULL);
        WOLFSENTRY_EXIT_ON_TRUE((alen = json_value_array_size(v1)) <= 0);
        for (i = 0; i < alen; ++i) {
            WOLFSENTRY_EXIT_ON_TRUE((v2 = json_value_array_get(v1, i)) == NULL);
            WOLFSENTRY_EXIT_ON_TRUE((v3 = json_value_path(v2, "family")) == NULL);
            WOLFSENTRY_EXIT_ON_TRUE((json_value_string(v3) == NULL) && (json_value_int32(v3) <= 0));
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v3)));
            v3 = NULL;
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v2)));
            v2 = NULL;
        }
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v1)));

        WOLFSENTRY_EXIT_ON_TRUE((v1 = json_value_path(&p_root, "user-values/user-null")) == NULL);
        WOLFSENTRY_EXIT_ON_FALSE(json_value_type(v1) == JSON_VALUE_NULL);

        if (v3)
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v3)));
        if (v2)
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v2)));
        if (v1)
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), v1)));
        WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), &p_root)));
        if (test_json_document != NULL)
            free(test_json_document);
        if (fd != -1)
            (void)close(fd);
    }
#endif /* WOLFSENTRY_HAVE_JSON_DOM */

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_shutdown(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&wolfsentry)));

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_RETURN_OK;
}

#endif /* TEST_JSON */

#ifdef TEST_JSON_CORPUS

#include "wolfsentry/wolfsentry_json.h"
#include <wolfsentry/centijson_dom.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>

static int dump_string_for_json(const unsigned char* str, size_t size, void* user_data) {
    (void)user_data;
    printf("%.*s", (int)size, str);
    return 0;
}

static int test_json_corpus(void) {
    wolfsentry_errcode_t ret = WOLFSENTRY_ERROR_ENCODE(ITEM_NOT_FOUND);
    struct wolfsentry_context *wolfsentry;

    WOLFSENTRY_THREAD_HEADER_CHECKED(WOLFSENTRY_THREAD_FLAG_NONE);

    WOLFSENTRY_EXIT_ON_FAILURE(
        wolfsentry_init_ex(
            wolfsentry_build_settings,
            WOLFSENTRY_CONTEXT_ARGS_OUT_EX(WOLFSENTRY_TEST_HPI),
            NULL /* config */,
            &wolfsentry,
            WOLFSENTRY_INIT_FLAG_NONE));

    do {
        static JSON_CONFIG centijson_config = {
            65536,  /* max_total_len */
            10000,  /* max_total_values */
            20,  /* max_number_len */
            4096,  /* max_string_len */
            255,  /* max_key_len */
            10,  /* max_nesting_level */
            0 /*JSON_IGNOREILLUTF8VALUE*/ /* flags */
        };
        unsigned dom_flags = 0;
        JSON_VALUE p_root, clone;
        JSON_INPUT_POS json_pos;
        DIR *corpus_dir;
        struct dirent *scenario_ent;
        int scenario_fd;
        struct stat st;
        const unsigned char *scenario = MAP_FAILED;
        const char *corpus_path;
        char *cp, *endcp;
        int dump_json = 0;
        int ignore_failed_parse = 0;

        if (! (corpus_path = getenv("JSON_TEST_CORPUS_DIR"))) {
            printf("JSON_TEST_CORPUS_DIR unset -- skipping test_json_corpus().\n");
            ret = 0;
            break;
        }

#define PARSE_UNSIGNED_EV(ev, type, elname) do { if ((cp = getenv(ev))) { \
            centijson_config.elname = (type)strtoul(cp, &endcp, 0);     \
            WOLFSENTRY_EXIT_ON_FALSE((endcp != cp) || (*endcp == 0));   \
            } } while (0)

        PARSE_UNSIGNED_EV("JSON_TEST_CORPUS_MAX_TOTAL_LEN", size_t, max_total_len);
        PARSE_UNSIGNED_EV("JSON_TEST_CORPUS_MAX_TOTAL_VALUES", size_t, max_total_values);
        PARSE_UNSIGNED_EV("JSON_TEST_CORPUS_MAX_STRING_LEN", size_t, max_string_len);
        PARSE_UNSIGNED_EV("JSON_TEST_CORPUS_MAX_KEY_LEN", size_t, max_key_len);
        PARSE_UNSIGNED_EV("JSON_TEST_CORPUS_MAX_NESTING_LEVEL", unsigned, max_nesting_level);

        if ((cp = getenv("JSON_TEST_CORPUS_FLAGS"))) {
            static const struct { const char *name; unsigned int flag; unsigned int dom_flag; } centijson_flag_map[] = {
#define FLAG_MAP_ENT(name) { #name, JSON_ ## name, 0 }
                FLAG_MAP_ENT(NONULLASROOT),
                FLAG_MAP_ENT(NOBOOLASROOT),
                FLAG_MAP_ENT(NONUMBERASROOT),
                FLAG_MAP_ENT(NOSTRINGASROOT),
                FLAG_MAP_ENT(NOARRAYASROOT),
                FLAG_MAP_ENT(NOOBJECTASROOT),
                FLAG_MAP_ENT(IGNOREILLUTF8KEY),
                FLAG_MAP_ENT(FIXILLUTF8KEY),
                FLAG_MAP_ENT(IGNOREILLUTF8VALUE),
                FLAG_MAP_ENT(FIXILLUTF8VALUE),
                FLAG_MAP_ENT(NOSCALARROOT) /* compound flag */,
                FLAG_MAP_ENT(NOVECTORROOT) /* compound flag */,
#define FLAG_MAP_DOM_ENT(name) { #name, 0, JSON_DOM_ ## name }
                FLAG_MAP_DOM_ENT(DUPKEY_ABORT),
                FLAG_MAP_DOM_ENT(DUPKEY_USEFIRST),
                FLAG_MAP_DOM_ENT(DUPKEY_USELAST),
                FLAG_MAP_DOM_ENT(MAINTAINDICTORDER)
            };
            while (*cp != 0) {
                size_t label_len, i;
                endcp = strchr(cp, '|');
                if (endcp)
                    label_len = (size_t)(endcp - cp);
                else
                    label_len = strlen(cp);
                for (i = 0; i < sizeof centijson_flag_map / sizeof centijson_flag_map[0]; ++i) {
                    if ((label_len == strlen(centijson_flag_map[i].name)) && (! memcmp(cp, centijson_flag_map[i].name, label_len))) {
                        centijson_config.flags |= centijson_flag_map[i].flag;
                        dom_flags |= centijson_flag_map[i].dom_flag;
                        break;
                    }
                }
                if (i == sizeof centijson_flag_map / sizeof centijson_flag_map[0]) {
                    fprintf(stderr, "unrecognized flag \"%.*s\" in JSON_TEST_CORPUS_FLAGS.\n", (int)label_len, cp);
                    exit(1);
                }
                cp += label_len;
                if (*cp == '|')
                    ++cp;
            }
        }

        if (getenv("JSON_TEST_CORPUS_DUMP"))
            dump_json = 1;

        if (getenv("JSON_TEST_CORPUS_IGNORE_FAILED_PARSE"))
            ignore_failed_parse = 1;

        corpus_dir = opendir(corpus_path);
        if (! corpus_dir) {
            perror(corpus_path);
            ret = WOLFSENTRY_ERROR_ENCODE(SYS_OP_FATAL);
            break;
        }

        json_value_init_null(&p_root);
        json_value_init_null(&clone);

        while ((scenario_ent = readdir(corpus_dir))) {
            size_t namelen = strlen(scenario_ent->d_name);
            if (namelen <= strlen(".json"))
                continue;
            if (strcmp(scenario_ent->d_name + strlen(scenario_ent->d_name) - strlen(".json"), ".json") != 0)
                continue;
            scenario_fd = openat(dirfd(corpus_dir), scenario_ent->d_name, O_RDONLY);
            if (scenario_fd < 0) {
                perror(scenario_ent->d_name);
                continue;
            }
            if (fstat(scenario_fd, &st) < 0) {
                perror(scenario_ent->d_name);
                goto inner_cleanup;
            }
            scenario = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_SHARED, scenario_fd, 0);
            if (scenario == MAP_FAILED) {
                perror(scenario_ent->d_name);
                goto inner_cleanup;
            }

            printf("%s\n", scenario_ent->d_name);

            if ((ret = json_dom_parse(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), scenario, (size_t)st.st_size, &centijson_config,
                                      dom_flags, &p_root, &json_pos)) < 0) {
                void *p = memchr((const char *)(scenario + json_pos.offset), '\n', (size_t)st.st_size - json_pos.offset);
                int linelen = p ? ((int)((unsigned char *)p - (scenario + json_pos.offset)) + (int)json_pos.column_number - 1) :
                    ((int)((int)st.st_size - (int)json_pos.offset) + (int)json_pos.column_number - 1);
                if (WOLFSENTRY_ERROR_DECODE_SOURCE_ID(ret) == WOLFSENTRY_SOURCE_ID_UNSET)
                    fprintf(stderr, "%s/%s: json_dom_parse failed at offset " SIZET_FMT ", L%u, col %u, with centijson code %d: %s\n", corpus_path, scenario_ent->d_name, json_pos.offset,json_pos.line_number, json_pos.column_number, ret, json_dom_error_str(ret));
                else
                    fprintf(stderr, "%s/%s: json_dom_parse failed at offset " SIZET_FMT ", L%u, col %u, with " WOLFSENTRY_ERROR_FMT "\n", corpus_path, scenario_ent->d_name, json_pos.offset,json_pos.line_number, json_pos.column_number, WOLFSENTRY_ERROR_FMT_ARGS(ret));
                fprintf(stderr,"%.*s\n", linelen, scenario + json_pos.offset - json_pos.column_number + 1);
                goto inner_cleanup;
            }

            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_clone(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), &p_root, &clone)));

            if (dump_json) {
                WOLFSENTRY_EXIT_ON_FAILURE(json_dom_dump(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), &clone, dump_string_for_json, NULL /* user_data */, 2 /* tab_width */, JSON_DOM_DUMP_INDENTWITHSPACES | (dom_flags & JSON_DOM_MAINTAINDICTORDER ? JSON_DOM_DUMP_PREFERDICTORDER : 0)));
            }

        inner_cleanup:

            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), &p_root)));
            WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_centijson_errcode_translate(json_value_fini(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(wolfsentry_get_allocator(wolfsentry)), &clone)));
            if (scenario_fd >= 0)
                (void)close(scenario_fd);
            if (scenario != MAP_FAILED) {
                munmap((void *)scenario, (size_t)st.st_size);
                scenario = MAP_FAILED;
            }

            if ((ret < 0) && (! ignore_failed_parse)) {
                ret = wolfsentry_centijson_errcode_translate(ret);
                ret = WOLFSENTRY_ERROR_RECODE(ret);
                break;
            }
        }

        WOLFSENTRY_EXIT_ON_SYSFAILURE(closedir(corpus_dir));
    } while (0);

    WOLFSENTRY_EXIT_ON_FAILURE(wolfsentry_shutdown(WOLFSENTRY_CONTEXT_ARGS_OUT_EX(&wolfsentry)));

    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_THREAD_TAILER(WOLFSENTRY_THREAD_FLAG_NONE));

    WOLFSENTRY_ERROR_RERETURN(ret);
}

#endif /* TEST_JSON_CORPUS */

int main (int argc, char* argv[]) {
    wolfsentry_errcode_t ret = 0;
    int err = 0;
    (void)argc;
    (void)argv;

#ifdef WOLFSENTRY_ERROR_STRINGS
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_REGISTER_SOURCE());
    WOLFSENTRY_EXIT_ON_FAILURE(WOLFSENTRY_REGISTER_ERROR(UNIT_TEST_FAILURE, "failure within unit test"));
#endif

#ifdef TEST_INIT
    ret = test_init();
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_init failed, " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

#ifdef TEST_RWLOCKS
    ret = test_rw_locks();
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_rw_locks failed, " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

#ifdef TEST_STATIC_ROUTES
    ret = test_static_routes();
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_static_routes failed, " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

#ifdef TEST_DYNAMIC_RULES
    ret = test_dynamic_rules();
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_dynamic_rules failed, " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

#ifdef TEST_USER_VALUES
    ret = test_user_values();
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_user_values failed, " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

#ifdef TEST_USER_ADDR_FAMILIES
    ret = test_user_addr_families();
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_addr_families failed, " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

#ifdef TEST_JSON
#if defined(WOLFSENTRY_PROTOCOL_NAMES) && !defined(WOLFSENTRY_NO_GETPROTOBY)
#ifdef EXTRA_TEST_JSON_CONFIG_PATH
    ret = test_json(TEST_JSON_CONFIG_PATH, EXTRA_TEST_JSON_CONFIG_PATH);
#else
    ret = test_json(TEST_JSON_CONFIG_PATH, NULL);
#endif
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_json failed for " TEST_JSON_CONFIG_PATH ", " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif
    ret = test_json(TEST_NUMERIC_JSON_CONFIG_PATH, NULL);
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_json failed for " TEST_NUMERIC_JSON_CONFIG_PATH ", " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

#ifdef TEST_JSON_CORPUS
    ret = test_json_corpus();
    if (! WOLFSENTRY_ERROR_CODE_IS(ret, OK)) {
        printf("test_json_corpus failed, " WOLFSENTRY_ERROR_FMT "\n", WOLFSENTRY_ERROR_FMT_ARGS(ret));
        err = 1;
    }
#endif

    WOLFSENTRY_RETURN_VALUE(err);
}
