/*
 *  settingsMenu.c
 *
 *  Wii CoverFloader
 *  Copyright 2009 Beardface April 29th, 2009
 *  Additional coding by: gitkua, scognito, F1SHE4RS, afour98, blackbird399, LoudBob11, alexcarlosantao
 *  Licensed under the terms of the GNU GPL, version 2
 *  http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 *  This file contains the methods to draw the two primary settings screens:
 *    - CoverFloader Settings
 *    - Graphics Settings
 */
#include "settingsMenu.h"
#include "localization.h"

extern char** languages;
extern char** vidmodes;

extern s_self         self;
extern s_pointer      pointer;
extern s_settings     settings;
extern s_gameSettings gameSetting;
extern s_title*       titleList;
extern u8             shutdown;
extern u8             reset;

//hook types for ocarina
char hooks[3][9] =
{{"   VI"},
{" Wii Pad"},
{" GC Pad"}};

char ghooks[3][9] =
{{"      VI"},
{" Wii Pad"},
{" GC Pad"}};

/////////////////////////////////////////
// This method shows the Settings Menu //
/////////////////////////////////////////
void Settings_Menu_Show()
{
	enum
	{
		settingsPanel=0,
		graphicsPanel=1,
		langPanel=2,
		aboutPanel=3
	};
	
	bool doloop       = true;
	int  stateMachine = settingsPanel;  // default to 'settings' panel
	menuSettingsButton.selected  = true;
	
	//////////////////
	// Draw the intro - lower the menu
	int header_x = 0;
	int i = 1;
	int fade = 0x00;
	float moving_y;
	
	for(i = 0; i <= 20; i++)
	{
		// Draw the covers in the back
		draw_covers();
		GRRLIB_2D_Init();
		// Draw the screen fade
		GRRLIB_FillScreen(0x00000000|fade);
		fade+=5;
		//Draw the panel
		moving_y = change_scale(i, 0, 20, -240, 0);
		GRRLIB_Rectangle(70, moving_y, 500, 240, 0x44444499, true);
		moving_y = change_scale(i, 0, 20, 480, 240);
		GRRLIB_Rectangle(70, moving_y, 500, 240, 0x44444499, true);
		// Draw the top menu header across the screen
		moving_y = change_scale(i, 0, 20, -72, 0);
		for (header_x=0; header_x<=80; header_x++)
			GRRLIB_DrawImg(header_x*8, moving_y, menu_header_texture, 0, 1, 1, 0xFFFFFFFF);
		// Draw the bottom menu header across the screen
		moving_y = change_scale(i, 0, 20, 481, 408);
		for (header_x=0; header_x<=80; header_x++)
			GRRLIB_DrawImg(header_x*8, moving_y, menu_header_vflip_texture, 0, 1, 1, 0xFFFFFFFF);
		moving_y = change_scale(i, 0, 20, -52, 20);
		// Draw the buttons
		menuLogoButton.y = moving_y;
		menuSettingsButton.y = moving_y;
		menuGraphicsButton.y = moving_y;
		menuLanguagesButton.y = moving_y;
		Button_Paint(&menuLogoButton);
		Button_Menu_Paint(&menuSettingsButton);
		Button_Menu_Paint(&menuGraphicsButton);
		Button_Menu_Paint(&menuLanguagesButton);
		// Draw the pointer
		WPAD_ScanPads();
		GetWiimoteData();
		DrawCursor(0, pointer.p_x, pointer.p_y, pointer.p_ang, 1, 1, 0xFFFFFFFF);
		GRRLIB_Render();
	}
	
	//////////////////
	// Menu do loop
	do{
		// Check the remotes
		WPAD_ScanPads();
		PAD_ScanPads();
		GetWiimoteData();
		
		// Draw the covers in the back
		draw_covers();
		// Draw the screen fade
		GRRLIB_2D_Init();
		GRRLIB_FillScreen(0x00000064);
		//Draw the panel
		GRRLIB_Rectangle(70, 0, 500, 452, 0x44444499, true);
		// Draw the menu header across the screen
		for (header_x=0; header_x<=80; header_x++)
			GRRLIB_DrawImg(header_x*8, 0, menu_header_texture, 0, 1, 1, 0xFFFFFFFF);
		// Draw the bottom menu header across the screen
		for (header_x=0; header_x<=80; header_x++)
			GRRLIB_DrawImg(header_x*8, 408, menu_header_vflip_texture, 0, 1, 1, 0xFFFFFFFF);
		// Draw the top menu buttons
		Button_Menu_Paint(&menuSettingsButton);
		Button_Menu_Paint(&menuGraphicsButton);
		Button_Menu_Paint(&menuLanguagesButton);
		// Draw the logo
		Button_Paint(&menuLogoButton);
		
		// Check for B to save and exit
		if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_B)||((PAD_ButtonsDown(0) & PAD_BUTTON_B)))
		{
			WPAD_Rumble(0,0); // Kill the rumble
			doloop = false;
		}
		
		// Check for A clicks on top menu buttons
		if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_A)||((PAD_ButtonsDown(0) & PAD_BUTTON_B)))
		{
			if (Button_Select(&menuSettingsButton, pointer.p_x, pointer.p_y))
			{
				stateMachine = settingsPanel;
				menuSettingsButton.selected  = true;
				menuGraphicsButton.selected  = false;
				menuLanguagesButton.selected = false;
			}
			else if (Button_Select(&menuGraphicsButton, pointer.p_x, pointer.p_y))
			{
				stateMachine = graphicsPanel;
				menuSettingsButton.selected  = false;
				menuGraphicsButton.selected  = true;
				menuLanguagesButton.selected = false;
			}
			else if (Button_Select(&menuLanguagesButton, pointer.p_x, pointer.p_y))
			{
				stateMachine = langPanel;
				menuSettingsButton.selected  = false;
				menuGraphicsButton.selected  = false;
				menuLanguagesButton.selected = true;
			}
			else if (Button_Select(&menuLogoButton, pointer.p_x, pointer.p_y))
			{
				stateMachine = aboutPanel;
				menuSettingsButton.selected  = false;
				menuGraphicsButton.selected  = false;
				menuLanguagesButton.selected = false;
			}
		}
		
		////////////////////////////////////
		// Switch on which screen to display
		////////////////////////////////////
		switch (stateMachine)
		{	
			case settingsPanel:		// Case for Settings Panel
			{
				// Handle Button events
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_1) // Check for screen shot
					GRRLIB_ScrShot(USBLOADER_PATH "/sshot.png");
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
				{
					if (Button_Select(&settingsButton, pointer.p_x, pointer.p_y))
					{
						doloop = false; // Clicked the setting button, exit to main screen
					}
					else if (Button_Select(&vidtvonButton, pointer.p_x, pointer.p_y) || Button_Select(&vidtvoffButton, pointer.p_x, pointer.p_y))
					{
						settings.vipatch = (settings.vipatch) ? 0 : 1; // Clicked the VIPATCH button, toggle state
					}
					else if (Button_Select(&cheatonButton, pointer.p_x, pointer.p_y) || Button_Select(&cheatoffButton, pointer.p_x, pointer.p_y))
					{
						settings.ocarina = (settings.ocarina) ? 0 : 1; // Clicked the Ocarina button, toggle state
					}
					else if (Button_Select(&viddownButton, pointer.p_x,pointer.p_y))
					{
						// Clicked on the video down button
						if (settings.video > 0)
							settings.video --;
						else
							settings.video = (CFG_VIDEO_COUNT -1);
					}
					else if (Button_Select(&vidupButton, pointer.p_x,pointer.p_y))
					{
						// Clicked on the video up button
						if (settings.video < (CFG_VIDEO_COUNT -1))
							settings.video ++;
						else
							settings.video = 0;
					}
					else if (Button_Select(&hookdownButton, pointer.p_x, pointer.p_y))
					{ // Clicked on the hooktype buttons
						if (settings.hooktype > 0)
							settings.hooktype --;
						else
							settings.hooktype = (CFG_HOOK_COUNT - 1);
					}
					else if (Button_Select(&hookupButton, pointer.p_x, pointer.p_y))
					{
						if (settings.hooktype < (CFG_HOOK_COUNT - 1))
							settings.hooktype ++;
						else
							settings.hooktype = 0;
					}
					else if (Button_Select(&coversButton, pointer.p_x, pointer.p_y))
					{
						// Clicked on the Download Covers button
						if (WindowPrompt(TX.coverDownload, TX.opNoCancel , &okButton, &cancelButton))
						{
							WPAD_Rumble(0,0); //sometimes rumble remain active
							if(networkInit(self.ipAddress))
							{
								batchDownloadCover(self.gameList);
								CoversDownloaded();
							}
							else
								WindowPrompt(TX.error, TX.iniNetErr , &okButton, 0);
						}
					}
					else if(Button_Select(&titlesButton, pointer.p_x, pointer.p_y))
					{
						WPAD_Rumble(0,0); //sometimes rumble remain active
						if(networkInit(self.ipAddress)){
							if(!downloadTitles())
								WindowPrompt( TX.error, TX.errTitles, &okButton, 0);
							else
							{
								if(self.usingTitlesTxt){
									self.usingTitlesTxt = false;
									self.titlesTxtSize = 0;
									free(titleList);
								}
								
								int numLines = initTitle();
								if(numLines > 0){
									self.usingTitlesTxt = true;
									self.titlesTxtSize = numLines;
									titleList = (s_title *) malloc (numLines * sizeof(s_title));
									fillTitleStruct(titleList, numLines);
									char numLinesTxt[250];
									sprintf(numLinesTxt, TX.successTitles, numLines);
									WindowPrompt(TX.Success, numLinesTxt, &okButton, 0);
								}
							}
						}
						else
							WindowPrompt(TX.error, TX.errNetTitles, &okButton, 0);
					}
					else if (Button_Select(&themeBlackButton, pointer.p_x, pointer.p_y))
					{
						if (settings.theme)
						{
							themeWhiteButton.selected = false;
							themeBlackButton.selected = true;
							settings.theme = 0; // black
							GRRLIB_SetBGColor(0); // set BG to black
						}
					}
					else if (Button_Select(&themeWhiteButton, pointer.p_x, pointer.p_y))
					{
						if (!settings.theme)
						{
							themeWhiteButton.selected = true;
							themeBlackButton.selected = false;
							settings.theme = 1; // white
							GRRLIB_SetBGColor(1); // set BG to white
						}
					}
					else if (Button_Select(&quickstartOnButton, pointer.p_x, pointer.p_y) || Button_Select(&quickstartOffButton, pointer.p_x, pointer.p_y))
					{
						settings.quickstart = (settings.quickstart) ? 0 : 1; // Clicked the "1-Click Launch" button, toggle state
					}
					else if (Button_Select(&rumbleOnButton, pointer.p_x, pointer.p_y) || Button_Select(&rumbleOffButton, pointer.p_x, pointer.p_y))
					{
						settings.rumble = (settings.rumble) ? 0 : 1; // Clicked the Rumble button, toggle state
					}
					else if (Button_Select(&musicOnButton, pointer.p_x, pointer.p_y) || Button_Select(&musicOffButton, pointer.p_x, pointer.p_y))
					{
						settings.music = (settings.music) ? 0 : 1; // Clicked the music button, toggle state
					}
					
				}
				
				// Draw the Background boxes
				GRRLIB_Rectangle(118, 90, 434, 144, 0xffffffdd, true);
				GRRLIB_Rectangle(120, 92, 430, 140, 0x737373FF, true);
				GRRLIB_Rectangle(118, 258, 434, 144, 0xffffffdd, true);
				GRRLIB_Rectangle(120, 260, 430, 140, 0x737373FF, true);
				GRRLIB_DrawImg(80, 76, dialog_box_titlebar_texture, 0, 1, 1, 0xFFFFFFFF);
				GRRLIB_DrawImg(80, 244, dialog_box_titlebar_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 158,96, TX.basic, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 158,264,TX.advanced, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				// Draw the attributes labels
				CFreeTypeGX_DrawText(ttf16pt, 300,116, TX.sound, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 300,150, TX.rumble, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 300,185, TX.oneClickLaunch, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 300,219, TX.theme, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 300,285, TX.getAddData, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 300,318, TX.patchVIDTV, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 300,352, TX.videoMode, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 450,352, vidmodes[settings.video], (GXColor){0xff, 0xff, 0xff, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 300,387, TX.ocarina, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
				CFreeTypeGX_DrawText(ttf16pt, 510,387, hooks[settings.hooktype], (GXColor){0xff, 0xff, 0xff, 0xff}, FTGX_JUSTIFY_CENTER);
				// Draw buttons
				Button_TTF_Toggle_Paint(&musicOffButton, &musicOnButton, TX.toggleOffB, TX.toggleOnB, settings.music);
				Button_TTF_Toggle_Paint(&rumbleOffButton, &rumbleOnButton, TX.toggleOffB, TX.toggleOnB, settings.rumble);
				Button_TTF_Toggle_Paint(&quickstartOffButton, &quickstartOnButton, TX.toggleOffB, TX.toggleOnB, settings.quickstart);
				if (settings.theme)
				{
					themeWhiteButton.selected = true;
					themeBlackButton.selected = false;
				}
				else
				{
					themeWhiteButton.selected = false;
					themeBlackButton.selected = true;
				}
				Button_TTF_Paint(&themeBlackButton);
				Button_TTF_Paint(&themeWhiteButton);
				Button_TTF_Paint(&coversButton);
				Button_TTF_Paint(&titlesButton);
				Button_TTF_Toggle_Paint(&vidtvoffButton, &vidtvonButton, TX.toggleOffB, TX.toggleOnB, settings.vipatch);
				Button_Paint(&vidupButton);
				Button_Paint(&viddownButton);
				Button_TTF_Toggle_Paint(&cheatoffButton, &cheatonButton, TX.toggleOffB, TX.toggleOnB, settings.ocarina);
				Button_Paint(&hookupButton);
				Button_Paint(&hookdownButton);
				
				// Check for button-pointer intersections, and rumble
				if (Button_Hover(&menuSettingsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuGraphicsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuLanguagesButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&quickstartOnButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&quickstartOffButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&musicOnButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&musicOffButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&rumbleOnButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&rumbleOffButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&themeBlackButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&themeWhiteButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&coversButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&titlesButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&vidtvonButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&vidtvoffButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&vidupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&viddownButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&cheatoffButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&cheatonButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&hookupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&hookdownButton, pointer.p_x, pointer.p_y) )
				{
					if (--self.rumbleAmt > 0) // Should we be rumbling?
					{
						if(settings.rumble)
							WPAD_Rumble(0,1); // Turn on Wiimote rumble
					}
					else 
						WPAD_Rumble(0,0); // Kill the rumble
				}
				else
				{ // If no button is being hovered, kill the rumble
					WPAD_Rumble(0,0);
					self.rumbleAmt = 4;
				}
				
				break;
			} // end case for 'settings' panel
				
			case graphicsPanel:		// Case for graphics
			{
				// Handle Button A events
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
				{
					if (Button_Select(&covers3dOnButton, pointer.p_x, pointer.p_y) || Button_Select(&covers3dOffButton, pointer.p_x, pointer.p_y))
					{
						settings.covers3d = (settings.covers3d) ? 0 : 1;
						ResetBuffer();
					}
					else if (Button_Select(&hidescrollOnButton, pointer.p_x, pointer.p_y) || Button_Select(&hidescrollOffButton, pointer.p_x, pointer.p_y))
					{
						settings.hideScroll = (settings.hideScroll) ? 0 : 1;
					}
					else if (Button_Select(&windowdownButton, pointer.p_x, pointer.p_y))
					{
						if (settings.drawWindow > 1)
							settings.drawWindow -= 1;
					}
					else if (Button_Select(&windowupButton, pointer.p_x, pointer.p_y))
					{
						if (settings.drawWindow < 40)
							settings.drawWindow += 1;
					}
					else if (Button_Select(&coverTextOnButton, pointer.p_x, pointer.p_y) || Button_Select(&coverTextOffButton, pointer.p_x, pointer.p_y))
						settings.coverText = (settings.coverText) ? 0 : 1;
				}
				if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_A)
				{
					if(Button_Select(&spacingdownButton, pointer.p_x, pointer.p_y))
					{
						if (settings.coverSpacing <= 2)
							settings.coverSpacing = 2; // sanity check
						else
							settings.coverSpacing -= 0.05;
					}
					else if(Button_Select(&spacingupButton, pointer.p_x, pointer.p_y))
					{
						if (settings.coverSpacing >= 9)
							settings.coverSpacing = 9; // sanity check
						else
							settings.coverSpacing += 0.05;
					}
					else if(Button_Select(&angledownButton, pointer.p_x, pointer.p_y))
					{
						if (settings.coverAngle <= -140)
							settings.coverAngle = -140; // sanity check
						else
							settings.coverAngle -= 1;
					}
					else if(Button_Select(&angleupButton, pointer.p_x, pointer.p_y))
					{
						if (settings.coverAngle >= 140)
							settings.coverAngle = 140; // sanity check
						else
							settings.coverAngle += 1;
					}
					else if(Button_Select(&zoomdownButton, pointer.p_x, pointer.p_y))
					{
						if (settings.coverZoom <= -8.0)
							settings.coverZoom = -8.0; // sanity check
						else
							settings.coverZoom -= 0.03;
					}
					else if(Button_Select(&zoomupButton, pointer.p_x, pointer.p_y))
					{
						if (settings.coverZoom >= .69)
							settings.coverZoom  = .69; // sanity check
						else
							settings.coverZoom += 0.03;
					}
					else if(Button_Select(&falloffdownButton, pointer.p_x, pointer.p_y))
						settings.coverFallOff -= 0.002;
					else if(Button_Select(&falloffupButton, pointer.p_x, pointer.p_y))
						settings.coverFallOff += 0.002;
				}
				// Draw the wireframe background
				GRRLIB_DrawImg(80, 130, menu_graphics_wireframe_texture, 0, 1, 1, 0xFFFFFFFF);
				// Draw the 5 Top boxes (falloff, window, zoom, spacaing, angle)
				GRRLIB_DrawImg(80, 94, menu_graphics_box1_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 126, 113, TX.falloff, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				GRRLIB_DrawImg(177, 80, menu_graphics_box1_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 223, 99, TX.drawWindow, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				GRRLIB_DrawImg(274, 80, menu_graphics_box1_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 320, 99, TX.zoom, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				GRRLIB_DrawImg(368, 80, menu_graphics_box1_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 414, 99, TX.spacing, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				GRRLIB_DrawImg(465, 94, menu_graphics_box1_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 511, 113, TX.angle, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				// Draw the 3 bottom boxes (scrollbar, 3D covers, title)
				GRRLIB_DrawImg(81, 341, menu_graphics_box2_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 153,360, TX.hideScrollbar, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				GRRLIB_DrawImg(248, 341, menu_graphics_box2_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 320,360, TX.covers3D, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				GRRLIB_DrawImg(415, 341, menu_graphics_box2_texture, 0, 1, 1, 0xFFFFFFFF);
				CFreeTypeGX_DrawText(ttf16pt, 487,360, TX.gameText, (GXColor){0xFF, 0xFF, 0xFF, 0xff}, FTGX_JUSTIFY_CENTER);
				// Draw the Title text
				if (settings.coverText)
					CFreeTypeGX_DrawText(ttf20pt, 320, 314, TX.gameTitle, (GXColor){0x22, 0x22, 0x22, 0xff}, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
				// Draw the scroll bar
				if (!settings.hideScroll)
					GRRLIB_DrawImg(120, 300, slidebar_texture, 0, .655, .655, 0xFFFFFFFF);
				// Draw the -/+ buttons
				Button_Paint(&zoomupButton);
				Button_Paint(&zoomdownButton);
				Button_Paint(&spacingupButton);
				Button_Paint(&spacingdownButton);
				Button_Paint(&angleupButton);
				Button_Paint(&angledownButton);
				Button_Paint(&falloffupButton);
				Button_Paint(&falloffdownButton);
				Button_Paint(&windowupButton);
				Button_Paint(&windowdownButton);
				// Draw the toggle buttons
				Button_TTF_Toggle_Paint(&hidescrollOffButton, &hidescrollOnButton, TX.toggleOffB, TX.toggleOnB, settings.hideScroll);
				Button_TTF_Toggle_Paint(&coverTextOffButton, &coverTextOnButton, TX.toggleOffB, TX.toggleOnB, settings.coverText);
				Button_TTF_Toggle_Paint(&covers3dOffButton, &covers3dOnButton, TX.toggleOffB, TX.toggleOnB, settings.covers3d);
				
				// Check for button-pointer intersections, and rumble
				if (Button_Hover(&menuSettingsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuGraphicsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuLanguagesButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&spacingupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&spacingdownButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&zoomupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&zoomdownButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&windowupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&windowdownButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&windowupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&coverTextOnButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&coverTextOffButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&angleupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&angledownButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&falloffupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&falloffdownButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&covers3dOnButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&covers3dOffButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&hidescrollOnButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&hidescrollOffButton, pointer.p_x, pointer.p_y))
					
				{
					if (--self.rumbleAmt > 0) // Should we be rumbling?
					{
						if(settings.rumble)
							WPAD_Rumble(0,1); // Turn on Wiimote rumble
					}
					else 
						WPAD_Rumble(0,0); // Kill the rumble
				}
				else
				{ // If no button is being hovered, kill the rumble
					WPAD_Rumble(0,0);
					self.rumbleAmt = 4;
				}
				
				break;
			} // end case for 'graphics' panel
				
			case langPanel:		// Case for languages
			{
				// Handle Button A events
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_1) // Check for screen shot
					GRRLIB_ScrShot(USBLOADER_PATH "/sshot.png");
				// TODO make the flag based selection work
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
				{
					if (Button_Select(&langdownButton, pointer.p_x, pointer.p_y))
					{ // Clicked on the language buttons
						if (settings.language > 0)
							settings.language --;
						else
							settings.language = (CFG_LANG_COUNT - 1);
					}
					else if (Button_Select(&langupButton, pointer.p_x, pointer.p_y))
					{
						if (settings.language < (CFG_LANG_COUNT - 1))
							settings.language ++;
						else
							settings.language = 0;
					}
				}
				
				// Draw Old School language changer
				// TODO replace this and make the flag based selection work
				CFreeTypeGX_DrawText(ttf18pt, 316, 110, languages[settings.language], (GXColor){0xff, 0xff, 0xff, 0xff}, FTGX_JUSTIFY_CENTER);
				//CFreeTypeGX_DrawText(ttf16pt, 320, 430, "The Flag selections do not work yet...", (GXColor){0x44, 0x44, 0x44, 0xff}, FTGX_JUSTIFY_CENTER);
				Button_Paint(&langupButton);
				Button_Paint(&langdownButton);
				
				
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
				{
					if (Button_Select(&flagUSButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "default");
						languageLoad();
					}
					else if (Button_Select(&flagDEButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "de-GER");
						languageLoad();
					}
					else if (Button_Select(&flagITButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "it-ITA");
						languageLoad();
					}
					else if (Button_Select(&flagUKButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "en-GB");
						languageLoad();
					}
					else if (Button_Select(&flagBRButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "pt-BR");
						languageLoad();
					}
					else if (Button_Select(&flagFRButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "fr-FRE");
						languageLoad();
					}
					else if (Button_Select(&flagDAButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "nl-DUT");
						languageLoad();
					}
					else if (Button_Select(&flagJPButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "ja-JPN");
						languageLoad();
					}
					else if (Button_Select(&flagPTButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "pt-BR");
						languageLoad();
					}
					else if (Button_Select(&flagTWButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "zh-TW");
						languageLoad();
					}
					else if (Button_Select(&flagRUButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "ru-RUS");
						languageLoad();
					}
					else if (Button_Select(&flagCNButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "zh-CN");
						languageLoad();
					}
					else if (Button_Select(&flagCTButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "ca-CAT");
						languageLoad();
					}
					else if (Button_Select(&flagESButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "es-SPA");
						languageLoad();
					}
					else if (Button_Select(&flagFIButton, pointer.p_x, pointer.p_y))
					{
						strcpy(settings.localLanguage, "fi-FIN");
						languageLoad();
					}
					
				}
				
				
				// Draw graphics stuff here
				Button_Flag_Paint(&flagUSButton);
				Button_Flag_Paint(&flagITButton);
				Button_Flag_Paint(&flagDEButton);
				Button_Flag_Paint(&flagUKButton);
				Button_Flag_Paint(&flagBRButton);
				Button_Flag_Paint(&flagFRButton);
				Button_Flag_Paint(&flagDAButton);
				//Button_Flag_Paint(&flagJPButton);
				Button_Flag_Paint(&flagFIButton);
				Button_Flag_Paint(&flagPTButton);
				//Button_Flag_Paint(&flagTWButton);
				Button_Flag_Paint(&flagRUButton);
				//Button_Flag_Paint(&flagCNButton);
				Button_Flag_Paint(&flagESButton);
				Button_Flag_Paint(&flagCTButton);
				
				
				// Check for button-pointer intersections, and rumble
				if (
					Button_Hover(&menuSettingsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuGraphicsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuLanguagesButton, pointer.p_x, pointer.p_y)||
					Button_Hover(&langupButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&langdownButton, pointer.p_x, pointer.p_y)||
					Button_Hover(&flagUSButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagITButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagDEButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagUKButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagBRButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagFRButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagDAButton, pointer.p_x, pointer.p_y) ||
					//Button_Hover(&flagJPButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagFIButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagPTButton, pointer.p_x, pointer.p_y) ||
					//Button_Hover(&flagTWButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagESButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagRUButton, pointer.p_x, pointer.p_y) ||
					//Button_Hover(&flagCNButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&flagCTButton, pointer.p_x, pointer.p_y)
				   )
				{
					if (--self.rumbleAmt > 0) // Should we be rumbling?
					{
						if(settings.rumble)
							WPAD_Rumble(0,1); // Turn on Wiimote rumble
					}
					else 
						WPAD_Rumble(0,0); // Kill the rumble
				}
				else
				{ // If no button is being hovered, kill the rumble
					WPAD_Rumble(0,0);
					self.rumbleAmt = 4;
				}
				
				break;
			}// end case for 'language' panel
				
			case aboutPanel:		// Case for About panel
			{
				GRRLIB_Rectangle(88, 88, 464, 306, 0xffffffdd, true);
				GRRLIB_Rectangle(90, 90, 460, 302, 0x737373FF, true);

				CFreeTypeGX_DrawText(ttf20pt, 320, 140, "About CoverFloader", (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 320, 170, "Copyright 2009 Beardface", (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 320, 210, "Licensed under the terms of the", (GXColor){0x22, 0x22, 0x22, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 320, 230, "GNU GPL version 2", (GXColor){0x22, 0x22, 0x22, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 320, 250, "http://code.google.com/p/wiicoverflow/", (GXColor){0x22, 022, 0x22, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf18pt, 320, 310, "Additional Development By:", (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 320, 330, "gitkua, scognito, F1SHE4RS, afour98,", (GXColor){0x22, 0x22, 0x22, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 320, 350, "blackbird399, LoudBob11, alexcarlosantao", (GXColor){0x22, 0x22, 0x22, 0xff}, FTGX_JUSTIFY_CENTER);
				CFreeTypeGX_DrawText(ttf16pt, 320, 430, "Flag icons courtesy of www.icondrawer.com", (GXColor){0x44, 0x44, 0x44, 0xff}, FTGX_JUSTIFY_CENTER);
				// Check for button-pointer intersections, and rumble
				if (Button_Hover(&menuSettingsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuGraphicsButton, pointer.p_x, pointer.p_y) ||
					Button_Hover(&menuLanguagesButton, pointer.p_x, pointer.p_y))
				{
					if (--self.rumbleAmt > 0) // Should we be rumbling?
					{
						if(settings.rumble)
							WPAD_Rumble(0,1); // Turn on Wiimote rumble
					}
					else 
						WPAD_Rumble(0,0); // Kill the rumble
				}
				else
				{ // If no button is being hovered, kill the rumble
					WPAD_Rumble(0,0);
					self.rumbleAmt = 4;
				}
				
				break;
			} // end case for 'about' panel
		}// end switch
		// Draw Cursor and render
		DrawCursor(0, pointer.p_x, pointer.p_y, pointer.p_ang, 1, 1, 0xFFFFFFFF);
		GRRLIB_Render();
		
	}while(doloop); // End Menu do loop
	
	//////////////////
	// Draw the outro - raise the menu
	for(i = 0; i <= 20; i++)
	{
		// Draw the covers in the back
		draw_covers();
		GRRLIB_2D_Init();
		// Draw the screen fade
		fade-=5;
		GRRLIB_FillScreen(0x00000000|fade);
		// Draw the closing background panel
		moving_y = change_scale(i, 0, 20, 0, -240);
		GRRLIB_Rectangle(70, moving_y, 500, 240, 0x44444499, true);
		moving_y = change_scale(i, 0, 20, 240, 480);
		GRRLIB_Rectangle(70, moving_y, 500, 240, 0x44444499, true);
		// Draw the menu header across the screen
		moving_y = change_scale(i, 0, 20, 0, -72);
		for (header_x=0; header_x<=80; header_x++)
			GRRLIB_DrawImg(header_x*8, moving_y, menu_header_texture, 0, 1, 1, 0xFFFFFFFF);
		// Draw the bottom menu header across the screen
		moving_y = change_scale(i, 0, 20, 408, 481);
		for (header_x=0; header_x<=80; header_x++)
			GRRLIB_DrawImg(header_x*8, moving_y, menu_header_vflip_texture, 0, 1, 1, 0xFFFFFFFF);
		// Draw the buttons
		moving_y = change_scale(i, 0, 20, 20, -52);
		menuLogoButton.y = moving_y;
		menuSettingsButton.y = moving_y;
		menuGraphicsButton.y = moving_y;
		menuLanguagesButton.y = moving_y;
		Button_Paint(&menuLogoButton);
		Button_Menu_Paint(&menuSettingsButton);
		Button_Menu_Paint(&menuGraphicsButton);
		Button_Menu_Paint(&menuLanguagesButton);
		// Draw the pointer
		WPAD_ScanPads();
		GetWiimoteData();
		DrawCursor(0, pointer.p_x, pointer.p_y, pointer.p_ang, 1, 1, 0xFFFFFFFF);
		GRRLIB_Render();
	}
	menuSettingsButton.selected  = true;
	menuGraphicsButton.selected  = false;
	menuLanguagesButton.selected = false;
	
} // Settings_Menu_Show()

//////////////////////////////////
// Shows the Game Settings Menu //
//////////////////////////////////
void Game_Settings_Menu_Show()
{
    //get/set per-game settings
    struct discHdr *header = NULL;
    header = &self.gameList[self.gameSelected];
    char titleID[7];
    char gameName[MAX_TITLE_LEN];
	
    // chomp the title to fit
	if(self.usingTitlesTxt)
	{
		sprintf(gameName, "%s", header->title);
		getTitle(titleList, (char*)header->id, gameName);
	}
	else
		sprintf(gameName, "%s", (header->title));
	
    if(strlen(header->title) >= 30)
    {
		//strncpy(gameName, header->title, 17);
		gameName[27] = '\0';
		strncat(gameName, "...", 3);
    }
	
    sprintf(titleID, "%s", header->id);
    if(!getGameSettings(titleID, &gameSetting));
    {
        setGameSettings(titleID, &gameSetting,-1);
        getGameSettings(titleID, &gameSetting);
    }
    // ocarina will be -1 if we never been into game settings before
    if(gameSetting.ocarina == -1)
    {
        gameSetting.ocarina = 0;
        gameSetting.hooktype = 0;
        gameSetting.language = 0;
        gameSetting.video = 0;
        gameSetting.vipatch = 0;
    }
	
    bool doloop = true;
	//////////////
    // Window loop
	do{
		WPAD_ScanPads();
		GetWiimoteData();

		// Handle button events
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_1)
			GRRLIB_ScrShot(USBLOADER_PATH "/sshot.png");
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
		{
			setGameSettings(titleID, &gameSetting,-1);
			return;
		}
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
		{
			setGameSettings(titleID, &gameSetting,-1);
			return;
		}
		if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
		{
			if(Button_Select(&gbackButton, pointer.p_x, pointer.p_y))
			{
				setGameSettings(titleID, &gameSetting,-1);
				return;
			}
			else if (Button_Select(&gcheatonButton, pointer.p_x, pointer.p_y) || Button_Select(&gcheatoffButton, pointer.p_x, pointer.p_y))
			{
				gameSetting.ocarina = (gameSetting.ocarina) ? 0 : 1; // Clicked the Ocarina button, toggle state
			}
			else if (Button_Select(&gvidtvonButton, pointer.p_x, pointer.p_y) || Button_Select(&gvidtvoffButton, pointer.p_x, pointer.p_y))
			{
				gameSetting.vipatch = (gameSetting.vipatch) ? 0 : 1; // Clicked the VIPATCH button, toggle state
			}
			else if (Button_Select(&glangdownButton, pointer.p_x, pointer.p_y))
			{ // Clicked on the language buttons
				if (gameSetting.language > 0)
				{
					gameSetting.language --;
				}
				else
				{
					gameSetting.language = (CFG_LANG_COUNT - 1);
				}
			}
			else if (Button_Select(&glangupButton, pointer.p_x, pointer.p_y))
			{
				if (gameSetting.language < (CFG_LANG_COUNT - 1))
				{
					gameSetting.language ++;
				}
				else
				{
					gameSetting.language = 0;
				}
			}
			else if (Button_Select(&ghookdownButton, pointer.p_x, pointer.p_y))
			{ // Clicked on the hooktype buttons
				if (gameSetting.hooktype > 0)
				{
					gameSetting.hooktype --;
				}
				else
				{
					gameSetting.hooktype = (CFG_HOOK_COUNT - 1);
				}
			}
			else if (Button_Select(&ghookupButton, pointer.p_x, pointer.p_y))
			{
				if (gameSetting.hooktype < (CFG_HOOK_COUNT - 1))
				{
					gameSetting.hooktype ++;
				}
				else
				{
					gameSetting.hooktype = 0;
				}
			}
			else if (Button_Select(&gviddownButton, pointer.p_x,pointer.p_y))
			{
				// Clicked on the video down button
				if (gameSetting.video > 0)
				{
					gameSetting.video --;
				}
				else
				{
					gameSetting.video = (CFG_VIDEO_COUNT -1);
				}
			}
			else if (Button_Select(&gvidupButton, pointer.p_x,pointer.p_y))
			{
				// Clicked on the video up button
				if (gameSetting.video <(CFG_VIDEO_COUNT -1))
				{
					gameSetting.video ++;
				}
				else
				{
					gameSetting.video = 0;
				}
			}
		}
		
		
		// Draw the covers behind the dialog
		draw_covers();
		// Draw the dialog panel
		GRRLIB_Rectangle(40, 106, 560, 276, 0xffffffdd, true);
		GRRLIB_Rectangle(42, 108, 556, 272, 0x737373FF, true);
		// Draw the game cover
		if(self.gameSelected < MAX_BUFFERED_COVERS || self.gameSelected >= 0)
		{
			if(BUFFER_IsCoverReady(self.gameSelected))
			{
				pthread_mutex_lock(&buffer_mutex[self.gameSelected]);
				if(_texture_data[self.gameSelected].data)
				{
					if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
					{
						if(settings.covers3d)
						{
							GRRLIB_DrawFlatCoverImg(60, 131, _texture_data[self.gameSelected], 0, AR_16_9, 1, 0xFFFFFFFF);
						}
						else
						{
							GRRLIB_DrawImg(60, 131, _texture_data[self.gameSelected], 0, AR_16_9, 1, 0xFFFFFFFF);
						}
					}
					else
					{
						if(settings.covers3d)
						{
							GRRLIB_DrawFlatCoverImg(60, 131, _texture_data[self.gameSelected], 0, 1, 1, 0xFFFFFFFF);
						}
						else
						{
							GRRLIB_DrawImg(60, 131, _texture_data[self.gameSelected], 0, 1, 1, 0xFFFFFFFF);
						}
					}
				}
				else
				{
					if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
					{
						if(settings.covers3d)
						{
							GRRLIB_DrawFlatCoverImg(60, 131, cover_texture_3d, 0, AR_16_9, 1, 0xFFFFFFFF);
						}
						else
						{
							GRRLIB_DrawImg(60, 131, cover_texture, 0, AR_16_9, 1, 0xFFFFFFFF);
						}
					}
					else
					{
						if(settings.covers3d)
						{
							GRRLIB_DrawFlatCoverImg(60, 131, cover_texture_3d, 0, 1, 1, 0xFFFFFFFF);
						}
						else
						{
							GRRLIB_DrawImg(60, 131, cover_texture, 0, 1, 1, 0xFFFFFFFF);
						}
					}
				}
				
				pthread_mutex_unlock(&buffer_mutex[self.gameSelected]);
			}
			else
			{	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
			{
				if(settings.covers3d)
				{
					GRRLIB_DrawFlatCoverImg(60, 131, cover_texture_3d, 0, 1, AR_16_9, 0xFFFFFFFF);
				}
				else
				{
					GRRLIB_DrawImg(60, 131, cover_texture, 0, 1, AR_16_9, 0xFFFFFFFF);
				}
			}
			else
			{
				if(settings.covers3d)
				{
					GRRLIB_DrawFlatCoverImg(60, 131, cover_texture_3d, 0, 1, 1, 0xFFFFFFFF);
				}
				else
				{
					GRRLIB_DrawImg(60, 131, cover_texture, 0, 1, 1, 0xFFFFFFFF);
				}
			}
			}
		}
		else
		{
			if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
			{
				if(settings.covers3d)
				{
					GRRLIB_DrawFlatCoverImg(60, 131, cover_texture_3d, 0, AR_16_9, 1, 0xFFFFFFFF);
				}
				else
				{
					GRRLIB_DrawImg(60, 131, cover_texture, 0, AR_16_9, 1, 0xFFFFFFFF);
				}
			}
			else
			{
				if(settings.covers3d)
				{
					GRRLIB_DrawFlatCoverImg(60, 131, cover_texture_3d, 0, 1, 1, 0xFFFFFFFF);
				}
				else
				{
					GRRLIB_DrawImg(60, 131, cover_texture, 0, 1, 1, 0xFFFFFFFF);
				}
			}
		}
		// Draw the attributes labels
		CFreeTypeGX_DrawText(ttf20pt, 420, 140, TX.gameSettings, (GXColor){0x00, 0x00, 0x00F, 0xff}, FTGX_JUSTIFY_CENTER);
		CFreeTypeGX_DrawText(ttf16pt, 355, 183, TX.patchVIDTV, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
		CFreeTypeGX_DrawText(ttf16pt, 355, 219, TX.ocarina, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
		CFreeTypeGX_DrawText(ttf16pt, 355, 251, TX.hook, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
		CFreeTypeGX_DrawText(ttf16pt, 500, 251, ghooks[gameSetting.hooktype], (GXColor){0xff, 0xff, 0xff, 0xff}, FTGX_JUSTIFY_CENTER);
		CFreeTypeGX_DrawText(ttf16pt, 355, 285, TX.language, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
		CFreeTypeGX_DrawText(ttf16pt, 500, 285, languages[gameSetting.language], (GXColor){0xff, 0xff, 0xff, 0xff}, FTGX_JUSTIFY_CENTER);
		CFreeTypeGX_DrawText(ttf16pt, 355, 319, TX.videoMode, (GXColor){0x00, 0x00, 0x00, 0xff}, FTGX_JUSTIFY_RIGHT);
		CFreeTypeGX_DrawText(ttf16pt, 500, 319, vidmodes[gameSetting.video], (GXColor){0xff, 0xff, 0xff, 0xff}, FTGX_JUSTIFY_CENTER);

		// Draw the buttons
		Button_TTF_Paint(&gbackButton);
		Button_Paint(&glangupButton);
		Button_Paint(&glangdownButton);
		Button_Paint(&gvidupButton);
		Button_Paint(&gviddownButton);
		Button_Paint(&ghookupButton);
		Button_Paint(&ghookdownButton);
		Button_TTF_Toggle_Paint(&gcheatoffButton, &gcheatonButton, TX.toggleOffB, TX.toggleOnB, gameSetting.ocarina);
		Button_TTF_Toggle_Paint(&gvidtvoffButton, &gvidtvonButton, TX.toggleOffB, TX.toggleOnB, gameSetting.vipatch);
		
		
		// Draw the default pointer hand
		
		// Check for button-pointer intersections, and rumble
		if ((Button_Hover(&gbackButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&glangupButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&glangdownButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&gvidupButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&gviddownButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&gcheatoffButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&gcheatonButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&gvidtvoffButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&gvidtvonButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&ghookupButton, pointer.p_x, pointer.p_y) ||
			 Button_Hover(&ghookdownButton, pointer.p_x, pointer.p_y)))
		{
			// Should we be rumbling?
			if (--self.rumbleAmt > 0)
			{
				if(settings.rumble)
					WPAD_Rumble(0,1); // Turn on Wiimote rumble
			}
			else 
				WPAD_Rumble(0,0); // Kill the rumble
		}
		else
		{ // If no button is being hovered, kill the rumble
			WPAD_Rumble(0,0);
			self.rumbleAmt = 5;
		}
		// Draw Cursor and render
		DrawCursor(0, pointer.p_x, pointer.p_y, pointer.p_ang, 1, 1, 0xFFFFFFFF);
		GRRLIB_Render();
		
	}while(doloop);
	
    return;
}

