#ifndef MENU_H
#define MENU_H

enum{
	STATE_SLOT_MAX=4,
};

enum{
	SCR_X1,
	SCR_X15,
	SCR_X2_UNCROPED,
	SCR_X2_FIT,
	SCR_X2_SCANLINE,
	SCR_X2_UTOP,
	SCR_X2_UBOTTOM,
	SCR_FULL,
	SCR_END,
};

enum{
	PAL_MONOCHROME,
	PAL_BRIGHT_SEPIA,
	PAL_RED,
	PAL_DARK_SEPIA,
	PAL_PASTEL_COLOR,
	PAL_ORANGE,
	PAL_YELLOW,
	PAL_BRIGHT_BLUE,
	PAL_DARK_BLUE,
	PAL_GRAY,
	PAL_BRIGHT_GREEN,
	PAL_DARK_GREEN,
	PAL_NEGA_POSI,
	PAL_END,
};

extern unsigned char org_gbtype;
extern int QuickSlot;

void save_config(void);
void load_config(void);
void menu_frame(const char *msg0, const char *msg1);
void main_menu(int bFirst);

#endif
