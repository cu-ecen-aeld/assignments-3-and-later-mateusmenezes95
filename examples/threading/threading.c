#include "threading.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void * threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* args = (struct thread_data *) thread_param;

    usleep(args->wait_to_obtain_mutex_in_ms * 1000);

    int pthread_mutex_lock_ret = pthread_mutex_lock(args->mutex);
    if (pthread_mutex_lock_ret != 0) {
        printf("pthread_mutex_lock failed with %d\n", pthread_mutex_lock_ret);
        args->thread_complete_success = false;
        pthread_exit((void *)args);
    }

    usleep(args->wait_to_release_mutex_in_ms * 1000);

    int pthread_mutex_unlock_ret = pthread_mutex_unlock(args->mutex);
    if (pthread_mutex_unlock_ret != 0) {
        printf("pthread_mutex_lock failed with %d\n", pthread_mutex_unlock_ret);
        args->thread_complete_success = false;
        pthread_exit((void *)args);
    }

    usleep(args->wait_to_release_mutex_in_ms * 1000);
    
    args->thread_complete_success = true;

    return (void *)args;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data * data = (struct thread_data *)malloc(sizeof(struct thread_data));
    data->wait_to_obtain_mutex_in_ms = wait_to_obtain_ms;
    data->wait_to_release_mutex_in_ms = wait_to_release_ms;

    data->mutex = mutex;

    int pthread_create_ret = pthread_create(thread, NULL, threadfunc, (void *) data);
    if (pthread_create_ret != 0) {
        errno = pthread_create_ret;
        perror("pthread_create");
        return false;
    }

    printf("Thread with ID %ld created\n", *thread);

    return true;
}
