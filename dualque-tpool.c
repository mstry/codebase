#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "list.h"

#define MAX_PTHREADS_NUM            300
#define ARG_BUF_LEN                 2

typedef int (*dealer_exec_t)(void *);
typedef struct {
	struct list_head entry;
	void *argp;
	char buf[ARG_BUF_LEN];
	dealer_exec_t exec;
} dealer_t;

struct worker_thread_mgr {
        pthread_mutex_t mutex;
        pthread_cond_t wait;
        uint32_t nr_running_threads;

        struct list_head job_queue;
	struct list_head idle_jobs;
};

static struct worker_thread_mgr worker_mgr;
static dealer_t worker_dealers[MAX_PTHREADS_NUM];

static void __worker_clean(void *arg)
{
        pthread_mutex_t *mutex = (pthread_mutex_t *)arg;
        pthread_mutex_unlock(mutex);
}

static void *__worker_thread(void *arg)
{
	struct worker_thread_mgr *mgr = (struct worker_thread_mgr *)arg;
        pthread_mutex_t *mutex = &mgr->mutex;
        dealer_t *dealer;

        pthread_cleanup_push(__worker_clean, (void *)mutex);

        while (1) {
                pthread_mutex_lock(mutex);
                while (list_empty(&mgr->job_queue)) {
                        pthread_cond_wait(&mgr->wait, mutex);
                }
                dealer = list_entry(mgr->job_queue.next, dealer_t, entry);
                list_del_init(&dealer->entry);
		pthread_mutex_unlock(mutex);

		dealer->exec(dealer->argp);
		if (dealer->argp != dealer->buf) {
			free(dealer->argp);
			dealer->argp = dealer->buf;
		}

		pthread_mutex_lock(mutex);
		list_add_tail(&dealer->entry, &mgr->idle_jobs);
		pthread_mutex_unlock(mutex);
        }

        pthread_cleanup_pop(0);

        return 0;
}

static int __worker_start(struct worker_thread_mgr *mgr)
{
        int ret;
        pthread_t thread;
	pthread_attr_t ta;

	(void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);

        ret = pthread_create(&thread, &ta, __worker_thread, (void *)mgr);
        if (ret) {
		ret = errno;
        	return ret;
        }
        mgr->nr_running_threads++;

        return 0;
}

static int __worker_init(struct worker_thread_mgr *mgr)
{
        int ret, i;
	dealer_t *dealer;

        ret = pthread_mutex_init(&mgr->mutex, NULL);
        if (ret)
                return ret;

        mgr->nr_running_threads = 0;

        INIT_LIST_HEAD(&mgr->job_queue);
        INIT_LIST_HEAD(&mgr->idle_jobs);
	for (i = 0; i < MAX_PTHREADS_NUM; i++) {
		dealer = worker_dealers + i;
		dealer->argp = (void *)dealer->buf;
		list_add_tail(&dealer->entry, &mgr->idle_jobs);
	}

        ret = pthread_cond_init(&mgr->wait, NULL);
        if (ret)
                return ret;

        return 0;
}

int worker_thread_queue(dealer_exec_t exec, void *arg, int size)
{
	dealer_t *dealer;

        pthread_mutex_lock(&worker_mgr.mutex);

	if (list_empty(&worker_mgr.idle_jobs)) {
		pthread_mutex_unlock(&worker_mgr.mutex);
		return EAGAIN;
	}

	dealer = list_entry(worker_mgr.idle_jobs.next, dealer_t, entry);

	if (size > ARG_BUF_LEN) {
		dealer->argp = malloc(size);
		if (NULL == dealer->argp) {
			dealer->argp = dealer->buf;
			pthread_mutex_unlock(&worker_mgr.mutex);
			return ENOMEM;
		}
	}

	memcpy(dealer->argp, arg, size);
	dealer->exec = exec;
	list_del_init(&dealer->entry);
        list_add_tail(&dealer->entry, &worker_mgr.job_queue);
        pthread_cond_signal(&worker_mgr.wait);

        pthread_mutex_unlock(&worker_mgr.mutex);

        return 0;
}

int worker_thread_init()
{
        int ret;
        ret = __worker_init(&worker_mgr);
        if (ret) return ret;
        while (worker_mgr.nr_running_threads < MAX_PTHREADS_NUM) {
                ret = __worker_start(&worker_mgr);
                if (ret) return ret;
        }
        return 0;
}

#if 1
static int __dealer(void *arg)
{
	int _arg = *((int *)arg);
	printf("[%ld] %d\n", pthread_self(), _arg);
	return 0;
}

int main()
{
	int ret,i;
	ret = worker_thread_init();
	for(i = 0; i < 100; i++)
	{
		ret = worker_thread_queue(__dealer, (void *)&i, sizeof(int));
	}
	sleep(3);
	return 0;
}
#endif
