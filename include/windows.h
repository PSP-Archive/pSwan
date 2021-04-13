#ifndef _WINDOWS_H_
#define _WINDOWS_H_

#ifdef __cplusplus
extern "C" {
#endif

// stdio.h
#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

// windows.h
#define VOID void
typedef void *PVOID;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef int                 INT32;
typedef unsigned int        UINT32;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;

// windef.h
#define far
#define near

#define pascal

#undef FAR
#undef NEAR
#define FAR                 far
#define NEAR                near
#ifndef CONST
#define CONST               const
#endif

#define PASCAL      pascal

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef BOOL                boolean;
typedef boolean             bool;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void far            *LPVOID;
typedef CONST void far      *LPCVOID;

typedef BYTE                BOOLEAN;




#define AIAPI
typedef CHAR*           LPCHAR;
typedef UINT*           LPUINT;





typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT, *PRECT, NEAR *NPRECT, FAR *LPRECT;


#define CALLBACK
#define WINAPI

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

typedef PVOID HANDLE;
typedef LONG  HRESULT;

typedef CONST CHAR *LPCSTR, *PCSTR;
typedef LPCSTR LPCTSTR;

#define DECLARE_HANDLE(name) typedef HANDLE name
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HKEY);

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef UINT WPARAM;
typedef LONG LPARAM;
typedef LONG LRESULT;

// winnt.h
typedef CHAR *LPSTR, *PSTR;



// winbase.h
#define WINBASEAPI



#ifdef __cplusplus
}
#endif

#endif // _WINDOWS_H_
