#include <windows.h>
#include <syscall.h>
#include <pg.h>
#include <sound.h>
#include "./audiow32.h"

//#define DEBUG
#undef DEBUG
#define MSG_WAIT 10
////////////////////////////////////////////////////////////////////////////////
WINBASEAPI DWORD WINAPI GetTickCount(VOID)
{
}
////////////////////////////////////////////////////////////////////////////////
#define TEST_CHANNEL  0

static int wav_terminate;
static int voice_channel = -1;
static int audio_channel = -1;

static char A_ERROR_TEXT[128] = "えら～だw";
AUDIOINFO ainfo;
A_AUDIO_VOICE_SEQ voice_seq[A_AUDIO_NUM_DEVS][VOICE_SEQ_LEN];
BYTE nWaveFormTmp[A_AUDIO_NUM_DEVS][BUFSIZE];
BYTE nWaveForm[A_AUDIO_NUM_DEVS][2][WAV_BUF_LEN];
int cur_buf_pos[A_AUDIO_NUM_DEVS];
int pre_buf_pos[A_AUDIO_NUM_DEVS];
int bef[A_AUDIO_NUM_DEVS];
extern SETTING setting;

////////////////////////////////////////////////////////////////////////////////

void init_voice_seq(int channel)
{
	if (channel >= A_AUDIO_NUM_DEVS) return;

	memset(voice_seq[channel], 0x00, sizeof(A_AUDIO_VOICE_SEQ) * VOICE_SEQ_LEN);
	cur_buf_pos[channel] = 0;
	pre_buf_pos[channel] = 0;
	bef[channel] = 0;
}

void set_voice_data(int channel)
{
#if 0
	if (cur_buf_pos[channel] < 0) return;
	if (channel >= A_AUDIO_NUM_DEVS) return;

	voice_seq[channel][cur_buf_pos[channel]].iClock			= nec_get_clock();
	voice_seq[channel][cur_buf_pos[channel]].nVolume		= voice_seq[channel][cur_buf_pos[channel]-1].nVolume;
	voice_seq[channel][cur_buf_pos[channel]].nPanning		= voice_seq[channel][cur_buf_pos[channel]-1].nPanning;
	voice_seq[channel][cur_buf_pos[channel]].dwFrequency	= voice_seq[channel][cur_buf_pos[channel]-1].dwFrequency;
	memcpy(voice_seq[channel][cur_buf_pos[channel]].nWaveForm,
           voice_seq[channel][cur_buf_pos[channel]-1].nWaveForm,
           32);
#endif
}

void set_data(int channel, BYTE *pnWaveForm)
{
	if (cur_buf_pos[channel] < 0) return;
	if (channel >= A_AUDIO_NUM_DEVS) return;

//	set_voice_data(channel);
//	renderer_update_sound();
//	cur_buf_pos[channel] = 0;
//	pre_buf_pos[channel] = 0;

	memcpy(nWaveForm[channel][bef[channel]], pnWaveForm, WAV_BUF_LEN);

//	bef[channel] = (bef[channel]?0:1); // 波形の変わり目で違う音が鳴っちゃうからなんだが。。。
}

void set_frequency(int channel, DWORD dwFrequency)
{
	if (cur_buf_pos[channel] < 0) return;
	if (channel >= A_AUDIO_NUM_DEVS) return;

//	set_voice_data(channel);
	voice_seq[channel][cur_buf_pos[channel]].iClock      = nec_get_clock();
	voice_seq[channel][cur_buf_pos[channel]].dwFrequency = dwFrequency;
	voice_seq[channel][cur_buf_pos[channel]].iFormBef = bef[channel];

	if (++cur_buf_pos[channel] == VOICE_SEQ_LEN)
		cur_buf_pos[channel] = 0;

//	renderer_update_sound();
}

void set_volume(int channel, UINT nVolume)
{
	if (cur_buf_pos[channel] < 0) return;

}

void set_panning(int channel, UINT nPanning)
{
	if (cur_buf_pos[channel] < 0) return;

}

//BYTE get_value(int channel, int k, int dd)
BYTE get_value(int channel, int bef, int k, int dd)
{
	BYTE ret;
	
	int pos = (int)(dd * WAV_BUF_LEN/360);
	ret = nWaveForm[channel][bef][pos];
	return ret;
}

void snd_render(char *sound_buf, int size)
{
	unsigned int i, j, k;
	static unsigned int  pos = 0;
	static unsigned int fEnd = 0;
	DWORD d, dd;
	int len;
	int cur_clock;
	int clock_diff;
	int channel;
	int limit;
	int output_channels;

//	if (fEnd)
		memset(sound_buf, 0x00, size);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ちゃんとしないとｗ
	for (channel = 0; channel < A_AUDIO_NUM_DEVS; channel++) {
		if (cur_buf_pos[channel] <= 0) goto END_LOOP;
		limit = (cur_buf_pos[channel]< VOICE_SEQ_LEN ? cur_buf_pos[channel] : VOICE_SEQ_LEN); // ちょっと違うけど。。。
		for (k = pre_buf_pos[channel]; k < limit ; k+=1) { // タイミングを合わせていない。。。
			len = SAMPLING_RATE / ((voice_seq[channel][k]).dwFrequency/32);	// 波長
			d = 360 / len; // 度／バイト数
			clock_diff = ((voice_seq[channel][k+1]).iClock - (voice_seq[channel][k]).iClock) * 1024; // 1024は適当
			if (clock_diff < 2048) clock_diff = 2048; // 2048は適当
			clock_diff /= (cur_buf_pos[channel] - pre_buf_pos[channel]);
			for (cur_clock = 0; cur_clock < clock_diff ; cur_clock++) { // 作成する長さ
				if (pos > size-1) {
					fEnd = 1;
					pos = 0;
					(voice_seq[channel][k]).iClock += cur_clock * (cur_buf_pos[channel] - pre_buf_pos[channel]);
					goto END_LOOP;
				}
				dd = d * ((cur_clock) % len); // 
				if (setting.sound && setting.sound_part[channel]) { // 出力設定されていなければカウンター消費以外何もしない（本当はどこで制御するのが良いのかねぇ？）
					sound_buf[pos]		+= ((short)(get_value(channel, (voice_seq[channel][k]).iFormBef, k, dd)))&0x7fff; // L
					sound_buf[pos + 1]	+= ((short)(get_value(channel, (voice_seq[channel][k]).iFormBef, k, dd)))&0x7fff; // R
				}
				pos += 2;
			} // end of cur_clock

		} // end of k
		fEnd = 0;

		END_LOOP :
		pre_buf_pos[channel] = k;

	} // end of channel
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ちゃんとしないとｗ

	return;
}

UINT AIAPI AInitialize(VOID)
{
#ifdef DEBUG
pgMessage("AInitialize()", MSG_INFO, MSG_WAIT);
#endif
	int i;
	int ret;
	int failed;
	char str[32];

	wav_terminate = 1;

	for (i = 0; i < A_AUDIO_NUM_DEVS; i++) {
		init_voice_seq(i);
//		AStopVoice((HAC)i);
		memset(nWaveFormTmp[i], 0x00, BUFSIZE);
		memset(nWaveForm[i][0], 0x00, WAV_BUF_LEN);
		memset(nWaveForm[i][1], 0x00, WAV_BUF_LEN);
		cur_buf_pos[i] = 0;
		pre_buf_pos[i] = 0;
		bef[i] = 0;
	}

	return AUDIO_ERROR_NONE;
}

/*
UINT AIAPI AGetVersion(VOID)
{
}
*/

UINT AIAPI AGetAudioNumDevs(VOID)
{
#ifdef DEBUG
pgMessage("AGetAudioNumDevs()", MSG_INFO, MSG_WAIT);
#endif
	return A_AUDIO_NUM_DEVS;
}

UINT AIAPI AGetAudioDevCaps(UINT nDeviceId, LPAUDIOCAPS lpCaps)
{
#ifdef DEBUG
pgMessage("AGetAudioDevCaps()", MSG_INFO, MSG_WAIT);
#endif
	lpCaps->wProductId = 0;
    strcpy(lpCaps->szProductName, "PSP DUMMY AUDIO DEV");
	lpCaps->dwFormats = AUDIO_FORMAT_16BITS | AUDIO_FORMAT_STEREO;

	return AUDIO_ERROR_NONE;
}

UINT AIAPI AGetErrorText(UINT nErrorCode, LPSTR lpText, UINT nSize)
{
#ifdef DEBUG
pgMessage("AGetErrorText()", MSG_INFO, MSG_WAIT);
#endif
	strcpy(lpText, A_ERROR_TEXT);

	return AUDIO_ERROR_NONE;
}

/*
UINT AIAPI APingAudio(LPUINT lpnDeviceId)
{
}
*/

UINT AIAPI AOpenAudio(LPAUDIOINFO lpInfo)
{
#ifdef DEBUG
pgMessage("AOpenAudio()", MSG_INFO, MSG_WAIT);
#endif
	int failed = 0;
	int ret;

	// set audio info
	ainfo.nDeviceId   = lpInfo->nDeviceId;
	ainfo.wFormat     = lpInfo->wFormat;
	ainfo.nSampleRate = lpInfo->nSampleRate;

	// create/start thread
	int i = 0;
	char str[32];
	strcpy(str,"crtwv0");
	str[6]='0'+i; str[7] = 0;

	wav_terminate = 0;
	voice_channel = 0;
	audio_channel = 0;
	memset(sound_buf, 0x00, SOUND_BUF_LEN);

//	if (setting.sound)
		wavoutStartPlay0(&wavinfo_bg);
	
	return AUDIO_ERROR_NONE;
}

UINT AIAPI ACloseAudio(VOID)
{
#ifdef DEBUG
pgMessage("ACloseAudio()", MSG_INFO, MSG_WAIT);
#endif
	// clear audio info
	// stop/delete thread
	wav_terminate = 1;

	int i;
	for (i = 0; i < A_AUDIO_NUM_DEVS; i++) {
		init_voice_seq(i);
		cur_buf_pos[i] = -1;
		pre_buf_pos[i] = -1;
	}
	memset(sound_buf, 0x00, SOUND_BUF_LEN);

	wavoutStopPlay0();

	return AUDIO_ERROR_NONE;
}

/*
UINT AIAPI AUpdateAudio(VOID)
{
}
*/

/*UINT AIAPI AUpdateAudioEx(UINT nFrames)
{
}
*/

/*
UINT AIAPI ASetAudioMixerValue(UINT nChannel, UINT nValue)
{
}
*/

UINT AIAPI AOpenVoices(UINT nVoices)
{
#ifdef DEBUG
pgMessage("AOpenVoices()", MSG_INFO, MSG_WAIT);
#endif
	return AUDIO_ERROR_NONE;
}

UINT AIAPI ACloseVoices(VOID)
{
#ifdef DEBUG
pgMessage("ACloseVoices()", MSG_INFO, MSG_WAIT);
#endif
	return AUDIO_ERROR_NONE;
}

/*
UINT AIAPI ASetAudioCallback(LPFNAUDIOWAVE lpfnAudioWave)
{
}
*/

/*
UINT AIAPI ASetAudioTimerProc(LPFNAUDIOTIMER lpfnAudioTimer)
{
}
*/

/*
UINT AIAPI ASetAudioTimerRate(UINT nTimerRate)
{
}
*/

/*
LONG AIAPI AGetAudioDataAvail(VOID)
{
}
*/


UINT AIAPI ACreateAudioVoice(LPHAC lphVoice)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "ACreateAudioVoice(%d)", voice_channel);
pgMessage(buf, MSG_INFO, MSG_WAIT);
#endif
	if (voice_channel >= A_AUDIO_NUM_DEVS) {
		(DWORD)(*lphVoice) = AUDIO_ERROR_HANDLE;
		return 1;
	}

	(DWORD)(*lphVoice) = voice_channel; //(DWORD)(&avoice[avoice_pos]);
	voice_channel++;

	return AUDIO_ERROR_NONE;
}

UINT AIAPI ACreateAudioData(LPAUDIOWAVE lpWave)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "ACreateAudioData(%d)", audio_channel);
pgMessage(buf, MSG_INFO, MSG_WAIT);
#endif
	if (lpWave == NULL) return 1;
	if (audio_channel >= A_AUDIO_NUM_DEVS) {
		lpWave->dwHandle = AUDIO_ERROR_HANDLE;
		lpWave->lpData   = NULL;
		return 1;
	}

	lpWave->dwHandle = audio_channel;
	lpWave->lpData   = nWaveFormTmp[audio_channel];
	audio_channel++;

	return AUDIO_ERROR_NONE;
}

UINT AIAPI AWriteAudioData(LPAUDIOWAVE lpWave, DWORD dwOffset, UINT nCount)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "AWriteAudioData(%d)", lpWave->dwHandle);
pgMessage(buf, MSG_INFO, MSG_WAIT);
#endif
	if (lpWave == NULL)                          return 1;
	if (lpWave->dwHandle == AUDIO_ERROR_HANDLE)  return 1;
	if (lpWave->lpData == NULL)                  return 1;

	set_data(lpWave->dwHandle, lpWave->lpData);

	return AUDIO_ERROR_NONE;
}

UINT AIAPI ADestroyAudioData(LPAUDIOWAVE lpWave)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "ADestroyAudioData(%d)", lpWave->dwHandle);
pgMessage(buf, MSG_INFO, MSG_WAIT);
#endif
	if (lpWave == NULL)                         return 1;
	if (lpWave->dwHandle == AUDIO_ERROR_HANDLE) return 1;

	wav_terminate = 1;

	lpWave->lpData      = NULL;
	lpWave->dwHandle    = AUDIO_ERROR_HANDLE;
	lpWave->dwLength    = 0;
	lpWave->dwLoopStart = 0;
	lpWave->dwLoopEnd   = 0;
	lpWave->nSampleRate = 0;
	lpWave->wFormat     = 0;
	lpWave = NULL;

	return AUDIO_ERROR_NONE;
}

UINT AIAPI ADestroyAudioVoice(HAC hVoice)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "ADestroyAudioVoice(%d)", (UINT)hVoice);
pgMessage(buf, MSG_INFO, MSG_WAIT);
#endif
	int channel = (UINT)(hVoice);
	if (channel == AUDIO_ERROR_HANDLE) return 1;

	init_voice_seq(channel);
	hVoice = 0;

	return AUDIO_ERROR_NONE;
}


UINT AIAPI APlayVoice(HAC hVoice, LPAUDIOWAVE lpWave)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "APlayVoice(%d, %d)", (UINT)hVoice, lpWave->dwHandle);
pgMessage(buf, MSG_INFO, MSG_WAIT);
#endif
	int channel = (UINT)hVoice;
	if (channel == AUDIO_ERROR_HANDLE)          return 1;
	if (lpWave->dwHandle == AUDIO_ERROR_HANDLE) return 1;

//	if (channel == TEST_CHANNEL) // TEST
//		wavoutStartPlay0(&wavinfo_bg);

	return AUDIO_ERROR_NONE;
}

/*
UINT AIAPI APrimeVoice(HAC hVoice, LPAUDIOWAVE lpWave)
{
}
*/

/*
UINT AIAPI AStartVoice(HAC hVoice)
{
}
*/

UINT AIAPI AStopVoice(HAC hVoice)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "AStopVoice(%d)", (UINT)hVoice);
pgMessage(buf, MSG_INFO, MSG_WAIT);
#endif
	int channel = (UINT)hVoice;
	if (channel == AUDIO_ERROR_HANDLE) return 1;

//	memset(nWaveFormTmp[channel], 0x00, BUFSIZE);
//	memset(nWaveForm[channel][0], 0x00, WAV_BUF_LEN);
//	memset(nWaveForm[channel][1], 0x00, WAV_BUF_LEN);
	cur_buf_pos[channel] = 0;
	pre_buf_pos[channel] = 0;
//	bef[channel] = 0;

	return AUDIO_ERROR_NONE;
}


UINT AIAPI ASetVoicePosition(HAC hVoice, LONG dwPosition)
{
}

UINT AIAPI ASetVoiceFrequency(HAC hVoice, LONG dwFrequency)
{
#ifdef DEBUG
if (hVoice == 0) {
char buf[128];
sprintf(buf, "ASetVoiceFrequency(%d, %d)", (UINT)hVoice, dwFrequency&0x0ffff);
pgMessage(buf, MSG_DEBUG, MSG_WAIT);
}
#endif
	int channel = (UINT)hVoice;
	if (channel == AUDIO_ERROR_HANDLE) return 1;

	set_frequency(channel, dwFrequency);

	return AUDIO_ERROR_NONE;
}

UINT AIAPI ASetVoiceVolume(HAC hVoice, UINT nVolume)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "ASetVoiceVolume(%d, %d)", (UINT)hVoice, nVolume);
pgMessage(buf, MSG_DEBUG, MSG_WAIT);
#endif
	int channel = (UINT)hVoice;
	if (channel == AUDIO_ERROR_HANDLE) return 1;

	set_volume(channel, nVolume);

	return AUDIO_ERROR_NONE;
}

UINT AIAPI ASetVoicePanning(HAC hVoice, UINT nPanning)
{
#ifdef DEBUG
char buf[128];
sprintf(buf, "ASetVoicePanning(%d, %d)", (UINT)hVoice, nPanning);
pgMessage(buf, MSG_DEBUG, MSG_WAIT);
#endif
	int channel = (UINT)hVoice;
	if (channel == AUDIO_ERROR_HANDLE) return 1;

	set_panning((UINT)hVoice, nPanning);

	return AUDIO_ERROR_NONE;
}


UINT AIAPI AGetVoicePosition(HAC hVoice, LPLONG lpdwPosition)
{
}

UINT AIAPI AGetVoiceFrequency(HAC hVoice, LPLONG lpdwFrequency)
{
}

UINT AIAPI AGetVoiceVolume(HAC hVoice, LPUINT lpnVolume)
{
}

UINT AIAPI AGetVoicePanning(HAC hVoice, LPUINT lpnPanning)
{
}

UINT AIAPI AGetVoiceStatus(HAC hVoice, LPBOOL lpnStatus)
{
}

/*
UINT AIAPI APlayModule(LPAUDIOMODULE lpModule)
{
}
*/

/*
UINT AIAPI AStopModule(VOID)
{
}
*/

/*
UINT AIAPI APauseModule(VOID)
{
}
*/

/*
UINT AIAPI AResumeModule(VOID)
{
}
*/

/*
UINT AIAPI ASetModuleVolume(UINT nVolume)
{
}
*/

/*
UINT AIAPI ASetModulePosition(UINT nOrder, UINT nRow)
{
}
*/

/*
UINT AIAPI AGetModuleVolume(LPUINT lpnVolume)
{
}
*/

/*
UINT AIAPI AGetModulePosition(LPUINT pnOrder, LPUINT lpnRow)
{
}
*/

/*
UINT AIAPI AGetModuleStatus(LPBOOL lpnStatus)
{
}
*/

/*
UINT AIAPI ASetModuleCallback(LPFNAUDIOCALLBACK lpfnAudioCallback)
{
}
*/

/*
UINT AIAPI ALoadModuleFile(LPSTR lpszFileName, 
                LPAUDIOMODULE* lplpModule, DWORD dwFileOffset)
{
}
*/

/*
UINT AIAPI AFreeModuleFile(LPAUDIOMODULE lpModule)
{
}
*/

/*
UINT AIAPI ALoadWaveFile(LPSTR lpszFileName, 
                LPAUDIOWAVE* lplpWave, DWORD dwFileOffset)
{
}
*/

/*
UINT AIAPI AFreeWaveFile(LPAUDIOWAVE lpWave)
{
}
*/

/*
UINT AIAPI AGetModuleTrack(UINT nTrack, LPAUDIOTRACK lpTrack)
{
}
*/

