/******************************************************************************
 *                                                                            *
 *      sceCtrl.h                                                             *
 *                                                                            *
 *      module sceController_Service                                          *
 *                                                                            *
 ******************************************************************************/

#ifndef _SCECTRL_H
#define _SCECTRL_H
//-----------------------------------------------------------------------------

// Index for the two analog directions
#define CTRL_ANALOG_X	0
#define CTRL_ANALOG_Y	1

// Button bit masks
#define CTRL_SQUARE		0x8000
#define CTRL_TRIANGLE	0x1000
#define CTRL_CIRCLE		0x2000
#define CTRL_CROSS		0x4000
#define CTRL_UP			0x0010
#define CTRL_DOWN		0x0040
#define CTRL_LEFT		0x0080
#define CTRL_RIGHT		0x0020
#define CTRL_START		0x0008
#define CTRL_SELECT		0x0001
#define CTRL_LTRIGGER	0x0100
#define CTRL_RTRIGGER	0x0200
#define CTRL_HOLD		0x00020000

// Returned control data
typedef struct _ctrl_data {
	unsigned long frame;
	unsigned long buttons;
	unsigned char analog[4];
	unsigned long unused;
} ctrl_data_t;

//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------

// Not 100% sure on this, init with 0
void sceCtrlInit(int unknown);
//-----------------------------------------------------------------------------

// Pass 1 to enable analogue mode
void sceCtrlSetAnalogMode(int on);
//-----------------------------------------------------------------------------

// Read in the controller data. Unknown should be set to 1
void sceCtrlRead(ctrl_data_t *paddata, int unknown);
//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
