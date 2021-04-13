#include <stdio.h>
#include <sound.h>
#include <syscall.h>
#include <define.h>
#include <pg.h>
#include "../oswan/source/seal/audiow32.h"

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#define SND1_MAXSLOT 16
wavout_wavinfo_t wavinfo_bg;
char wavdata_bg[WAVFILEMAX_BG];
wavout_wavinfo_t *wavout_snd0_wavinfo=0;
int wavout_snd0_ready=0;
int wavout_snd0_playend=0;
unsigned long wavout_snd0_playptr=0;
int wavout_snd1_playing[SND1_MAXSLOT];
unsigned long cur_play;
unsigned long cur_nloop;

/******************************************************************************/
int pga_ready=0;
int pga_handle[PGA_CHANNELS];
void (*pga_channel_callback[PGA_CHANNELS])(void *buf, unsigned long reqn);
int pga_threadhandle[PGA_CHANNELS];
volatile int pga_terminate=0;
short pga_sndbuf[PGA_CHANNELS][2][PGA_SAMPLES][2];  // 出力バッファ

BYTE sound_buf[SOUND_BUF_LEN];

static int pga_channel_thread(int args, void *argp)
{
	volatile int cur_pos  = 0;
	volatile int bufidx=0;
	int channel=*(int *)argp;
	char *wavptr;
	int size = 44100 * 2 * 2;

	unsigned int i, j, pos;
	int    d, dd;
	unsigned int volume = 0x8000>>2;
	int    len;
	
	while (pga_terminate==0) {
		// 波形の作成
		void *bufptr=&pga_sndbuf[channel][bufidx];
		void (*callback)(void *buf, unsigned long reqn);
		callback=pga_channel_callback[channel];
		if (callback) {
			callback(bufptr,PGA_SAMPLES);
		} else {
			unsigned long *ptr=bufptr;
			int i;
			for (i=0; i<PGA_SAMPLES; ++i) *(ptr++)=0;
		}
		wavoutBlock(channel,volume,volume,bufptr);
		bufidx=(bufidx?0:1);
	}
	sceKernelExitThread(0);
	return 0;
}

void wavoutSetChannelCallback(int channel, void *callback)
{
	pga_channel_callback[channel]=callback;
}

int wavoutBlock(unsigned long channel,unsigned long vol1,unsigned long vol2,void *buf)
{
	if (!pga_ready) return -1;
	if (channel>=PGA_CHANNELS) return -1;
	if (vol1>MAXVOLUME) vol1=MAXVOLUME;
	if (vol2>MAXVOLUME) vol2=MAXVOLUME;
	return sceAudio_2(pga_handle[channel],vol1,vol2,buf);
}

//バッファは64バイト境界じゃなくても大丈夫みたい
//[0]が左、[1]が右
//サンプル速度は44100
//vol1が左
static void wavout_snd0_callback(short *_buf, unsigned long _reqn)
{
#ifdef DEBUG
char buff[128];
if (cur_buf_pos > 2) {
sprintf(buff, "pre_buf_pos = %d", pre_buf_pos);
pgDebug(buff, 0);
sprintf(buff, "cur_buf_pos = %d", cur_buf_pos);
pgDebug(buff, 1);
sprintf(buff, "freq = %d", voice_seq[TEST_CHANNEL][cur_buf_pos-2].dwFrequency);
pgDebug(buff, 2);
/*
sprintf(buff, "wi = %d", wavout_snd0_wavinfo);
pgDebug(buff, 3);
sprintf(buff, "sound_buf[0] = %d", sound_buf[0]);
pgDebug(buff, 4);
*/
}
#endif
	static int power[128];
	
	unsigned long i;
	unsigned long ptr,frac,rr,max;
	int channels;
	char *src;
	short *buf=_buf;
	unsigned long reqn=_reqn;
	
	wavout_wavinfo_t *wi=wavout_snd0_wavinfo;

	if (wi==0) {
		wavout_snd0_ready=1;
		memset(buf,0,reqn*4);
		return;
	}
	
	wavout_snd0_ready=0;
	
	ptr=wi->playptr;
	frac=wi->playptr_frac;
	rr=wi->rateratio;
	max=wi->samplecount;
	channels=wi->channels;
	src=wi->wavdata;
	
	cur_nloop=0;
	for (; reqn>0; --reqn) {
		frac+=rr;
		ptr+=(frac>>16);
		cur_play = ptr*4;
		frac&=0xffff;
		if (ptr>=max) {
			if (wi->playloop) {
				ptr=0;
				cur_nloop++;
			} else {
				for (; reqn>0; --reqn) {
					*(buf++)=0;
					*(buf++)=0;
				}
				goto playend;
			}
		}
		if (channels==1) {
			buf[0]=buf[1]=*(short *)(src+ptr*2);
			buf+=2;
		} else {
			buf[0]=*(short *)(src+ptr*4);
			buf[1]=*(short *)(src+ptr*4+2);
			buf+=2;
		}
	}

//	powercalc(_buf);	//単にwaveを出すだけなら不要

	wavout_snd0_playptr=ptr;
	wi->playptr=ptr;
	wi->playptr_frac=frac;
	return;
	
playend:
	wavout_snd0_playend=1;
	return;
}


static int pga_init()
{
	int i,ret;
	int failed=0;
	char str[32];

	pga_terminate=0;
	pga_ready=0;

	for (i=0; i<PGA_CHANNELS; i++) {
		pga_handle[i]=-1;
		pga_threadhandle[i]=-1;
		pga_channel_callback[i]=0;
	}
	for (i=0; i<PGA_CHANNELS; i++) {
		if ((pga_handle[i]=sceAudio_3(-1,PGA_SAMPLES,0))<0) failed=1;
	}
	if (failed) {
		for (i=0; i<PGA_CHANNELS; i++) {
			if (pga_handle[i]!=-1) sceAudio_4(pga_handle[i]);
			pga_handle[i]=-1;
		}
		return -1;
	}
	pga_ready=1;

	strcpy(str,"pgasnd0");
	for (i=0; i<PGA_CHANNELS; i++) {
		str[6]='0'+i;
		pga_threadhandle[i]=sceKernelCreateThread(str,(pg_threadfunc_t)&pga_channel_thread,0x12,0x10000,0,NULL);
		if (pga_threadhandle[i]<0) {
			pga_threadhandle[i]=-1;
			failed=1;
			break;
		}
		ret=sceKernelStartThread(pga_threadhandle[i],sizeof(i),&i);
		if (ret!=0) {
			failed=1;
			break;
		}
	}
	if (failed) {
		pga_terminate=1;
		for (i=0; i<PGA_CHANNELS; i++) {
			if (pga_threadhandle[i]!=-1) {
				sceKernelWaitThreadEnd(pga_threadhandle[i],NULL);
				sceKernelDeleteThread(pga_threadhandle[i]);
			}
			pga_threadhandle[i]=-1;
		}
		pga_ready=0;
		return -1;
	}
	return 0;
}

int wavoutInit()
{
	wavout_snd0_wavinfo=0;

	if (pga_init()) return -1;

	int i;
	for (i=0; i<SND1_MAXSLOT; i++) {
		wavout_snd1_playing[i]=0;
	}

	wavoutSetChannelCallback(0,wavout_snd0_callback);
	//pgaSetChannelCallback(1,wavout_snd1_callback);


	int datalength = SAMPLING_RATE*2*2 * MILLISEC/1000;
	wavinfo_bg.channels=2;
	wavinfo_bg.samplerate=44100;
	wavinfo_bg.samplecount=datalength/4;
	wavinfo_bg.datalength=datalength;
	wavinfo_bg.wavdata=sound_buf;
	wavinfo_bg.rateratio=(44100*0x4000)/11025/2;
	wavinfo_bg.playptr=0;
	wavinfo_bg.playptr_frac=0;
	wavinfo_bg.playloop=1;

	return 0;
}

void wavoutStopPlay0()
{
	if (wavout_snd0_wavinfo!=0) {
		while (wavout_snd0_ready) pgWaitV();
		wavout_snd0_wavinfo=0;
		while (!wavout_snd0_ready) pgWaitV();
	}
	memset(sound_buf, 0x00, sizeof(sound_buf));
}

void wavoutStartPlay0(wavout_wavinfo_t *wi)
{
	wavoutStopPlay0();
	while (!wavout_snd0_ready) pgWaitV();
	wavout_snd0_playptr=0;
	wavout_snd0_playend=0;
	wavout_snd0_wavinfo=wi;
	while (wavout_snd0_ready) pgWaitV();
}

int wavoutWaitEnd0()
{
	if (wavout_snd0_wavinfo==0) return -1;
	if (wavout_snd0_wavinfo->playloop) return -1;
	while (!wavout_snd0_playend) pgWaitV();
	return 0;
}

void renderer_update_sound()
{
	//sound_buf: 0.16秒分のサウンドバッファ
	//cur_play:  バッファの現在再生位置	
	static int size=SOUND_BUF_LEN; // バッファ長
	static int bef=1;
	static int n=0;

#if 0
//	if(setting.sound){
		if (bef!=(cur_play*2/size)){
			bef=(cur_play*2/size);
			// 0.8秒分の波形データを生成
			snd_render((short*)&sound_buf[(!bef)?size/2:0], size/2);
		}
//	}
#else
	snd_render((short*)&sound_buf[0], size/2);
#endif
}





//----------------------------------------------------------------------------------------------------------------------
#if 1
int wavoutLoadWav(const char *filename, wavout_wavinfo_t *wi, void *buf, unsigned long buflen)
{
	unsigned int filelen;
	int fd;
	unsigned long channels;
	unsigned long samplerate;
	unsigned long blocksize;
	unsigned long bitpersample;
	unsigned long datalength;
	unsigned long samplecount;
	unsigned long i;
	
	char *wavfile=buf;
	wi->wavdata=NULL;
	
	wavoutStopPlay0();

	fd=pgfOpen(filename,O_RDONLY);
	if (fd<0) return -1;
	
	filelen=pgfRead(fd,wavfile,buflen);
	pgfClose(fd);
	if (filelen>=buflen) {
		//too long
		return -1;
	}
	
	if (memcmp(wavfile,"RIFF",4)!=0) {
//		pgcPuts("format err");
		return -1;
	}
	
	if (memcmp(wavfile+8,"WAVEfmt \x10\x00\x00\x00\x01\x00",14)!=0) {
//		pgcPuts("format err");
		return -1;
	}
	
	channels=*(short *)(wavfile+0x16);
	samplerate=*(long *)(wavfile+0x18);
	blocksize=*(short *)(wavfile+0x20);
	bitpersample=*(short *)(wavfile+0x22);
	
	if (memcmp(wavfile+0x24,"data",4)!=0) {
//		pgcPuts("format err");
		return -1;
	}
	
	datalength=*(unsigned long *)(wavfile+0x28);
	
	if (datalength+0x2c>filelen) {
//		pgcPuts("format err");
		return -1;
	}
	
	if (channels!=2 && channels!=1) {
//		pgcPuts("format err, channel");
		return -1;
	}
	
//	if (samplerate!=44100 && samplerate!=22050 && samplerate!=11025) {
	if (samplerate>100000 || samplerate<2000) {
//		pgcPuts("format err, samplerate");
		return -1;
	}
	
	if (blocksize!=channels*2) {
//		pgcPuts("format err, blocksize");
		return -1;
	}
	
	if (bitpersample!=16) {
//		pgcPuts("format err, bitpersample");
		return -1;
	}
	
	if (channels==2) {
		samplecount=datalength/4;
	} else {
		samplecount=datalength/2;
	}
	if (samplecount<=0) {
//		pgcPuts("format err, samplecount");
		return -1;
	}

	wi->channels=channels;
	wi->samplerate=samplerate;
	wi->samplecount=samplecount;
	wi->datalength=datalength;
	wi->rateratio=(samplerate*0x4000)/11025;
	wi->playptr=0;
	wi->playptr_frac=0;
	wi->playloop=0;

	wi->wavdata=wavfile+0x2c;
	
	return 0;
}
#endif
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
