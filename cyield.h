#ifndef __CYIELD__
#define __CYIELD__

#ifndef __linux__

#include <windows.h>

typedef union _cyield_param_t {
    unsigned long ul;
    void         *ptr;
    const char   *psz;
} cyield_param_t;

typedef struct _cyield_t {
    HANDLE hEvent;
    HANDLE hContinue;
    cyield_param_t param[4];
    cyield_param_t yield;
    int quit;
} cyield_t;

#define CYIELD_FUNC(f) \
    DWORD WINAPI f(cyield_t *self) { \
    cyield_param_t *arg0 = &self->param[0]; \
    cyield_param_t *arg1 = &self->param[1]; \
    cyield_param_t *arg2 = &self->param[2]; \
    cyield_param_t *arg3 = &self->param[3];

#define CYIELD_FUNC_END(ret) \
    self->quit = 1; \
    SetEvent(self->hEvent); \
    return ret; }

#define CYIELD(ptr) { \
    /* set the argument */ \
    self->yield.ul = (unsigned long)(ptr); \
    /* signal the callee and wait */ \
    SignalObjectAndWait(self->hEvent, self->hContinue, INFINITE, FALSE); }

#define CYIELD_FOREACH(type, name, callback, arg0, arg1, arg2, arg3) { \
    /* create event and initialize cyield_t object */ \
    cyield_t _yield = { \
        CreateEvent(NULL, FALSE, FALSE, NULL), \
        CreateEvent(NULL, FALSE, FALSE, NULL), { \
        (unsigned long)(arg0), (unsigned long)(arg1), \
        (unsigned long)(arg2), (unsigned long)(arg3), \
    }}; \
    /* create a thread, where all magic will happen */ \
    HANDLE _hThread = CreateThread(NULL, 0, \
        (LPTHREAD_START_ROUTINE)(callback), &_yield, 0, NULL); \
    while (WaitForSingleObject(_yield.hEvent, INFINITE) == \
            WAIT_OBJECT_0 && _yield.quit == 0) { \
        type name = (type) _yield.yield.ptr;

#define CYIELD_FOREACH1(type, name, callback, arg0) \
    CYIELD_FOREACH(type, name, callback, arg0, 0, 0, 0)

#define CYIELD_FOREACH2(type, name, callback, arg0, arg1) \
    CYIELD_FOREACH(type, name, callback, arg0, arg1, 0, 0)

#define CYIELD_FOREACH3(type, name, callback, arg0, arg1, arg2) \
    CYIELD_FOREACH(type, name, callback, arg0, arg1, arg2, 0)

#define CYIELD_FOREACH_NEXT() \
    /* signal the continue event */ \
    SetEvent(_yield.hContinue); } \
    /* close the event handle */ \
    CloseHandle(_yield.hEvent); \
    /* close the continue handle */ \
    CloseHandle(_yield.hContinue); \
    /* close thread handle */ \
    CloseHandle(_hThread); }

#else

#include <pthread.h>

typedef union _cyield_param_t {
    unsigned long ul;
    void         *ptr;
    const char   *psz;
} cyield_param_t;

typedef struct _cyield_t {
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    cyield_param_t param[4];
    cyield_param_t yield;
    int quit;
} cyield_t;

#define CYIELD_FUNC(f) \
    void *f(cyield_t *self) { \
    cyield_param_t *arg0 = &self->param[0]; \
    cyield_param_t *arg1 = &self->param[1]; \
    cyield_param_t *arg2 = &self->param[2]; \
    cyield_param_t *arg3 = &self->param[3]; \
    pthread_mutex_lock(self->mutex); \
    self->quit = 0;

#define CYIELD_FUNC_END(ret) \
    self->quit = 1; \
    pthread_cond_signal(self->cond); \
    pthread_mutex_unlock(self->mutex); \
    return (void *)(ret); }

#define CYIELD(ptr) { \
    /* set the argument */ \
    self->yield.ul = (unsigned long)(ptr); \
    /* signal the callee and wait */ \
    pthread_cond_signal(self->cond); \
    pthread_cond_wait(self->cond, self->mutex); }

#define CYIELD_FOREACH(type, name, callback, arg0, arg1, arg2, arg3) { \
    /* create event and initialize cyield_t object */ \
    pthread_mutex_t _mutex; pthread_cond_t _cond; \
    pthread_mutex_init(&_mutex, NULL); \
    pthread_cond_init(&_cond, NULL); \
    cyield_t _yield = { \
        &_mutex, &_cond, \
        (unsigned long)(arg0), (unsigned long)(arg1), \
        (unsigned long)(arg2), (unsigned long)(arg3), \
    }; \
    /* create a thread, where all magic will happen */ \
    pthread_t _thread; \
    pthread_create(&_thread, NULL, (void *(*)(void *))(callback), &_yield); \
    while (pthread_cond_wait(&_cond, &_mutex) == 0 && _yield.quit == 0) { \
        type name = (type)(unsigned long) _yield.yield.ptr;

#define CYIELD_FOREACH1(type, name, callback, arg0) \
    CYIELD_FOREACH(type, name, callback, arg0, 0, 0, 0)

#define CYIELD_FOREACH2(type, name, callback, arg0, arg1) \
    CYIELD_FOREACH(type, name, callback, arg0, arg1, 0, 0)

#define CYIELD_FOREACH3(type, name, callback, arg0, arg1, arg2) \
    CYIELD_FOREACH(type, name, callback, arg0, arg1, arg2, 0)

#define CYIELD_FOREACH_NEXT() \
    /* signal the continue event */ \
    pthread_cond_signal(&_cond); } \
    /* close the event handle */ \
    pthread_mutex_destroy(&_mutex); \
    /* close the continue handle */ \
    pthread_cond_destroy(&_cond); \
    /* close thread handle */ \
    pthread_join(_thread, NULL); }

#endif

#endif
