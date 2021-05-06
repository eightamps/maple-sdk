#ifndef __MAPLE_THREAD_WRAP_H__
#define __MAPLE_THREAD_WRAP_H__

typedef int thread_wrap_t;
typedef struct thread_wrap_attr_t_ *thread_wrap_attr_t;

int thread_wrap_create(thread_wrap_t *tid,
    const thread_wrap_attr_t *attr,
    void *(*start) (void *),
    void *arg);
int thread_wrap_cancel (thread_wrap_t thread);
void thread_wrap_exit(void *value_ptr);
int thread_wrap_join(thread_wrap_t thread, void **value_ptr);

#endif // __MAPLE_THREAD_WRAP_H__