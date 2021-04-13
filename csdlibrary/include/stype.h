/******************************************************************************
 *                                                                            *
 *      stype.h                                                               *
 *                                                                            *
 *      文字列処理関数                                                        *
 *                                                                            *
 ******************************************************************************/

#ifndef _STYPE_H
#define _STYPE_H

#define __need_size_t
#include <stddef.h>

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		文字判定モード（mischar mode, sischar mode）
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// 論理式
#define ISL_OR			0x00000000			// 全判定の論理和
#define ISL_AND			0x10000000			// 全判定の論理積
#define ISL_ORNOT		0x20000000			// 全判定の論理和の否定
#define ISL_ANDNOT		0x30000000			// 全判定の論理積の否定

// 判定
#define ISJ_SPACE		0x00000001			// 間隔文字
#define ISJ_DIGIT		0x00000002			// 数字
#define ISJ_UPPER		0x00000004			// 英大文字
#define ISJ_LOWER		0x00000008			// 英小文字
#define ISJ_XDIGIT		0x00000010			// 16進表記文字
#define ISJ_CNTRL		0x00000020			// 制御文字
#define ISJ_PUNCT		0x00000040			// 句切り文字
#define ISJ_BLANK		0x00000080			// 空白文字
#define ISJ_KMOJI		0x00000100			// 句読点以外の半角カナ文字
#define ISJ_KPUN		0x00000200			// 半角カナ文字の句読点
#define ISJ_JJIS		0x00000400			// JISコード
#define ISJ_JSJIS		0x00000800			// シフトJISコード
#define ISJ_JKUTEN		0x00001000			// 区点コード
#define ISJ_JL0			0x00002000			// シフトJISの漢字以外の文字
#define ISJ_JL1			0x00004000			// シフトJIS第一水準
#define ISJ_JL2			0x00008000			// シフトJIS第二水準
#define ISJ_JALPHA		0x00010000			// シフトJISの英文字
#define ISJ_JUPPER		0x00020000			// シフトJISの英大文字
#define ISJ_JLOWER		0x00040000			// シフトJISの英小文字
#define ISJ_JDIGIT		0x00080000			// シフトJISの数字
#define ISJ_JKANA		0x00100000			// シフトJISのカタカナ
#define ISJ_JHIRA		0x00200000			// シフトJISのひらがな
#define ISJ_JKIGOU		0x00400000			// シフトJISの記号
#define ISJ_JSPACE		0x00800000			// シフトJISのスペース

#define ISJ_ALPHA		ISJ_UPPER | ISJ_LOWER	// 英文字
#define ISJ_ALNUM		ISJ_DIGIT | ISJ_ALPHA	// 英文字と数字
#define ISJ_GRAPH		ISJ_ALNUM | ISJ_PUNCT	// 空白以外の表示可能文字
#define ISJ_PRINT		ISJ_GRAPH | ISJ_BLANK	// 表示可能文字
#define ISJ_ASCII		ISJ_PRINT | ISJ_CNTRL	// ASCII文字
#define ISJ_KANA		ISJ_KMOJI | ISJ_KPUN	// 半角のカナ文字
#define ISJ_ALKANA		ISJ_ALPHA | ISJ_KMOJI	// 英文字とカナ文字
#define ISJ_PNKANA		ISJ_PUNCT | ISJ_KPUN	// 英字の句読点とカナ文字の句読点
#define ISJ_ALNMKANA	ISJ_ALNUM | ISJ_KMOJI	// 英文字と数字とカナ文字
#define ISJ_PRKANA		ISJ_PRINT | ISJ_KANA	// 表示可能文字とカナ文字
#define ISJ_GRKANA		ISJ_GRAPH | ISJ_KANA	// 空白以外の表示可能文字とカナ文字
#define ISJ_PRKASJ		ISJ_PRKANA | ISJ_JSJIS	// 表示可能文字とカナ文字とシフトJIS

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		文字判定以外対象（mischar except, sischar except）
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define ISE_NONE		0x00000000			// なし
#define ISE_SPACE		0x00000001			// 間隔文字
#define ISE_DIGIT		0x00000002			// 数字
#define ISE_UPPER		0x00000004			// 英大文字
#define ISE_LOWER		0x00000008			// 英小文字
#define ISE_XDIGIT		0x00000010			// 16進表記文字
#define ISE_CNTRL		0x00000020			// 制御文字
#define ISE_PUNCT		0x00000040			// 句切り文字
#define ISE_BLANK		0x00000080			// 空白文字
#define ISE_KMOJI		0x00000100			// 句読点以外の半角カナ文字
#define ISE_KPUN		0x00000200			// 半角カナ文字の句読点
#define ISE_JJIS		0x00000400			// JISコード
#define ISE_JSJIS		0x00000800			// シフトJISコード
#define ISE_JKUTEN		0x00001000			// 区点コード
#define ISE_JL0			0x00002000			// シフトJISの漢字以外の文字
#define ISE_JL1			0x00004000			// シフトJIS第一水準
#define ISE_JL2			0x00008000			// シフトJIS第二水準
#define ISE_JALPHA		0x00010000			// シフトJISの英文字
#define ISE_JUPPER		0x00020000			// シフトJISの英大文字
#define ISE_JLOWER		0x00040000			// シフトJISの英小文字
#define ISE_JDIGIT		0x00080000			// シフトJISの数字
#define ISE_JKANA		0x00100000			// シフトJISのカタカナ
#define ISE_JHIRA		0x00200000			// シフトJISのひらがな
#define ISE_JKIGOU		0x00400000			// シフトJISの記号
#define ISE_JSPACE		0x00800000			// シフトJISのスペース
#define ISE_ALL			0x00ffffff			// 全て

#define ISE_ALPHA		ISE_UPPER | ISE_LOWER	// 英文字
#define ISE_ALNUM		ISE_DIGIT | ISE_ALPHA	// 英文字と数字
#define ISE_GRAPH		ISE_ALNUM | ISE_PUNCT	// 空白以外の表示可能文字
#define ISE_PRINT		ISE_GRAPH | ISE_BLANK	// 表示可能文字
#define ISE_ASCII		ISE_PRINT | ISE_CNTRL	// ASCII文字
#define ISE_KANA		ISE_KMOJI | ISE_KPUN	// 半角のカナ文字
#define ISE_ALKANA		ISE_ALPHA | ISE_KMOJI	// 英文字とカナ文字
#define ISE_PNKANA		ISE_PUNCT | ISE_KPUN	// 英字の句読点とカナ文字の句読点
#define ISE_ALNMKANA	ISE_ALNUM | ISE_KMOJI	// 英文字と数字とカナ文字
#define ISE_PRKANA		ISE_PRINT | ISE_KANA	// 表示可能文字とカナ文字
#define ISE_GRKANA		ISE_GRAPH | ISE_KANA	// 空白以外の表示可能文字とカナ文字
#define ISE_PRKASJ		ISJ_PRKANA | ISJ_JSJIS	// 表示可能文字とカナ文字とシフトJIS

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
		プロトタイプ宣言
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __cplusplus
extern "C" {
#endif
void *mischar(const void *mem, size_t size, long mode, long except);
void *misrange(const void *mem, size_t size, unsigned long min, unsigned long max);
void *misxrange(const void *mem, size_t size, unsigned long min, unsigned long max);
int sischar(const char *str, long mode, long except);
int sisrange(const char *str, unsigned long min, unsigned long max);
int sisxrange(const char *str, unsigned long min, unsigned long max);
int ttostr(char *str, const char *mode, const void *value);
int strtot(const char *str, const char *mode, void *value);
void strltrim(char *str);
void strrtrim(char *str);
void strtrim(char *str);
void strleft(char *dest, const char *src, size_t len);
void strmid(char *dest, const char *src, int start, size_t len);
void strright(char *dest, const char *src, size_t len);
#ifdef __cplusplus
}
#endif

#endif
