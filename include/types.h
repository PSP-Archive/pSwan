//////////////////////////////////////////////////////////////////////////////
// moved from ../source
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __TYPES_H__
#define __TYPES_H__

#define UINT8	BYTE
#define UINT16	WORD
#define INT8	signed char
#define INT16	signed short
#define INT32	signed long
#define UINT32	unsigned long

#define uint8	UINT8
#define uint16	UINT16
#define uint32	UINT32
#define int8	INT8
#define int16	INT16
#define int32	INT32

#define u8		uint8
#define u16		uint16
#define u32		uint32
#define s8		int8
#define s16		int16
#define s32		int32

#if 1
typedef struct
{
	char vercnf[16];
	int screensize;
	int vsync;
	int state_slot;
	int sound;
	int sound_part[7];
	int sampling_rate;
	int sound_buffer ;
	int key_config[30]; //—]•ª‚ÉŠm•Û
	int analog2dpad;
	unsigned long color[4];
	int bgbright;
	int wait;
	int frameskip;
} SETTING;

#endif

#endif
