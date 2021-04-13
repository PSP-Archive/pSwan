///////////////////////////////////////////////////////////////////////////////
// Wonderswan emulator
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <time.h>
#include <syscall.h>
#include <define.h>
#include <pg.h>
#include <sound.h>
#include "button.h"
#include "../oswan/source/nec/nec.h"
#include "../oswan/source/gpu.h"
#include "../oswan/source/io.h"
#include "../oswan/source/ws.h"

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
int			app_gameRunning=0;
int			app_rotated=0;
int			ws_colourScheme=COLOUR_SCHEME_DEFAULT;
int			ws_system=WS_SYSTEM_AUTODETECT;

char		ws_rom_path[MAX_PATH];
char		old_rom_path[MAX_PATH];
char		ws_app_path[MAX_PATH];
char		ws_rom_name[MAX_NAME];
char		ws_save_path[MAX_PATH];
ctrl_data_t	paddata;

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
// COMMENT 後でEnhanceする！！！
int bSleep = 0;
int exit_callback(void) 
{ 
#if 0
	scePowerSetClockFrequency(222, 222, 111);
#endif

	bSleep=1;

	// Cleanup the games resources etc (if required) 
	ws_saveSRAM(ws_save_path);

	// Exit game 
	sceKernelExitGame(); 

	return 0;
}
void power_callback(int unknown, int pwrflags) 
{
	int cbid;

#if 0
	scePowerSetClockFrequency(222, 222, 111);
#endif

	// Combine pwrflags and the above defined masks 
	if(pwrflags & POWER_CB_POWER){
		bSleep=1;
		ws_saveSRAM(ws_save_path);
	}
	
	// コールバック関数の再登録
	cbid = sceKernelCreateCallback("Power Callback", power_callback);
	scePowerRegisterCallback(0, cbid);
} 

// Thread to create the callbacks and then begin polling 
//int CallbackThread(void *arg) 
int CallbackThread(int args, void *argp)
{ 
	int cbid; 

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback); 
	SetExitCallback(cbid); 
	cbid = sceKernelCreateCallback("Power Callback", power_callback); 
	scePowerRegisterCallback(0, cbid); 

	KernelPollCallbacks(); 
} 

/* Sets up the callback thread and returns its thread id */ 
int SetupCallbacks(void) 
{ 
	int thid = 0; 

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0); 
	if(thid >= 0) 
	{ 
		sceKernelStartThread(thid, 0, 0); 
	} 

	return thid; 
} 


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
signed short backbuffer[224*144*16];
extern SETTING setting;
void renderer_update_pad()
{
	sceCtrlRead(&paddata, 1);
	ws_key_up		= 0;
	ws_key_down		= 0;
	ws_key_left		= 0;
	ws_key_right	= 0;
	ws_key_button_1	= 0;
	ws_key_button_2	= 0;
	ws_key_start	= 0;
	ws_key_menu		= 0;

	if (setting.key_config[BUTTON_X3]    && paddata.buttons == setting.key_config[BUTTON_X3])		ws_key_down		= 1; // DOWN
	if (setting.key_config[BUTTON_X1]    && paddata.buttons == setting.key_config[BUTTON_X1])		ws_key_up		= 1; // UP
	if (setting.key_config[BUTTON_X4]    && paddata.buttons == setting.key_config[BUTTON_X4])		ws_key_left		= 1; // LEFT
	if (setting.key_config[BUTTON_X2]    && paddata.buttons == setting.key_config[BUTTON_X2])		ws_key_right	= 1; // RIGHT
	if (setting.key_config[BUTTON_A]     && paddata.buttons == setting.key_config[BUTTON_A])		ws_key_button_1	= 1; // A
	if (setting.key_config[BUTTON_B]     && paddata.buttons == setting.key_config[BUTTON_B])		ws_key_button_2	= 1; // B
	if (setting.key_config[BUTTON_START] && paddata.buttons == setting.key_config[BUTTON_START])	ws_key_start	= 1; // START
	if (setting.key_config[BUTTON_MENU]  && paddata.buttons == setting.key_config[BUTTON_MENU])		ws_key_menu		= 1; // MENU

	if(setting.analog2dpad){
		if (paddata.analog[CTRL_ANALOG_Y] > UPPER_THRESHOLD) ws_key_down	= 1; // DOWN
		if (paddata.analog[CTRL_ANALOG_Y] < LOWER_THRESHOLD) ws_key_up		= 1; // UP
		if (paddata.analog[CTRL_ANALOG_X] < LOWER_THRESHOLD) ws_key_left	= 1; // LEFT
		if (paddata.analog[CTRL_ANALOG_X] > UPPER_THRESHOLD) ws_key_right	= 1; // RIGHT
	}

	if (setting.key_config[BUTTON_QUICKSAVE] && paddata.buttons == setting.key_config[BUTTON_QUICKSAVE]) {
		wavoutStopPlay0();
		int ret = ws_saveState();
		if (!ret) pgMessage("Status save error.", MSG_ERROR, 300);
		if (setting.sound) wavoutStartPlay0(&wavinfo_bg);
	}
	if (setting.key_config[BUTTON_QUICKLOAD] && paddata.buttons == setting.key_config[BUTTON_QUICKLOAD]) {
		wavoutStopPlay0();
		int ret = ws_loadState();
		if (!ret) pgMessage("Status load error.", MSG_ERROR, 300);
		if (setting.sound) wavoutStartPlay0(&wavinfo_bg);
	}
}

void render_screen()
{
	if (app_rotated) {
		pgBitBltN1v((SCREEN_WIDTH - 144) / 2, (SCREEN_HEIGHT - 224) / 2, (unsigned long *)backbuffer);
	} else {
		pgBitBltN1h((SCREEN_WIDTH - 224) / 2, (SCREEN_HEIGHT - 144) / 2, (unsigned long *)backbuffer);
	}
}

extern SETTING setting;
void ws_emulate_standard(void)
{
	unsigned long startTime, endTime, totalFrames;
	unsigned int nNormalLast=0;
	int nNormalFrac=0;
	int nTime=0,nCount=0; int i=0;
	int surfacePitch;
	
	char buf[128];
	memset(backbuffer,0x00,224*144*sizeof(signed short));
	totalFrames=0;
	nNormalLast=0;
	nNormalFrac=0;
	
//	ws_loadSRAM(ws_save_path); // -> menu
	
	const unsigned int sync_time=16666;
	unsigned long prev_time, cur_time, diff_time = 1000000;
	unsigned long framecount = 0;
	
	while (1) {
		pgFillvram(0);
		
		nTime = sceKernelLibcClock()/1000 - nNormalLast;
		nCount=(nTime*600 - nNormalFrac) /10000;
		nNormalFrac+=nCount*10000;
		nNormalLast+=nNormalFrac/600;
		nNormalFrac%=600;
		
		renderer_update_pad();
		
		if (nCount>5)
			nCount=5; // Original 10
		for (i=0;i<nCount-1;i++)
			while (!ws_executeLine(backbuffer,0));
		while (!ws_executeLine(backbuffer,1));
		totalFrames++;
		
		if (ws_key_menu == 1) {
			// 
			wavoutStopPlay0();
			main_menu(0);
//			if(setting.sound)
				wavoutStartPlay0(&wavinfo_bg);
			
			// 調整
		}
		
		pgScreenFlipV(); // やべ、pgScreenFlipV()にしないとHOMEに返らくなっちゃった。。。

		if(bSleep){
			wavoutStopPlay0();
			pgWaitVn(220);
			
//			if(setting.sound)
				wavoutStartPlay0(&wavinfo_bg);
			bSleep=0;
		}

		
	}
}

extern char LastPath[MAX_PATH];
int xmain(int argc, char *argv)
{

	// set path info
	int n;
	char *p;

	n = argc;
	if (n > sizeof(ws_app_path) - 1) n = sizeof(ws_app_path) - 1;
	memcpy(ws_app_path, argv, n);
	ws_app_path[sizeof(ws_app_path) - 1] = 0;
	strcpy(ws_app_path, argv);
	p = strrchr(ws_app_path, '/');
	*++p = 0;
	strcpy(LastPath, ws_app_path);

	// Initialize screen
	pgScreenInit();

	// Initialize controler
	pgControlInit();
	
	// Set Callback
	int tid;
	tid = SetupCallbacks();

	// Initialize wave
	wavoutInit();

	pgScreenClear();

	load_config(); // call before getFilePath()

	// Load ROM
//	while (!getFilePath(ws_rom_path))
//		;
	main_menu(1);

/*
	p = strrchr(ws_rom_path, '/');
	strcpy(ws_rom_name, ++p);
	strcpy(ws_save_path, ws_rom_path);
	p = strrchr(ws_save_path, '/');
	*++p = 0;
	strcat(ws_save_path, ws_rom_name);
	p = strrchr(ws_save_path, '.');
	*++p = 0;
	strcat(ws_save_path, "sav");
*/

//	pgScreenClear();


#if 0
scePowerSetClockFrequency(333,333,166);
char buf[128];
int freq = scePowerGetCpuClockFrequency();
sprintf(buf, "scePowerGetCpuClockFrequency() = %d", freq);
pgMessage(buf, MSG_DEBUG, 100);
#endif

/* 場所変更 -> menu
	if (ws_init(ws_rom_path)) {
		app_rotated=ws_rotated();
		app_gameRunning=1;
		ws_set_system(ws_system);
		ws_set_colour_scheme(ws_colourScheme);
		ws_reset();
		
		ws_emulate_standard();
	}
*/
	ws_emulate_standard();

	ws_done();

	return(0);
}

