#include <windows.h>
#include <msio.h>
#include <syscall.h>
#include <types.h>
#include <define.h>
#include "zlibInterface.h"
#include "colbl.c"
#include "pg.h"
#include "menu.h"
#include "button.h"


////////////////////////////////////////////////////////////////////////////////
extern SETTING setting;
extern u32 new_pad, old_pad;
extern ctrl_data_t paddata;
extern char ws_rom_path[MAX_PATH];
extern char ws_app_path[MAX_PATH];

////////////////////////////////////////////////////////////////////////////////
int bBitmap;
unsigned short bgBitmap[480*272];
unsigned char org_gbtype;

////////////////////////////////////////////////////////////////////////////////
static void load_menu_bg(void);
static unsigned short rgbTransp(unsigned short fgRGB, unsigned short bgRGB, int alpha);
static void menu_colorconfig(void);
static void set_default_key(void);
static void menu_keyconfig(void);
static void menu_soundconfig(void);
static int menu_screensize(int n);
static int menu_frameskip(int sel);
static bool get_state_time(int slot, char *out);
#if 0
static int menu_stateslot(statemethod method);
#endif

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Unzip (not available)
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
// Unzip 対応 by ruka
// コールバック受け渡し用
typedef struct {
	byte *p_rom_image;			// pointer to rom image
	long rom_size;				// rom size
	char szFileName[MAX_PATH];	// extracted file name
}ROM_INFO, *LPROM_INFO;

// せっかくなのでプログレスでも出してみます
void draw_load_rom_progress(unsigned long ulExtractSize, unsigned long ulCurrentPosition)
{
#if 0
	int nPer = 100 * ulExtractSize / ulCurrentPosition;
	static int nOldPer = 0;
	if (nOldPer == nPer & 0xFFFFFFFE) {
		return ;
	}
	nOldPer = nPer;
	if(bBitmap)
		pgBitBlt(0,0,480,272,1,bgBitmap);
	else
		pgFillvram(setting.color[0]);
	// プログレス
	pgDrawFrame(89,121,391,141,setting.color[1]);
	pgFillBox(90,123, 90+nPer*3, 139,setting.color[1]);
	// ％
	char szPer[16];
	_itoa(nPer, szPer);
	strcat(szPer, "%");
	pgPrint(28,16,setting.color[3],szPer);
	// pgScreenFlipV()を使うとpgWaitVが呼ばれてしまうのでこちらで。
	// プログレスだからちらついても良いよね～
	pgScreenFlip();
#endif
}

// Unzip コールバック
int funcUnzipCallback(int nCallbackId, unsigned long ulExtractSize, unsigned long ulCurrentPosition,
                      const void *pData, unsigned long ulDataSize, unsigned long ulUserData)
{
#if 0
    const char *pszFileName;
    int nExtId;
    const unsigned char *pbData;
    LPROM_INFO pRomInfo = (LPROM_INFO)ulUserData;

    switch(nCallbackId) {
    case UZCB_FIND_FILE:
		pszFileName = (const char *)pData;
		nExtId = getExtId(pszFileName);
		// 拡張子がGBかGBCなら展開
		if (nExtId == EXT_GB || nExtId == EXT_GBC) {
			// 展開する名前、rom sizeを覚えておく
			strcpy(pRomInfo->szFileName, pszFileName);
			pRomInfo->rom_size = ulExtractSize;
			return UZCBR_OK;
		}
        break;
    case UZCB_EXTRACT_PROGRESS:
		pbData = (const unsigned char *)pData;
		// 展開されたデータを格納しよう
		_memcpy(pRomInfo->p_rom_image + ulCurrentPosition, pbData, ulDataSize);
		draw_load_rom_progress(ulCurrentPosition + ulDataSize, ulExtractSize);
		return UZCBR_OK;
        break;
    default: // unknown...
		pgFillvram(RGB(255,0,0));
		mh_print(0,0,"Unzip fatal error.",0xFFFF);
		pgScreenFlipV();
        break;
    }
#endif
    return UZCBR_PASS;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Menu characters color & background BMP configuration
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
// by kwn
static void load_menu_bg()
{
	byte *menu_bg;
	unsigned char *vptr;
	static byte menu_bg_buf[480*272*3+0x36];
	char ws_bg_path[MAX_PATH];
	char *p;
 	unsigned short x,y,yy,r,g,b,data;

	strcpy(ws_bg_path, ws_app_path);
	p = strrchr(ws_bg_path, '/')+1;
	strcpy(p, PSWAN_MENU_BMP);

	int fd;
	if ((fd = ms_fopen(ws_bg_path, "r")) >= 0) {
		if (ms_fread(menu_bg_buf, 480*272*3+0x36, 1, fd) < 1) {
			pgMessage("menu background bmp load error.", MSG_ERROR, 100);
			ms_fclose(fd);
			return;
		}
		ms_fclose(fd);

		menu_bg = menu_bg_buf + 0x36;
		vptr=(unsigned char*)bgBitmap;
		for(y=0; y<272; y++){
			for(x=0; x<480; x++){
				yy = 271 - y;
				r = *(menu_bg + (yy*480 + x)*3 + 2);
				g = *(menu_bg + (yy*480 + x)*3 + 1);
				b = *(menu_bg + (yy*480 + x)*3);
				data = (((b & 0xf8) << 7) | ((g & 0xf8) << 2) | (r >> 3));
				*(unsigned short *)vptr=data;
				vptr+=2;
			}
		}
		bBitmap = 1;
	} else {
		bBitmap = 0;
	}
}

// 半透明処理
static unsigned short rgbTransp(unsigned short fgRGB, unsigned short bgRGB, int alpha) {

    unsigned short fgR, fgG, fgB;
    unsigned short bgR, bgG, bgB;
	unsigned short R, G, B;
 	unsigned short rgb;

    fgB = (fgRGB >> 10) & 0x1F;
    fgG = (fgRGB >> 5) & 0x1F;
    fgR = fgRGB & 0x1F;

    bgB = (bgRGB >> 10) & 0x1F;
    bgG = (bgRGB >> 5) & 0x1F;
    bgR = bgRGB & 0x1F;

	R = coltbl[fgR][bgR][alpha/10];
	G = coltbl[fgG][bgG][alpha/10];
	B = coltbl[fgB][bgB][alpha/10];

	rgb = (((B & 0x1F)<<10)+((G & 0x1F)<<5)+((R & 0x1F)<<0)+0x8000);
    return rgb;
}

static void bgbright_change()
{
	unsigned short *vptr,rgb;
 	int i;

//	load_menu_bg();
	vptr=bgBitmap;
	for(i=0; i<272*480; i++){
			rgb = *vptr;
			*vptr = rgbTransp(rgb, 0x0000, setting.bgbright);
			vptr++;
	}
}

static void menu_colorconfig(void)
{
	enum
	{
		COLOR0_R=0,
		COLOR0_G,
		COLOR0_B,
		COLOR1_R,
		COLOR1_G,
		COLOR1_B,
		COLOR2_R,
		COLOR2_G,
		COLOR2_B,
		COLOR3_R,
		COLOR3_G,
		COLOR3_B,
		BG_BRIGHT,
		INIT,
		EXIT,
	};
	char tmp[4], msg[256];
	int color[4][3];
	int sel=0, x, y, i;
	int nCursor = 0;

	memset(color, 0, sizeof(int)*4*3);
	for(i=0; i<4; i++){
		color[i][2] = setting.color[i]>>10 & 0x1F;
		color[i][1] = setting.color[i]>>5 & 0x1F;
		color[i][0] = setting.color[i] & 0x1F;
	}

	for(;;){
		readpad();
		if(new_pad & CTRL_CIRCLE){
			if(sel==EXIT){
				break;
			}else if(sel==INIT){
				color[0][2] = DEF_COLOR0>>10 & 0x1F;
				color[0][1] = DEF_COLOR0>>5 & 0x1F;
				color[0][0] = DEF_COLOR0 & 0x1F;
				color[1][2] = DEF_COLOR1>>10 & 0x1F;
				color[1][1] = DEF_COLOR1>>5 & 0x1F;
				color[1][0] = DEF_COLOR1 & 0x1F;
				color[2][2] = DEF_COLOR2>>10 & 0x1F;
				color[2][1] = DEF_COLOR2>>5 & 0x1F;
				color[2][0] = DEF_COLOR2 & 0x1F;
				color[3][2] = DEF_COLOR3>>10 & 0x1F;
				color[3][1] = DEF_COLOR3>>5 & 0x1F;
				color[3][0] = DEF_COLOR3 & 0x1F;
				setting.bgbright = 100;
				if(bBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else if(sel == BG_BRIGHT) {
				//輝度変更
				setting.bgbright += 10;
				if(setting.bgbright > 100) setting.bgbright=0;
				if(bBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else{
				if(color[sel/3][sel%3]<31)
					color[sel/3][sel%3]++;
			}
		}else if(new_pad & CTRL_CROSS){
			if(sel == BG_BRIGHT) {
				//輝度変更
				setting.bgbright -= 10;
				if(setting.bgbright < 0) setting.bgbright=100;
				if(bBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else if(sel>=COLOR0_R && sel<=COLOR3_B){
				if(color[sel/3][sel%3]>0)
					color[sel/3][sel%3]--;
			}
		}else if(new_pad & CTRL_UP){
			if(sel!=0)	sel--;
			else		sel=EXIT;
			if (sel == BG_BRIGHT && !bBitmap) sel--;
		}else if(new_pad & CTRL_DOWN){
			if(sel!=EXIT)	sel++;
			else			sel=0;
			if (sel == BG_BRIGHT && !bBitmap) sel++;
		}else if(new_pad & CTRL_RIGHT){
			if(sel<COLOR1_R) 		sel=COLOR1_R;
			else if(sel<COLOR2_R)	sel=COLOR2_R;
			else if(sel<COLOR3_R)	sel=COLOR3_R;
			else if(sel<BG_BRIGHT) {
				if (bBitmap)		sel=BG_BRIGHT;
				else				sel=INIT;
			}
			else if(sel<EXIT)		sel=INIT;
		}else if(new_pad & CTRL_LEFT){
			if(sel>BG_BRIGHT) {
				if (bBitmap)		sel=BG_BRIGHT;
				else				sel=COLOR3_R;
			}
			else if(sel>COLOR3_B)	sel=COLOR3_R;
			else if(sel>COLOR2_B)	sel=COLOR2_R;
			else if(sel>COLOR1_B)	sel=COLOR1_R;
			else					sel=COLOR0_R;
		}

		for(i=0; i<4; i++)
			setting.color[i]=color[i][2]<<10|color[i][1]<<5|color[i][0]|0x8000;

		x = 2;
		y = 5;

		if(sel>=COLOR0_R && sel<=BG_BRIGHT)
			sprintf(msg, "%c:Add %c:Sub", 1, 2);
//			strcpy(msg, "○：Add  ×：Sub");
		else
			sprintf(msg, "%c:OK", 1);
//			strcpy(msg, "○：OK");

		menu_frame(0, msg);

		pgPrint(x,y++,setting.color[3],"  COLOR0 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR0 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR0 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR1 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR1 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR1 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR2 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR2 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR2 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR3 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR3 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR3 B:");
		y++;

		int char_color;
		if (bBitmap) char_color = setting.color[3];
		else         char_color = setting.color[2];

		if(setting.bgbright / 100 == 1)
			pgPrint(x, y++, char_color, "  BG BRIGHT:100%");
		else
			pgPrint(x, y++, char_color, "  BG BRIGHT:  0%");

		if(setting.bgbright % 100 != 0)			// 10%～90%
			pgPutChar((x+13)*8, (y-1)*8, char_color, 0, '0'+setting.bgbright/10, 1, 0, 1);
		y++;

		pgPrint(x,y++,setting.color[3],"  Set default");
		pgPrint(x,y++,setting.color[3],"  Return to Main Menu");

		x=14; y=5;
		for(i=0; i<12; i++){
			if(i!=0 && i%3==0) y++;
			sprintf(tmp, "%d", color[i/3][i%3]);
			pgPrint(x,y++,setting.color[3],tmp);
		}

		x = 2;
		y = sel + 5;
		if(sel>=COLOR1_R) y++;
		if(sel>=COLOR2_R) y++;
		if(sel>=COLOR3_R) y++;
		if(sel>=BG_BRIGHT) y++;
		if(sel>=INIT) y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

#define KEY_MAX 13
#define CTRL_UNDEFINED	0x0000
int ctrl_key[KEY_MAX] = {
	CTRL_UNDEFINED	,
	CTRL_SQUARE		,
	CTRL_TRIANGLE	,
	CTRL_CIRCLE		,
	CTRL_CROSS		,
	CTRL_UP			,
	CTRL_DOWN		,
	CTRL_LEFT		,
	CTRL_RIGHT		,
	CTRL_START		,
	CTRL_SELECT		,
	CTRL_LTRIGGER	,
	CTRL_RTRIGGER	,
};

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Key configuration
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
static void set_default_key(void)
{
	int i;

	strcpy(setting.vercnf, PSWAN_CFG_VERSION);
	setting.screensize = SCR_X1;
	setting.vsync = 0;
	setting.sound = 1;
	setting.state_slot = 0;
	setting.sound_part[0]  = 0;
	setting.sound_part[1]  = 0;
	setting.sound_part[2]  = 0;
	setting.sound_part[3]  = 0;
	setting.sound_part[4]  = 0;
	setting.sound_part[5]  = 0;
	setting.sound_part[6]  = 0;
	setting.sampling_rate  = 2;
	setting.sound_buffer   = 0;
	setting.key_config[BUTTON_A]			= CTRL_CIRCLE;
	setting.key_config[BUTTON_B]			= CTRL_CROSS;
	setting.key_config[BUTTON_X1]			= CTRL_UP;	
	setting.key_config[BUTTON_X2]			= CTRL_RIGHT;
	setting.key_config[BUTTON_X3]			= CTRL_DOWN;	
	setting.key_config[BUTTON_X4]			= CTRL_LEFT;	
	setting.key_config[BUTTON_Y1]			= 0;
	setting.key_config[BUTTON_Y2]			= 0;
	setting.key_config[BUTTON_Y3]			= 0;
	setting.key_config[BUTTON_Y4]			= 0;
	setting.key_config[BUTTON_START]		= CTRL_START;
	setting.key_config[BUTTON_RAPIDA]		= 0;
	setting.key_config[BUTTON_RAPIDB]		= 0;
	setting.key_config[BUTTON_QUICKSAVE]	= 0;
	setting.key_config[BUTTON_QUICKLOAD]	= 0;
	setting.key_config[BUTTON_MENU]			= CTRL_RTRIGGER;

	for (i = BUTTON_MENU+1; i < MAX_BUTTON_CONFIG; i++)
		setting.key_config[i] = 0;

	setting.analog2dpad    = 0;

	setting.color[0]       = DEF_COLOR0;
	setting.color[1]       = DEF_COLOR1;
	setting.color[2]       = DEF_COLOR2;
	setting.color[3]       = DEF_COLOR3;
	setting.bgbright       = 100;

	setting.wait = 1;
	setting.frameskip = 0;
}

static void menu_keyconfig(void)
{
	enum
	{
		CONFIG_A = 0,
		CONFIG_B,
		CONFIG_X1,
		CONFIG_X2,
		CONFIG_X3,
		CONFIG_X4,
		CONFIG_Y1,
		CONFIG_Y2,
		CONFIG_Y3,
		CONFIG_Y4,
		CONFIG_START,
		CONFIG_RAPIDA,
		CONFIG_RAPIDB,
		CONFIG_QUICKSAVE,
		CONFIG_QUICKLOAD,
		CONFIG_MENU,
//		CONFIG_STATE_SLOT,
//		CONFIG_WAIT,
//		CONFIG_VSYNC,
//		CONFIG_SOUND,
//		CONFIG_SCREENSIZE,
		CONFIG_ANALOG2DPAD,
		CONFIG_DEFAULT,
		CONFIG_EXIT,
	};
	char msg[256];
	int nCursor = 0;
	int sel=0, x, y, i, j, bPad;
	int nKey[CONFIG_ANALOG2DPAD+1];
	memset(nKey, 0x00, sizeof(nKey));

	for (i = 0; i < CONFIG_ANALOG2DPAD+1; i++) {
		for (j = 0; j < KEY_MAX; j++) {
			if (setting.key_config[i] == ctrl_key[j]) nKey[i] = j;
		}
	}

	for(;;){
		readpad();
		if (now_pad == CTRL_LEFT) {
			if (bPad == 0 || bPad >= 25) {
				if(sel!=CONFIG_EXIT && sel!=CONFIG_MENU && sel!=CONFIG_ANALOG2DPAD) {
					if (--nKey[sel] < 0) nKey[sel] = KEY_MAX - 1;
					setting.key_config[sel] = ctrl_key[nKey[sel]];
				}
				if (bPad == 0) bPad++;
				else           bPad = 20;
			} else bPad++;
		} else if (now_pad == CTRL_RIGHT) {
			if (bPad == 0 || bPad >= 25) {
				if(sel!=CONFIG_EXIT && sel!=CONFIG_MENU && sel!=CONFIG_ANALOG2DPAD && sel != CONFIG_DEFAULT) {
					if (++nKey[sel] > KEY_MAX - 1) nKey[sel] = 0;
					setting.key_config[sel] = ctrl_key[nKey[sel]];
				}
				if (bPad == 0) bPad++;
				else           bPad = 20;
			} else bPad++;
		} else if (now_pad == CTRL_UP) {
			if(bPad==0 || bPad >= 25){
				if(sel!=0)	sel--;
				else		sel=CONFIG_EXIT;

				if (bPad == 0) bPad++;
				else           bPad = 20;
			}else
				bPad++;
		} else if(now_pad==CTRL_DOWN) {
			if(bPad==0 || bPad >= 25){
				if(sel!=CONFIG_EXIT)sel++;
				else				sel=0;
	
				if (bPad == 0) bPad++;
				else           bPad = 20;
			}else
				bPad++;
		} else if(new_pad != 0 ){
			if(sel==CONFIG_EXIT && new_pad&CTRL_CIRCLE)
				break;
			else if (sel == CONFIG_ANALOG2DPAD && new_pad&CTRL_CIRCLE)
				setting.analog2dpad = !setting.analog2dpad;
			else if (sel == CONFIG_DEFAULT && new_pad&CTRL_CIRCLE)
				set_default_key();
			else
				setting.key_config[sel] = now_pad;
		} else {
			bPad=0;
		}
		
		if(sel>=CONFIG_ANALOG2DPAD)
			sprintf(msg, "%c:OK", 1);
//			strcpy(msg,"○：OK");
		else
			sprintf(msg, "%c%c:Next Key", 8, 6);
//			strcpy(msg,"←→：Clear");
		
		menu_frame(0, msg);
		
		x=2; y=5;
		pgPrint(x,y++,setting.color[3],"  A  BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  B  BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  X1 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  X2 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  X3 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  X4 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  Y1 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  Y2 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  Y3 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  Y4 BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  START BUTTON    :");

		pgPrint(x,y++,setting.color[3],"  A  BUTTON(RAPID):");
		pgPrint(x,y++,setting.color[3],"  B  BUTTON(RAPID):");
		pgPrint(x,y++,setting.color[3],"  QUICK SAVE      :");
		pgPrint(x,y++,setting.color[3],"  QUICK LOAD      :");

		pgPrint(x,y++,setting.color[3],"  MENU BUTTON     :");

//		pgPrint(x,y++,setting.color[3],"  QUICK SLOT      :");
//		pgPrint(x,y++,setting.color[3],"  WAIT ON/OFF     :");
//		pgPrint(x,y++,setting.color[3],"  VSYNC ON/OFF    :");
//		pgPrint(x,y++,setting.color[3],"  SOUND ON/OFF    :");
//		pgPrint(x,y++,setting.color[3],"  SCREEN SIZE     :");
		y++;

		if(setting.analog2dpad)
			pgPrint(x,y++,setting.color[3],"  AnalogPad to D-Pad: ON");
		else
			pgPrint(x,y++,setting.color[3],"  AnalogPad to D-Pad: OFF");
		y++;

		pgPrint(x, y++, setting.color[3], "  Set default");
		pgPrint(x, y++, setting.color[3], "  Return to Main Menu");
		
		for (i=0; i<CONFIG_ANALOG2DPAD; i++){
			y = i + 5;
			int j = 0;
			msg[0]=0;
			if(setting.key_config[i] == 0){
				strcpy(msg,"UNDEFINED");
			}else{
				if (setting.key_config[i] & CTRL_LTRIGGER){
					msg[j++]='L'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_RTRIGGER){
					msg[j++]='R'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_CIRCLE){
					msg[j++]=1; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_CROSS){
					msg[j++]=2; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_SQUARE){
					msg[j++]=3; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_TRIANGLE){
					msg[j++]=4; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_START){
					strcat(msg,"START+"); j+=6;
				}
				if (setting.key_config[i] & CTRL_SELECT){
					strcat(msg,"SELECT+"); j+=7;
				}
				if (setting.key_config[i] & CTRL_UP){
					msg[j++]=5; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_RIGHT){
					msg[j++]=6; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_DOWN){
					msg[j++]=7; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_LEFT){
					msg[j++]=8; msg[j++]='+'; msg[j]=0;
				}
				msg[strlen(msg)-1]=0;
			}
			pgPrint(21,y,setting.color[3],msg);
		}
		
		// カーソル表示
		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_ANALOG2DPAD) y++;
		if(sel >= CONFIG_DEFAULT)     y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1) %10;
		
		pgScreenFlipV();
	}
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Sound configuration
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#define SAMPLINGRATE_COUNT 3
#define CHANNEL1 0
#define CHANNEL2 1
#define CHANNEL3 2
#define CHANNEL4 3
#define NOISE    4
#define PCM      5
#define DMA      6
static void menu_soundconfig(void)
{
	enum
	{
		CONFIG_SOUND = 0,
		CONFIG_CHANNEL1,
		CONFIG_CHANNEL2,
		CONFIG_CHANNEL3,
		CONFIG_CHANNEL4,
		CONFIG_NOISE,
		CONFIG_PCM,
		CONFIG_DMA,
		CONFIG_SAMPLINGRATE,
		CONFIG_EXIT
	};
	char sampling_rate_str[SAMPLINGRATE_COUNT][32] = {
		"11025",
		"22050",
		"44100"
	};

	char msg[256], szTemp[256];
	int sel=0, x, y, i, bPad;
	int nCursor = 0;
	int color;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad==CTRL_UP){
			if (!setting.sound && sel == CONFIG_EXIT) sel = CONFIG_SOUND;
			else {
#if 1
				if (sel == CONFIG_EXIT) sel = CONFIG_CHANNEL4;
				else {
#endif
				if(sel!=0)	sel--;
				else		sel=CONFIG_EXIT;
#if 1
				}
#endif
			}
		}else if(new_pad==CTRL_DOWN){
			if (!setting.sound && sel == CONFIG_SOUND) sel = CONFIG_EXIT;
			else {
#if 1
				if (sel == CONFIG_CHANNEL4) sel = CONFIG_EXIT;
				else {
#endif
				if(sel!=CONFIG_EXIT)sel++;
				else				sel=0;
#if 1
			}
#endif
			}
		}else if(new_pad & CTRL_CROSS){
			break;
		}else if(new_pad & CTRL_CIRCLE){
			if(sel==CONFIG_SOUND)
				setting.sound = !setting.sound;
			else if(sel==CONFIG_CHANNEL1)
				setting.sound_part[CHANNEL1] = !setting.sound_part[CHANNEL1];
			else if(sel==CONFIG_CHANNEL2)
				setting.sound_part[CHANNEL2] = !setting.sound_part[CHANNEL2];
			else if(sel==CONFIG_CHANNEL3)
				setting.sound_part[CHANNEL3] = !setting.sound_part[CHANNEL3];
			else if(sel==CONFIG_CHANNEL4)
				setting.sound_part[CHANNEL4] = !setting.sound_part[CHANNEL4];
			else if(sel==CONFIG_NOISE)
				setting.sound_part[NOISE]    = !setting.sound_part[NOISE];
			else if(sel==CONFIG_PCM)
				setting.sound_part[PCM]      = !setting.sound_part[PCM];
			else if(sel==CONFIG_DMA)
				setting.sound_part[DMA]      = !setting.sound_part[DMA];
			else if(sel==CONFIG_SAMPLINGRATE)
#if 0
				setting.sampling_rate = ++setting.sampling_rate % SAMPLINGRATE_COUNT;
#else
				;
#endif
			else if (sel == CONFIG_EXIT) break;
		}else if(new_pad & CTRL_LEFT){
#if 0
			if(sel<=CONFIG_SOUND)             sel=CONFIG_SAMPLINGRATE;
#else
			if(sel<=CONFIG_SOUND)             sel=CONFIG_CHANNEL4;
#endif
			else if(sel<=CONFIG_CHANNEL1)     sel=CONFIG_SOUND;
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_CHANNEL1;
		}else if(new_pad & CTRL_RIGHT){
			if(sel<=CONFIG_SOUND)             sel=CONFIG_CHANNEL1;
#if 0
			else if(sel<=CONFIG_DMA)          sel=CONFIG_SAMPLINGRATE;
#else
			else if(sel<=CONFIG_CHANNEL3)     sel=CONFIG_CHANNEL4;
#endif
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_SOUND;
		}

		sprintf(msg, "%c:OK %c:Return to Main Menu", 1, 2);
		menu_frame(0, msg);

		x=2; y=5;
		if (setting.sound) color = setting.color[3];
		else               color = setting.color[2];

		sprintf(msg, "  SOUND  : %s", (setting.sound ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, setting.color[3], msg);
		y++;
		sprintf(msg, "    CHANNEL 1     : %s", (setting.sound_part[CHANNEL1] ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, color, msg);
		sprintf(msg, "    CHANNEL 2     : %s", (setting.sound_part[CHANNEL2] ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, color, msg);
		sprintf(msg, "    CHANNEL 3     : %s", (setting.sound_part[CHANNEL3] ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, color, msg);
		sprintf(msg, "    CHANNEL 4     : %s", (setting.sound_part[CHANNEL4] ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, color, msg);
#if 1
color = setting.color[2];
#endif
		sprintf(msg, "    NOISE         : %s", (setting.sound_part[NOISE]    ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, color, msg);
		sprintf(msg, "    PCM           : %s", (setting.sound_part[PCM]      ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, color, msg);
		sprintf(msg, "    DMA           : %s", (setting.sound_part[DMA]      ? "ENABLE" : "DISABLE"));
		pgPrint(x, y++, color, msg);
		sprintf(msg, "    SAMPLING RATE : %s", sampling_rate_str[setting.sampling_rate]);
		pgPrint(x,y++,color,msg);
		y++;
		pgPrint(x, y++, setting.color[3], "  Return to Main Menu");

		x = 2;
		y = sel + 5;
		if (sel >= CONFIG_CHANNEL1) y++;
		if (sel >= CONFIG_EXIT)    y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
#if 0
	PSPEMU_ApplySoundConfig();
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Select Screen size (not available)
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
static int menu_screensize(int n)
{
#if 0
	int x,y,i,sel=n;
	
	for(;;){
		readpad();
		if(new_pad & CTRL_CIRCLE)
			return sel;
		else if(new_pad & CTRL_CROSS)
			return -1;
		else if(new_pad & CTRL_DOWN){
			sel++;
			if(sel>=SCR_END) sel=0;
		}else if(new_pad & CTRL_UP){
			sel--;
			if(sel<0) sel=SCR_END-1;
		}
		
		menu_frame("Select Screen Size", "○：OK　×：CANCEL");
		
		x=4, y=5;
		pgPrint(x++,y++,setting.color[3],"SCREEN SIZE:");
		for(i=0; i<SCR_END; i++){
			if(i==sel)
				pgPrint(x,y++,setting.color[2],scr_names[i]);
			else
				pgPrint(x,y++,setting.color[3],scr_names[i]);
		}
		
		pgScreenFlipV();
	}
#else
	pgScreenFlipV();
#endif
}

static int menu_frameskip(int sel)
{
#if 0
	char tmp[8];
	int x,y,i;
	
	strcpy(tmp,"0");
	
	for(;;){
		readpad();
		if(new_pad & CTRL_CIRCLE)
			return sel;
		else if(new_pad & CTRL_CROSS)
			return -1;
		else if(new_pad & CTRL_DOWN){
			sel++;
			if(sel>9) sel=0;
		}else if(new_pad & CTRL_UP){
			sel--;
			if(sel<0) sel=9;
		}
		
		menu_frame("Select Max Frame Skip", "○：OK　×：CANCEL");
		
		x=4, y=5;
		pgPrint(x++,y++,setting.color[3],"MAX FRAME SKIP:");
		for(i=0; i<=9; i++){
			tmp[0] = i + '0';
			if(i==sel)
				pgPrint(x,y++,setting.color[2],tmp);
			else
				pgPrint(x,y++,setting.color[3],tmp);
		}
		
		pgScreenFlipV();
	}
#else
	pgScreenFlipV();
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Select quick save slot (not available)
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
static bool get_state_time(int slot, char *out)
{
	static dirent_t files[MAX_ENTRY];
	char state_name[MAX_PATH],state_path[MAX_PATH], tmp[MAX_PATH];
	int fd, nfiles, ret;
	char *p;

	// path
	strcpy(state_path, ws_rom_path);
	p = strrchr(state_path, '/');
	*(++p) = 0;

	// name
	p = strrchr(ws_rom_path, '/');
	strcpy(tmp, ++p);
	p = strrchr(tmp, '.');
	*p = 0;
	sprintf(state_name, "%s.sv%d", tmp, slot);

	fd = sceIoDopen(state_path);
	for (nfiles = 0; nfiles < MAX_ENTRY; nfiles++) {
		if ((ret = sceIoDread(fd, &files[nfiles])) <= 0) break;
		if (!stricmp(files[nfiles].name, state_name))    break;
	}
	sceIoDclose(fd);

	if (ret > 0 && nfiles < MAX_ENTRY) 
		sprintf(out, "%04d/%02d/%02d %02d:%02d:%02d", files[nfiles].mtime.year,
		                                              files[nfiles].mtime.mon,
		                                              files[nfiles].mtime.mday,
		                                              files[nfiles].mtime.hour,
		                                              files[nfiles].mtime.min,
		                                              files[nfiles].mtime.sec);
	else
		strcpy(out, "NOT EXIST");

	return 1;
}

#if 0
static int menu_stateslot(statemethod method)
{
	char msg[STATE_SLOT_MAX+1][32], path[MAX_PATH], name[MAX_NAME], tmp[8];
	char thumbnailPath[MAX_PATH];
	uint16 thumbnail[112*128];
	boolean bThumbnail = FALSE;
	int selOld = -1;
	int x,y,i,j,fd,sel=0;

	GetStatePath(path, sizeof(path));
	_strcpy(name, NES_ROM_GetRomName());
	_strcpy(thumbnailPath, path);
	_strcat(thumbnailPath, name);
	_strcat(thumbnailPath, ".tn0");
	_strcat(name,".ss0");

	int nfiles = 0;
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		nfiles++;
	}
	sceIoDclose(fd);

	for(i=0; i<STATE_SLOT_MAX; i++){
		_strcpy(msg[i],"0 - ");
		msg[i][0] = name[_strlen(name)-1] = i + '0';
		for(j=0; j<nfiles; j++){
			if(!_stricmp(name,files[j].name)){
				_itoa(files[j].mtime.year,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],"/");

				if(files[j].mtime.mon < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.mon,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],"/");

				if(files[j].mtime.mday < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.mday,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i]," ");

				if(files[j].mtime.hour < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.hour,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],":");

				if(files[j].mtime.min < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.min,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],":");

				if(files[j].mtime.sec < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.sec,tmp);
				_strcat(msg[i],tmp);

				break;
			}
		}
		if(j>=nfiles){
			_strcat(msg[i],"None");
		}
	}

	for(;;){
		readpad();
		if(new_pad & CTRL_CIRCLE)
			return sel;
		else if(new_pad & CTRL_CROSS)
			return -1;
		else if(new_pad & CTRL_DOWN){
			sel++;
			if(sel>=STATE_SLOT_MAX) sel=0;
		}else if(new_pad & CTRL_UP){
			sel--;
			if(sel<0) sel=STATE_SLOT_MAX-1;
		}else if(new_pad & CTRL_RIGHT){
			sel+=STATE_SLOT_MAX/2;
			if(sel>=STATE_SLOT_MAX) sel=STATE_SLOT_MAX-1;
		}else if(new_pad & CTRL_LEFT){
			sel-=STATE_SLOT_MAX/2;
			if(sel<0) sel=0;
		}

		if (selOld != sel) {
			thumbnailPath[_strlen(thumbnailPath)-1] = sel + '0';
			bThumbnail = LoadThumnailFile(thumbnailPath, (uint16*)thumbnail);
			selOld = sel;
		}

		if(method == SAVE_STATE)
			menu_frame("Select State Save Slot", "○：OK　×：CANCEL");
		else
			menu_frame("Select State Load Slot", "○：OK　×：CANCEL");

		x=4, y=5;
		if(method == SAVE_STATE)
			pgPrint(x++,y++,setting.color[3],"SAVE STATE:");
		else
			pgPrint(x++,y++,setting.color[3],"LOAD STATE:");
		for(i=0; i<STATE_SLOT_MAX; i++){
			if(i==sel)
				pgPrint(x,y++,setting.color[2],msg[i]);
			else
				pgPrint(x,y++,setting.color[3],msg[i]);
		}

		if (bThumbnail) {
			pgDrawFrame(300-1,50-1,300+128,50+112,setting.color[1]);
			pgDrawFrame(300-2,50-2,300+128+1,50+112+1,setting.color[1]);
			pgBitBlt(300,50,128,112,1,(uint16*)thumbnail);
		}

		pgScreenFlipV();
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Save configuration to file
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
// by kwn
void save_config(void)
{
	char ws_cfg_path[MAX_PATH];
	char *p;
	
	strcpy(ws_cfg_path, ws_app_path);
	p = strrchr(ws_cfg_path, '/')+1;
	strcpy(p, "PSWAN.CFG");

	int fd;
	fd = ms_fopen(ws_cfg_path, "w");
	if (fd >= 0){
		ms_fwrite(&setting, sizeof(setting), 1, fd);
		ms_fclose(fd);
	} else {
		pgMessage("config file save failed.", MSG_ERROR, 100);
	}
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Load configuration from file
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
// by kwn
void load_config(void)
{
	int i;
	char ws_cfg_path[MAX_PATH];
	char *p;
	
	memset(&setting, 0, sizeof(setting));
	
	strcpy(ws_cfg_path, ws_app_path);
	p = strrchr(ws_cfg_path, '/')+1;
	strcat(p, "PSWAN.CFG");

	int fd;
	fd = ms_fopen(ws_cfg_path, "r");
	if (fd >= 0) {
		ms_fread(&setting, sizeof(setting), 1, fd);
		ms_fclose(fd);

		if (setting.state_slot < 0 || setting.state_slot > STATE_SLOT_MAX) setting.state_slot = 0;
		if (setting.key_config[BUTTON_MENU]  == 0)	setting.key_config[BUTTON_MENU]  = CTRL_RTRIGGER;
		if (setting.key_config[BUTTON_A]     == 0)	setting.key_config[BUTTON_A]     = CTRL_CIRCLE;
		if (setting.key_config[BUTTON_B]     == 0)	setting.key_config[BUTTON_B]     = CTRL_CROSS;
		if (setting.key_config[BUTTON_X1]    == 0)	setting.key_config[BUTTON_X1]    = CTRL_UP;
		if (setting.key_config[BUTTON_X2]    == 0)	setting.key_config[BUTTON_X2]    = CTRL_RIGHT;
		if (setting.key_config[BUTTON_X3]    == 0)	setting.key_config[BUTTON_X3]    = CTRL_DOWN;
		if (setting.key_config[BUTTON_X4]    == 0)	setting.key_config[BUTTON_X4]    = CTRL_LEFT;
		if (setting.key_config[BUTTON_START] == 0)	setting.key_config[BUTTON_START] = CTRL_START;
		if (setting.bgbright < 0 || 100 < setting.bgbright)	setting.bgbright       = 100;

#if 1
		setting.state_slot     = 0;
		setting.sampling_rate  = 2;
		setting.sound_part[4]  = 0;
		setting.sound_part[5]  = 0;
		setting.sound_part[6]  = 0;
		setting.wait = 1;
#endif
		if(!strcmp(setting.vercnf, PSWAN_CFG_VERSION))
			return;
	}
	
	set_default_key();
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Draw menu frame
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
void menu_frame(const char *msg0, const char *msg1)
{
	char buf[128];

	if(bBitmap)
		pgBitBlt(0,0,480,272,1,bgBitmap);
	else
		pgFillvram(setting.color[0]);
	sprintf(buf, " %c pSwan Ver0.07 %c", 9, 9);
	pgPrint(39, 1, setting.color[1], buf);

	// メッセージなど
//	if(msg0) mh_print(17, 14, msg0, setting.color[2]);
	if(msg0) pgPrint(2, 2, setting.color[2], msg0);
	pgDrawFrame(17,25,463,248,setting.color[1]);
	pgDrawFrame(18,26,462,247,setting.color[1]);

	// 操作説明
//	if(msg1) mh_print(17, 252, msg1, setting.color[2]);
	if(msg1) pgPrint(2, 32, setting.color[2], msg1);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
// Main menu
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
extern char ws_save_path[MAX_PATH];
extern int  app_rotated;
extern int  app_gameRunning;
extern char ws_rom_name[MAX_NAME];
extern int  ws_colourScheme;
extern int  ws_system;
void main_menu(int fFirst)
{
	enum
	{
		LOAD_ROM,
		STATE_SLOT,
		SAVE_STATE,
		LOAD_STATE,
		PREFERENCES_CONFIG,
		GRAPHIC_CONFIG,
		SOUND_CONFIG,
		MENU_COLOR_CONFIG,
		KEY_CONFIG,
		RESET,
		CONTINUE,
		EXIT_GAME,
	};
	char msg0[256], msg1[128], statefile[MAX_PATH], szSlotNum[128], szTmp[32], state_time[128];
	char buf[128];
	int  color;

	uint16 thumbnail[112*128];
#if 0
	boolean bAvailableThumbnail = FALSE;
	boolean bRefreshThumbnail = TRUE;
#endif
	int selOld = -1;
	static int sel=0;
	int x, y, ret;
	int bSave, fd, romsize, ramsize;
	int nCursor = 0;
	char *p;

	old_pad = 0;
	readpad();
	old_pad = paddata.buttons;

	msg0[0]=0;
//	state_time[0]='\0';

	load_menu_bg();

	if (!fFirst)
		get_state_time(setting.state_slot, state_time);

	for(;;){
		// キー操作
		readpad();
		if(new_pad & CTRL_CIRCLE){
			if (sel == LOAD_ROM) {
				if(getFilePath(ws_rom_path)){

					// Save played game SRAM
					if (!fFirst) ws_saveSRAM(ws_save_path);

					// Load ROM
					p = strrchr(ws_rom_path, '/');
					strcpy(ws_rom_name, ++p);
					strcpy(ws_save_path, ws_rom_path);
					p = strrchr(ws_save_path, '/');
					*++p = 0;
					strcat(ws_save_path, ws_rom_name);
					p = strrchr(ws_save_path, '.');
					*++p = 0;
					strcat(ws_save_path, "sav");
					
					if (ws_init(ws_rom_path)) {
						app_rotated=ws_rotated();
						app_gameRunning=1;
						ws_set_system(ws_system);
						ws_set_colour_scheme(ws_colourScheme);
//						ws_buildHalfBrightnessTable();
						ws_reset();
						ws_loadSRAM(ws_save_path);
						break;
					} else {
						strcpy(msg0, "Load failed");
					}
				}
			}else if(sel == STATE_SLOT){
#if 0
				setting.state_slot = (++setting.state_slot) % STATE_SLOT_MAX;
				bRefreshThumbnail = TRUE; // refresh thumbnail
#else
				setting.state_slot = 0;
#endif
			}else if(sel == SAVE_STATE){
#if 0
				ret = submenu_stateslot(SAVE_STATE);
				if(ret>=0){
					if(PSPEMU_SaveState(ret)) {
						_strcpy(msg0, "State Saved Successfully");
						bRefreshThumbnail = TRUE;
					}
					else
						_strcpy(msg0, "State Save Failed");
				}
#else
				if (ws_saveState()) break;
#endif
			}else if(sel == LOAD_STATE){
#if 0
				ret = submenu_stateslot(LOAD_STATE);
				if(ret>=0){
					if(PSPEMU_LoadState(ret))
						break;
					else
						_strcpy(msg0, "State Load Failed");
				}
#else
				if (ws_loadState()) break;
#endif
			}else if(sel == PREFERENCES_CONFIG){
#if 0
				menu_preferencesconfig();
#endif
				msg0[0]=0;
			}else if(sel == GRAPHIC_CONFIG){
#if 0
				menu_graphicsconfig();
#endif
				msg0[0]=0;
			}else if(sel == SOUND_CONFIG){
#if 1
				menu_soundconfig();
				msg0[0]=0;
#else
				setting.sound = ++setting.sound % 3;
#endif
			}else if(sel == MENU_COLOR_CONFIG){
				menu_colorconfig();
				msg0[0]=0;
			}else if(sel == KEY_CONFIG){
				menu_keyconfig();
				msg0[0]=0;
			}else if(sel == RESET){
				// Save played game SRAM
				ws_saveSRAM(ws_save_path);

				pgScreenClear();
				// Reload ROM
				if (ws_init(ws_rom_path)) {
					app_rotated=ws_rotated();
					app_gameRunning=1;
					ws_set_system(ws_system);
					ws_set_colour_scheme(ws_colourScheme);
//					ws_buildHalfBrightnessTable();
					ws_reset();
					ws_loadSRAM(ws_save_path);
					break;
				} else {
					strcpy(msg0, "load failed");
				}
			}else if(sel == CONTINUE){
				break;
			} else if (sel == EXIT_GAME) {
				if (!fFirst)  ws_saveSRAM(ws_save_path);
				save_config();
				sceKernelExitGame(); 
			}
		}else if(new_pad & CTRL_CROSS){
			if (!fFirst) break;
		}else if(new_pad & CTRL_LEFT){
			if (sel <= EXIT_GAME) sel = LOAD_ROM;
		}else if(new_pad & CTRL_RIGHT){
			if (sel >= LOAD_ROM) sel = EXIT_GAME;
		}else if(setting.key_config[BUTTON_MENU] && (new_pad&setting.key_config[BUTTON_MENU])==setting.key_config[BUTTON_MENU]){
			if (!fFirst) break;
		}else if(new_pad & CTRL_UP){
			if (fFirst) {
#if 1
				     if (sel == SOUND_CONFIG)       sel = LOAD_ROM;
#else
				     if (sel == SOUND_CONFIG)       sel = STATUS_SLOT;
#endif
				else if (sel == PREFERENCES_CONFIG) sel = LOAD_STATE;
				else if (sel == EXIT_GAME)          sel = KEY_CONFIG;
				else if (sel == LOAD_ROM)           sel = EXIT_GAME;
				else                                sel--;
			}
#if 1
			else {
				     if (sel == SOUND_CONFIG) sel = LOAD_STATE;
                else if (sel == SAVE_STATE)   sel = LOAD_ROM;
#endif
				else if (sel == LOAD_ROM)     sel = EXIT_GAME;
				else                          sel--;
#if 1
			}
#endif
		}else if(new_pad & CTRL_DOWN){
			if (fFirst) {
#if 1
				     if (sel == LOAD_ROM)   sel = SOUND_CONFIG;
#endif
				else if (sel == KEY_CONFIG) sel = EXIT_GAME;
				else if (sel == EXIT_GAME)  sel = LOAD_ROM;
				else                        sel++;
			}
#if 1
			else {
				     if (sel == LOAD_ROM)   sel = SAVE_STATE;
#endif
				else if (sel == LOAD_STATE) sel = SOUND_CONFIG;
				else if (sel == EXIT_GAME)  sel = LOAD_ROM;
				else                        sel++;
#if 1
			}
#endif
		}
#if 0
		if (bRefreshThumbnail) {
			// thumbnail呼び出し
			char thumbnailPath[MAX_PATH];
			GetStatePath(thumbnailPath, sizeof(thumbnailPath));
			_strcat(thumbnailPath, NES_ROM_GetRomName());
			_strcat(thumbnailPath, ".tn0");
			thumbnailPath[_strlen(thumbnailPath)-1] = setting.state_slot + '0';
			bAvailableThumbnail = LoadThumnailFile(thumbnailPath, (uint16*)thumbnail);
			// get timestamp string
			_strcpy(statefile, NES_ROM_GetRomName());
			_strcat(statefile, ".ss0");
			statefile[_strlen(statefile)-1] = setting.state_slot + '0';
			GetStateTime(statefile, state_time);
			bRefreshThumbnail = FALSE;
		}
#endif
		
		// メニュー表示
//		menu_frame(msg0, "○：OK　×：CANCEL");
		if (fFirst) sprintf(msg1, "%c:OK", 1);
		else        sprintf(msg1, "%c:OK %c:CANCEL", 1, 2);
		menu_frame(msg0, msg1);

		if (fFirst) color = setting.color[2];
		else        color = setting.color[3];

		x = 2;
		y = 5;

		sprintf(buf, "  LOAD ROM      %c", 127);
		pgPrint(x, y++, setting.color[3], buf);
		y++;

		if (fFirst) {
			sprintf(buf, "  STATE SLOT NUMBER : %d", setting.state_slot);
#if 0
			pgPrint(x, y++, setting.color[3], buf);
#else
			pgPrint(x, y++, setting.color[2], buf);
#endif
		} else {
			sprintf(buf, "  STATE SLOT NUMBER : %d - %s", setting.state_slot, state_time);
			pgPrint(x, y++, setting.color[2], buf);
		}
		pgPrint(x, y++, color, "  SAVE STATE");
		pgPrint(x, y++, color, "  LOAD STATE");
		y++;

		sprintf(buf, "  PREFERENCES   %c", 127);
#if 0
		pgPrint(x, y++, setting.color[3], buf);
#else
		pgPrint(x, y++, setting.color[2], buf);
#endif
		y++;

		sprintf(buf, "  GRAPHICS      %c", 127);
#if 0
		pgPrint(x, y++, setting.color[3], buf);
#else
		pgPrint(x, y++, setting.color[2], buf);
#endif
		y++;

		sprintf(buf, "  SOUND         %c", 127);
		pgPrint(x, y++, setting.color[3], buf);
		y++;

		sprintf(buf, "  MENU COLOR    %c", 127);
		pgPrint(x, y++, setting.color[3], buf);
		y++;

		sprintf(buf, "  KEY CONFIG    %c", 127);
		pgPrint(x, y++, setting.color[3], buf);
		y++;

		pgPrint(x, y++, color, "  Reset");
		pgPrint(x, y++, color, "  Continue");

		pgPrint(x, y++, setting.color[3], "  Exit Game");

		// カーソル表示
		y = sel + 5;
		if (sel >= STATE_SLOT)         y++;
		if (sel >= PREFERENCES_CONFIG) y++;
		if (sel >= GRAPHIC_CONFIG)     y++;
		if (sel >= SOUND_CONFIG)       y++;
		if (sel >= MENU_COLOR_CONFIG)  y++;
		if (sel >= KEY_CONFIG)         y++;
		if (sel >= RESET)              y++;

		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1) %10;
#if 0
		if (bAvailableThumbnail && sel == STATE_SLOT) {
			pgDrawFrame(300-1,50-1,300+128,50+112,setting.color[1]);
			pgDrawFrame(300-2,50-2,300+128+1,50+112+1,setting.color[1]);
			pgBitBlt(300,50,128,112,1,(uint16*)thumbnail);
		}
#endif
		pgScreenFlipV();
	}

	save_config();
	pgScreenClear();
	pgWaitVn(10);
	memset(&paddata, 0x00, sizeof(paddata));

}
