/******************************************************************************
 *                                                                            *
 *      msio.h                                                                *
 *                                                                            *
 *      メモリースティック入出力関数                                          *
 *                                                                            *
 ******************************************************************************/

#ifndef _MSIO_H
#define _MSIO_H

#define __need_size_t
#include <stddef.h>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		ファイル位置移動（ms_fseek whence）
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef SEEK_SET
#define SEEK_SET	0						// 先頭
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	1						// 現在の位置
#endif
#ifndef SEEK_END
#define SEEK_END	2						// 終端
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		プロトタイプ宣言
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __cplusplus
extern "C" {
#endif
void ms_fclose(int fd);
int ms_fgetc(int fd);
char *ms_fgets(char *s, int n, int fd);
int ms_fopen(const char *path, const char *mode);
int ms_fprintf(int fd, const char *format, ...);
int ms_fputc(int c, int fd);
int ms_fputs(const char *s, int fd);
size_t ms_fread(void *ptr, size_t size, size_t n, int fd);
int ms_freopen(const char *path, const char *mode, int fd);
int ms_fseek(int fd, long offset, int whence);
long ms_ftell(int fd);
size_t ms_fwrite(const void *ptr, size_t size, size_t n, int fd);
int ms_remove(const char *path);
int ms_rename(const char *oldname, const char *newname);
void ms_rewind(int fd);
#ifdef __cplusplus
}
#endif

#endif
