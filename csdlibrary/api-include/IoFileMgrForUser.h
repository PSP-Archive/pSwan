/******************************************************************************
 *                                                                            *
 *      IoFileMgrForUser.h                                                    *
 *                                                                            *
 *      module sceIOFileManager                                               *
 *                                                                            *
 ******************************************************************************/

#ifndef _IOFILEMGRFORUSER_H
#define _IOFILEMGRFORUSER_H
//-----------------------------------------------------------------------------

#define O_RDONLY	0x0001
#define O_WRONLY	0x0002
#define O_RDWR		0x0003
#define O_NBLOCK	0x0010
#define O_APPEND	0x0100
#define O_CREAT		0x0200
#define O_TRUNC		0x0400
#define O_NOWAIT	0x8000
//-----------------------------------------------------------------------------

enum {
	TYPE_DIR=0x10,
	TYPE_FILE=0x20
};

#if 0
// dirent seems to have a little larger size.  you'd better to pad something after the struct.
typedef struct dirent {
	unsigned long unk0;
	unsigned long type;
	unsigned long size;
	unsigned long unk[19];
	char name[0x108];
} dirent_t;
#else
typedef struct dirent_tm {
	unsigned short unk[2]; // 0?
	unsigned short year;
	unsigned short mon;
	unsigned short mday;
	unsigned short hour;
	unsigned short min;
	unsigned short sec;
} dirent_tm_t;

typedef struct dirent {
    unsigned long unk0;
    unsigned long type;
    unsigned long size;
	struct dirent_tm ctime; // creation date
	struct dirent_tm atime; // last access date
	struct dirent_tm mtime; // last update date
	unsigned long unk[7];             // 0?
    char name[0x108];
} dirent_t;
#endif

//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------

void sceIoClose(int fd);
//-----------------------------------------------------------------------------

// may not be correct.  need to confirm.  by this interface file can not be created
//int sceIoOpen(const char *file, int mode);

// by this interface with unknown=0777, file can be created
int sceIoOpen(const char *file, int mode, int unknown);

// actual interface is unknown!
//-----------------------------------------------------------------------------

int sceIoRead(int fd, void *data, int size);
//-----------------------------------------------------------------------------

int sceIoWrite(int fd, void *data, int size);
//-----------------------------------------------------------------------------

int sceIoLseek(int fd, long long offset, int whence);
//-----------------------------------------------------------------------------

int sceIoDopen(const char *fn);
//-----------------------------------------------------------------------------

int sceIoDread(int fd, dirent_t *de);
//-----------------------------------------------------------------------------

void sceIoDclose(int fd);
//-----------------------------------------------------------------------------

int sceIoRemove(const char *file);
//-----------------------------------------------------------------------------

// may not be correct.  need to confirm
int sceIoMkdir(const char *dir, int mode);
//-----------------------------------------------------------------------------

int sceIoRmdir(const char *dir);
//-----------------------------------------------------------------------------

int sceIoRename(const char *oldname, const char *newname);
//-----------------------------------------------------------------------------

//sceIoDevctl
//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
