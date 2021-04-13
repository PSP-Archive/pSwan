			nTime=timeGetTime()-nNormalLast;					  // calcule le temps écoulé depuis le dernier affichage
																  // nTime est en mili-secondes.
			// détermine le nombre de trames à passer + 1
			nCount=(nTime*600 - nNormalFrac) /10000; 	

			// si le nombre de trames à passer + 1 est nul ou négatif,
			// ne rien faire pendant 2 ms
			if (nCount<=0) 
			{ 
#if 0
				Sleep(2);
#else
				pgWaitVn(2);
#endif
			} // No need to do anything for a bit
			else
			{
				nNormalFrac+=nCount*10000;				// 
				nNormalLast+=nNormalFrac/600;				// add the duration of nNormalFrac frames
				nNormalFrac%=600;							// 

				// Pas plus de 9 (10-1) trames non affichées 
				if (nCount>10) 
				  nCount=10; 

/*
				ws_key_start=0;
				ws_key_left=0;
				ws_key_right=0;
				ws_key_up=0;
				ws_key_down=0;
				ws_key_button_1=0;
				ws_key_button_2=0;
*/
				int ws_key_esc=0;

#if 0
				#include "./source/temp/key.h"
#else
				#include "temp/key.h"
#endif
				if (ws_key_esc)
				{
					console.close();
					if (ws_rom_path)
						strcpy(old_rom_path,ws_rom_path);
					gui_open();

					if ((ws_rom_path!=NULL)||(app_terminate))
						break;
					if (gui_command)
					{
						if (gui_command==GUI_COMMAND_RESET)
							ws_reset();
						if (gui_command==GUI_COMMAND_SCHEME_CHANGE)
							ws_set_colour_scheme(ws_colourScheme);
						if (gui_command==GUI_COMMAND_FILTER_CHANGE)
						{
							ws_saveState("oswan.wss");
							ws_rom_path=old_rom_path;
							delete surface;
							return;
						}
					}
					console.option("DirectX");
					if (app_fullscreen) 
						console.option("fullscreen output"); 
					else 
						console.option("windowed output");
					console.option("fixed window");
					console.option("center window");
