//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_H__
#define __AUDIO_H__

void ws_audio_init(void);
void ws_audio_reset(void);
void ws_audio_port_write(DWORD port,BYTE value);
BYTE ws_audio_port_read(BYTE port);
void ws_audio_done(void);

unsigned int ws_audio_mrand(unsigned int Degree);
int ws_audio_seal_init(void);
void ws_audio_seal_done(void);
int ws_audio_play_channel(int Channel);
int ws_audio_stop_channel(int Channel);
void ws_audio_clear_channel(int Channel);
void ws_audio_set_channel_frequency(int Channel,int Period);
void ws_audio_set_channel_volume(int Channel,int Vol);
void ws_audio_set_channel_pan(int Channel,int Left,int Right);
void ws_audio_set_channel_pdata(int Channel,int Index);
void ws_audio_set_channels_pbuf(int Addr,int Data);
void ws_audio_rst_channel(int Channel);
int ws_audio_int(void);
void ws_audio_set_pcm(int Data);
void ws_audio_flash_pcm(void);
void ws_audio_write_byte(DWORD offset, BYTE value);
void ws_audio_process(void);
#if 0
void ws_audio_readState(int fp);
void ws_audio_writeState(int fp);
#else
int  ws_audio_readState(int fd);
int  ws_audio_writeState(int fd);
#endif

#endif
