/* system call prototype for PSP */
#ifndef _SYSCALL_H_INCLUDED
#define _SYSCALL_H_INCLUDED

#define POWER_CB_POWER		0x80000000 
#define POWER_CB_HOLDON		0x40000000 
#define POWER_CB_STANDBY	0x00080000 
#define POWER_CB_RESCOMP	0x00040000 
#define POWER_CB_RESUME		0x00020000 
#define POWER_CB_SUSPEND	0x00010000 
#define POWER_CB_EXT		0x00001000 
#define POWER_CB_BATLOW		0x00000100 
#define POWER_CB_BATTERY	 0x00000080 
#define POWER_CB_BATTPOWER	0x0000007F 

#include "../csdlibrary/api-include/IoFileMgrForUser.h"
#include "../csdlibrary/api-include/LoadExecForUser.h"
#include "../csdlibrary/api-include/sceCtrl.h"
#include "../csdlibrary/api-include/sceDisplay.h"
#include "../csdlibrary/api-include/scePower.h"
#include "../csdlibrary/api-include/ThreadManForUser.h"
#include "../csdlibrary/api-include/UtilsForUser.h"

#endif // _SYSCALL_H_INCLUDED
