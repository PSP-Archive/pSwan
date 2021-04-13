/******************************************************************************
 *                                                                            *
 *      ThreadManForUser.h                                                    *
 *                                                                            *
 *      module sceThreadManager                                               *
 *                                                                            *
 ******************************************************************************/

#ifndef _THREADMANFORUSER_H
#define _THREADMANFORUSER_H
//-----------------------------------------------------------------------------

typedef int (*pg_threadfunc_t)(int args, void *argp);
//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------

int sceKernelCreateCallback(const char *name, void *func);
//-----------------------------------------------------------------------------

//sceKernelSleepThread
//-----------------------------------------------------------------------------

void sceKernelPollCallbacks(void);
//-----------------------------------------------------------------------------

int sceKernelWaitThreadEnd(int hthread, void *unk);
//-----------------------------------------------------------------------------

//sceKernelCreateEventFlag
//-----------------------------------------------------------------------------

//sceKernelDeleteEventFlag
//-----------------------------------------------------------------------------

//sceKernelSetEventFlag
//-----------------------------------------------------------------------------

//sceKernelClearEventFlag
//-----------------------------------------------------------------------------

//sceKernelWaitEventFlag
//-----------------------------------------------------------------------------

int sceKernelCreateThread(const char *name, pg_threadfunc_t func, unsigned long priority, unsigned long stacksize, unsigned long attrib, void *unk);
//-----------------------------------------------------------------------------

int sceKernelDeleteThread(int hthread);
//-----------------------------------------------------------------------------

int sceKernelStartThread(int hthread, int arg0, void *arg1);
//-----------------------------------------------------------------------------

void sceKernelExitThread(int ret);
//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
