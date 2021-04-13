/******************************************************************************
 *                                                                            *
 *      inifile.h                                                             *
 *                                                                            *
 *      INIファイル処理関数                                                   *
 *                                                                            *
 ******************************************************************************/

#ifndef _INIFILE_H
#define _INIFILE_H

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		プロトタイプ宣言
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __cplusplus
extern "C" {
#endif
void ini_open(const char *fname);
void ini_close(void);
void ini_writeinteger(const char *section, const char *ident, int value);
void ini_writelong(const char *section, const char *ident, long value);
void ini_writestring(const char *section, const char *ident, const char *value);
int ini_readinteger(const char *section, const char *ident, int value);
long ini_readlong(const char *section, const char *ident, long value);
char *ini_readstring(const char *section, const char *ident, const char *value, char *data);
#ifdef __cplusplus
}
#endif

#endif
