/******************************************************************************
 *                                                                            *
 *      jctype.h                                                              *
 *                                                                            *
 *      日本語処理関数                                                        *
 *                                                                            *
 ******************************************************************************/

#ifndef _JCTYPE_H
#define _JCTYPE_H

#ifndef _CTYPE_H_
#include <ctype.h>
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		プロトタイプ宣言
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __cplusplus
extern "C" {
#endif
int iskmoji(int c);
int iskpun(int c);
int iskana(int c);
int isalkana(int c);
int ispnkana(int c);
int isalnmkana(int c);
int isprkana(int c);
int isgrkana(int c);
int isjis(int c);
int issjis1(int c);
int issjis2(int c);
int iskuten1(int c);
int iskuten2(int c);
int jisjis(unsigned short c);
int jissjis(unsigned short c);
int jiskuten(unsigned short c);
int jisl0(unsigned short c);
int jisl1(unsigned short c);
int jisl2(unsigned short c);
int jisalpha(unsigned short c);
int jisupper(unsigned short c);
int jislower(unsigned short c);
int jisdigit(unsigned short c);
int jiskana(unsigned short c);
int jishira(unsigned short c);
int jiskigou(unsigned short c);
int jisspace(unsigned short c);
//unsigned short jtolower(unsigned short c);
//unsigned short jtoupper(unsigned short c);
//unsigned short jtohira(unsigned short c);
//unsigned short jtokana(unsigned short c);
//unsigned short zentohan(unsigned short c);
//unsigned short hantozen(unsigned short c);
unsigned short sjistojis(unsigned short c);
unsigned short jistosjis(unsigned short c);
unsigned short jistokuten(unsigned short c);
unsigned short kutentojis(unsigned short c);
unsigned short sjistokuten(unsigned short c);
unsigned short kutentosjis(unsigned short c);
#ifdef __cplusplus
}
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		マクロ定義
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define iskmoji(c)		( c >= 0xa6 && c <= 0xdf )
#define iskpun(c) 		( c >= 0xa1 && c <= 0xa5 )
#define iskana(c) 		( c >= 0xa1 && c <= 0xdf )
#define isalkana(c)		( isalpha(c) || iskmoji(c) )
#define ispnkana(c)		( ispunct(c) || iskpun(c) )
#define isalnmkana(c)	( isalnum(c) || iskmoji(c) )
#define isprkana(c)		( isprint(c) || iskana(c) )
#define isgrkana(c)		( isgraph(c) || iskana(c) )
#define isjis(c) 		( c >= 0x21 && c <= 0x7e )
#define issjis1(c)		( ( c >= 0x81 && c <= 0x9f ) || ( c >= 0xe0 && c <= 0xfc ) )
#define issjis2(c)		( ( c >= 0x40 && c <= 0x7e ) || ( c >= 0x80 && c <= 0xfc ) )
#define iskuten1(c)		( ( c >= 1 && c <= 8 ) || c == 13 || ( c >= 16 && c <= 84 ) )
#define iskuten2(c)		( c >= 1 && c <= 94 )

#endif
