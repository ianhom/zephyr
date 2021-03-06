/*
 * Copyright (c) 2016 Intel Corporation
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

/**
 * @addtogroup t_threads_scheduling
 * @{
 * @defgroup t_sched_timeslice_and_lock test_sched_timeslice_and_lock
 * @brief TestPurpose: verify sched time slice and lock/unlock
 * @}
 */

#include "test_sched.h"
#define THREADS_NUM     3
static char __noinit __stack tstack[THREADS_NUM][STACK_SIZE];

static struct thread_data tdata[THREADS_NUM];
static int old_prio, init_prio;

static void thread_entry(void *p1, void *p2, void *p3)
{
	int sleep_ms = (int)p2;

	if (sleep_ms > 0) {
		k_sleep(sleep_ms);
	}

	int tnum = (int)p1;

	tdata[tnum].executed = 1;
}

static void setup_threads(void)
{
	old_prio = k_thread_priority_get(k_current_get());
	for (int i = 0; i < THREADS_NUM; i++) {
		if (i == 0) {
			/* spawn thread with higher priority */
			tdata[i].priority = init_prio - 1;
		} else if (i == 1) {
			/* spawn thread with same priority */
			tdata[i].priority = init_prio;
		} else {
			/* spawn thread with lower priority */
			tdata[i].priority = init_prio + 1;
		}
		tdata[i].executed = 0;
	}
	k_thread_priority_set(k_current_get(), init_prio);
}

static void spawn_threads(int sleep_sec)
{
	for (int i = 0; i < THREADS_NUM; i++) {
		tdata[i].tid = k_thread_spawn(tstack[i], STACK_SIZE,
					      thread_entry, (void *)i, (void *)sleep_sec, NULL,
					      tdata[i].priority, 0, 0);
	}
}

static void teardown_threads(void)
{
	for (int i = 0; i < THREADS_NUM; i++) {
		k_thread_abort(tdata[i].tid);
	}
	k_thread_priority_set(k_current_get(), old_prio);
}

/*test cases*/
void test_yield_cooperative(void)
{

	/* set current thread to a cooperative priority */
	init_prio = -1;
	setup_threads();

	spawn_threads(0);
	/* checkpoint: only higher priority thread get executed when yield */
	k_yield();
	assert_true(tdata[0].executed == 1, NULL);
	assert_true(tdata[1].executed == 1, NULL);
	for (int i = 2; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 0, NULL);
	}
	/* restore environment */
	teardown_threads();
}

void test_sleep_cooperative(void)
{
	/* set current thread to a cooperative priority */
	init_prio = -1;
	setup_threads();

	spawn_threads(0);
	/* checkpoint: all ready threads get executed when k_sleep */
	k_sleep(100);
	for (int i = 0; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 1, NULL);
	}

	/* restore environment */
	teardown_threads();
}

void test_busy_wait_cooperative(void)
{
	/* set current thread to a preemptible priority */
	init_prio = -1;
	setup_threads();

	spawn_threads(0);
	k_busy_wait(100000); /* 100 ms */
	/* checkpoint: No other threads get executed */
	for (int i = 0; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 0, NULL);
	}
	/* restore environment */
	teardown_threads();
}

void test_sleep_wakeup_preemptible(void)
{
	/* set current thread to a preemptible priority */
	init_prio = 0;
	setup_threads();

	spawn_threads(10 * 1000); /* 10 second */
	/* checkpoint: lower threads not executed, high threads are in sleep */
	for (int i = 0; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 0, NULL);
	}
	k_wakeup(tdata[0].tid);
	assert_true(tdata[0].executed == 1, NULL);
	/* restore environment */
	teardown_threads();
}

void test_time_slicing_preemptible(void)
{
	/* set current thread to a preemptible priority */
	init_prio = 0;
	setup_threads();

	k_sched_time_slice_set(200, 0); /* 200 ms */
	spawn_threads(0);
	/* checkpoint: higher priority threads get executed immediately */
	assert_true(tdata[0].executed == 1, NULL);
	k_busy_wait(500000); /* 500 ms */
	/* checkpoint: equal priority threads get executed every time slice */
	assert_true(tdata[1].executed == 1, NULL);
	for (int i = 2; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 0, NULL);
	}

	/* restore environment */
	k_sched_time_slice_set(0, 0); /* disable time slice */
	teardown_threads();
}

void test_time_slicing_disable_preemptible(void)
{
	/* set current thread to a preemptible priority */
	init_prio = 0;
	setup_threads();

	spawn_threads(0);
	/* checkpoint: higher priority threads get executed immediately */
	assert_true(tdata[0].executed == 1, NULL);
	k_busy_wait(500000); /* 500 ms */
	/* checkpoint: equal priority threads get executed every time slice */
	assert_true(tdata[1].executed == 0, NULL);
	for (int i = 2; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 0, NULL);
	}
	/* restore environment */
	teardown_threads();
}

void test_lock_preemptible(void)
{
	/* set current thread to a preemptible priority */
	init_prio = 0;
	setup_threads();

	k_sched_lock();
	spawn_threads(0);
	/* do critical thing */
	k_busy_wait(100000);
	/* checkpoint: all other threads not been executed */
	for (int i = 0; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 0, NULL);
	}
	/* make current thread unready */
	k_sleep(100);
	/* checkpoint: all other threads get executed */
	for (int i = 0; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 1, NULL);
	}
	/* restore environment */
	teardown_threads();
}

void test_unlock_preemptible(void)
{
	/* set current thread to a preemptible priority */
	init_prio = 0;
	setup_threads();

	k_sched_lock();
	spawn_threads(0);
	/* do critical thing */
	k_busy_wait(100000);

	k_sched_unlock();
	/* checkpoint: higher threads get executed */
	assert_true(tdata[0].executed == 1, NULL);
	for (int i = 1; i < THREADS_NUM; i++) {
		assert_true(tdata[i].executed == 0, NULL);
	}
	/* restore environment */
	teardown_threads();
}
