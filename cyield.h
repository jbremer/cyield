#ifndef __CYIELD__
#define __CYIELD__

#include <windows.h>

typedef union _cyield_param_t {
	unsigned long ul;
	void         *ptr;
	const char   *psz;
} cyield_param_t;

typedef struct _cyield_t {
	HANDLE hEvent;
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

#ifdef __GNUC__
#define _CYIELD_ASM asm(".byte 0xeb \n .byte 0xfe");
#else
#define _CYIELD_ASM __asm _emit 0xeb __asm _emit 0xfe
#endif

#define CYIELD(ptr) { \
	/* set the argument */ \
	self->yield.ul = (unsigned long)(ptr); \
	/* signal the callee */ \
	SetEvent(self->hEvent); \
	/* while(1) loop */ \
	_CYIELD_ASM }

#define CYIELD_FOREACH(type, name, callback, arg0, arg1, arg2, arg3) { \
	/* create event and initialize cyield_t object */ \
	cyield_t _yield = {CreateEvent(NULL, FALSE, FALSE, NULL), { \
	(unsigned long)(arg0), (unsigned long)(arg1), \
	(unsigned long)(arg2), (unsigned long)(arg3)}}; \
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
	/* increase Eip with 2, */ \
	/* steps over the while(1) loop */ \
	SuspendThread(_hThread); \
	CONTEXT _ctx = {CONTEXT_FULL}; \
	GetThreadContext(_hThread, &_ctx); \
	_ctx.Eip += 2; \
	SetThreadContext(_hThread, &_ctx); \
	ResumeThread(_hThread); } \
	/* close the event handle */ \
	CloseHandle(_yield.hEvent); \
	/* close thread handle */ \
	CloseHandle(_hThread); }

#endif
