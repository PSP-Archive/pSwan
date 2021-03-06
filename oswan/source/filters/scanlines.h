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
void ws_emulate_scanlines(void)
{
	#include "filter_partA.h"
	if (app_rotated)
	{
		surface=new Surface(144*2,224*2,format);
		#include "filter_partB.h"
		console.open(app_window_title,144*2,224*2,format);
		while (1)
		{
				#include "filter_partC.h"
				console.open(app_window_title,144*2,224*2,format);
				#include "filter_partD.h"
				ws_rotate_backbuffer(backbuffer);
				int16 *vs = (int16 *)surface->lock();
				int16	*backbuffer_alias=backbuffer;
				for (int line=0;line<224;line++)
				{
					ws_drawDoubledRotatedScanline(vs,backbuffer_alias);
					vs+=surfacePitch;
					vs+=surfacePitch;
					backbuffer_alias+=144;
				}
				surface->unlock();
				surface->copy(console);
				console.update();
			}
		}
		#include "filter_partE.h"
	}
	else
	{
		surface=new Surface(224*2,144*2,format);
		#include "filter_partB.h"
		console.open(app_window_title,224*2,144*2,format);
		while (1)
		{
				#include "filter_partC.h"
				console.open(app_window_title,224*2,144*2,format);
				#include "filter_partD.h"
				int16 *vs = (int16 *)surface->lock();
				int16	*backbuffer_alias=backbuffer;
#ifdef __cplusplus
				for (int line=0;line<144;line++)
#else
				int line;
				for (line=0;line<144;line++)
#endif
				{
					ws_drawDoubledScanline(vs,backbuffer_alias);
					vs+=surfacePitch;
					vs+=surfacePitch;
					backbuffer_alias+=224;
				}
				surface->unlock();
				surface->copy(console);
				console.update();
			}
		}
		#include "filter_partE.h"
	}
}
