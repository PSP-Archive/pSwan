#ifndef __SOUND_H__
#define __SOUND_H__
#include <windows.h>
#include <define.h>

#define WAVFILEMAX_BG 1*1024*1024 // 8*1024*1024
#define PGA_CHANNELS	1	//3
#define PGA_SAMPLES		512	//256
#define MAXVOLUME 0x8000

#define SAMPLING_RATE 44100
#define MILLISEC 20
#define SOUND_BUF_LEN SAMPLING_RATE*2*2 * MILLISEC/1000 *2

typedef struct {
	unsigned long channels;
	unsigned long samplerate;
	unsigned long samplecount;
	unsigned long datalength;
	char *wavdata;
	unsigned long rateratio;		// samplerate / 44100 * 0x10000
	unsigned long playptr;
	unsigned long playptr_frac;
	int playloop;
} wavout_wavinfo_t;

extern wavout_wavinfo_t wavinfo_bg;
extern char wavdata_bg[WAVFILEMAX_BG];
extern wavout_wavinfo_t *wavout_snd0_wavinfo;
extern unsigned long cur_play;
extern unsigned long cur_nloop;
// extern unsigned char sound_buf[44100*16/8*SOUND_BUF_LEN/1000 * 2];
extern int pga_handle[PGA_CHANNELS];
extern unsigned long cur_play;
extern BYTE sound_buf[SOUND_BUF_LEN];

#endif // __SOUND_H__
