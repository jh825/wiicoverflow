#include "coverflow.h"

//static char prozent[MAX_CHARACTERS + 16];
static char timet[MAX_CHARACTERS + 16];

// Language selection config
char languages[11][22] =
{{"Console Default"},
{"   Japanese"},
{"    English"},
{"    German"},
{"    French"},
{"    Spanish"},
{"    Italian"},
{"     Dutch"},
{"   S. Chinese"},
{"   T. Chinese"},
{"    Korean"}};
//video mode text
char vidmodes[6][10] =
{{ " game " },
	{ " auto ", },
	{ " pal50", },
	{ " pal60", },
	{ " ntsc ", },
        { "system"}};

/* Gamelist buffer */
static struct discHdr *gameList = NULL;

static wbfs_t *hdd = NULL;

/* WBFS device */
static s32 my_wbfsDev = WBFS_DEVICE_USB;

float progress = 0.0;

s_self self;
s_pointer pointer;

#ifdef TEST_MODE
int COVER_COUNT = 29;
#else
int COVER_COUNT = 0;
#endif


float SCROLL_SPEED = 0.050;

bool firstTimeDownload = true;
bool inetOk = false;
bool imageNotFound = false;

char* _title;
char* _msg;

/*--------------------------------------*/
#include "settings.h"


Mtx GXmodelView2D;

//int self.array_size = 0;


void Download_Cover(struct discHdr *header, int v, int max);
void batchDownloadCover();
int WindowPrompt(char* title, char* txt, struct Button* choice_a, struct Button* choice_b);

int buffer_window_min = 0;
int buffer_window_max = 0;
int oldmin = 0;
int oldmax = 0;

void UpdateBufferedImages()
{
	int i;
	int index = 0;
	
	for(i = (-1*(COVER_COUNT/2.0)); i < (COVER_COUNT/2.0); i++)
	{
		index = i+(COVER_COUNT/2.0);
		if(index < self.gameCnt)
		{
			/*Some logic to avoid drawing everything*/
			if(abs(self.shift+i) <= BUFFER_WINDOW)
			{
				//Is this cover already loaded?
				if(!BUFFER_IsCoverReady(index))
				{
					//Is this cover already queued up?
					if(!BUFFER_IsCoverQueued(index))
					{
						//Request this cover
						struct discHdr *header = &gameList[index];
		
						BUFFER_RequestCover(index, header);
					}
				}
			}
			else
			{
				if(BUFFER_IsCoverReady(index))
				{
					//TODO Fix this
					//calling this fixes the fuked up graphics when you load
					//too many covers, but it causes a code dump eventually... HELP
					//BUFFER_RemoveCover(index);
				}
			}
		}
	}
	
}

void quit()
{
	//we should free all allocated textures (SCO);
	GRRLIB_Exit();
	exit(0);
}

void LoadCurrentCover(int id)
{

	#ifndef TEST_MODE
	void *imgData;// = (void *)no_cover_png;

	char filepath[128];
	s32  ret;

	struct discHdr *header = &gameList[id];
	
//		substr
	
	//sprintf(filepath, USBLOADER_PATH "/disks/%c%c%c.png", header->id[0],header->id[1],header->id[2]);
	sprintf(filepath, USBLOADER_PATH "/disks/%c%c%c%c.png", header->id[0],header->id[1],header->id[2],header->id[3]);

	ret = Fat_ReadFile(filepath, &imgData);
	
	
	if (ret > 0) {
		current_cover_texture = GRRLIB_LoadTexture((const unsigned char*)imgData);
	}
	else
	{
		sprintf(filepath, USBLOADER_PATH "/disks/%c%c%c.png", header->id[0],header->id[1],header->id[2]);
		ret = Fat_ReadFile(filepath, &imgData);

		if (ret > 0) {
			current_cover_texture = GRRLIB_LoadTexture((const unsigned char*)imgData);
		}
		else
			current_cover_texture = no_disc_texture;
	}
	
	#else
	current_cover_texture = no_disc_texture;
	#endif
	
}

void AddCover(GRRLIB_texImg tex)
{
	if(self.array_size < MAX_COVERS)
	{
		covers[self.array_size] = tex;
		self.array_size++;
	}
}

void ClearCovers()
{
	BUFFER_ClearCovers();
}

void Init_Covers()
{
	#ifdef TEST_MODE
	progress+=0.05;
	Paint_Progress(progress, NULL);
	
	int i;
	int CoverCount = COVER_COUNT;
	float max_progress = 1.7;
	float per_game_prog = max_progress/self.gameCnt;
	
	for(i = 0; i < CoverCount; i++)
	{
		AddCover( GRRLIB_LoadTexture(no_cover_png) );
		progress+=per_game_prog;
		Paint_Progress(progress, NULL);
	}
	
	#endif
}

s32 __Menu_EntryCmp(const void *a, const void *b)
{
	struct discHdr *hdr1 = (struct discHdr *)a;
	struct discHdr *hdr2 = (struct discHdr *)b;

	/* Compare strings */
	return strcmp(hdr1->title, hdr2->title);
}

s32 GetEntries(void)
{
	struct discHdr *buffer = NULL;

	u32 cnt, len;
	s32 ret;

	/* Get list length */
	ret = WBFS_GetCount(&cnt);
	if (ret < 0)
		return ret;

	/* Buffer length */
	len = sizeof(struct discHdr) * cnt;

	/* Allocate memory */
	buffer = (struct discHdr *)memalign(32, len);
	if (!buffer)
		return -1;

	/* Clear buffer */
	memset(buffer, 0, len);

	progress+=0.05;
	Paint_Progress(progress, NULL);
	
	/* Get header list */
	ret = WBFS_GetHeaders(buffer, cnt, sizeof(struct discHdr));
	if (ret < 0)
		goto err;

	progress+=0.05;
	Paint_Progress(progress, NULL);
	
	/* Sort entries */
	qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);

	progress+=0.05;
	Paint_Progress(progress, NULL);
	
	/* Free memory */
	if (gameList)
		free(gameList);

	/* Set values */
	gameList = buffer;
	self.gameCnt  = cnt;
	COVER_COUNT = self.gameCnt;
	
	Init_Covers();

	progress+=0.05;
	Paint_Progress(progress, NULL);
	
	/* Reset variables */
	self.gameSelected = self.gameStart = 0;

	return 0;

err:
	/* Free memory */
	if (buffer)
		free(buffer);

	return ret;
}

bool init_usbfs()
{    
   // __Disc_SetLowMem();

	s32 ret;

	
	/* Initialize system */
	Sys_Init();

	///* Initialize subsystems */
	Wpad_Init();

	/* Mount SDHC */
	Fat_MountSDHC();
	
	progress+=0.05;
	Paint_Progress(progress, NULL);
	

	/* Initialize DIP module */
	ret = Disc_Init();
	progress+=0.05;
	Paint_Progress(progress, NULL);
	
	if (ret < 0) {
		printf("[+] ERROR:\n");
		printf("    Could not initialize DIP module! (ret = %d)\n", ret);

		return false;
	}

	return true;
}

bool Init_Game_List(void)
{

	Paint_Progress(progress, NULL);
	
	/* Try to open device */
	if (WBFS_Open() >= 0) {
		/* Get game list */
		
		progress+=0.05;
		Paint_Progress(progress, NULL);
		GetEntries();
		return true;
	}
	else
	{
		return false;
	}
}



void DragSlider(int xPos)
{

	int min_loc = 126;
	int max_loc = 439;
		
	self.shift = change_scale(xPos, min_loc, max_loc, -1*(COVER_COUNT/2.0), COVER_COUNT/2.0);
}

int DiscWait()
{
    u32 cover = 0;
	s32 ret = 0;

	while(!(cover & 0x2))
	{
		//TODO Add GUI For Cancel Button
		
		ret = WDVD_GetCoverStatus(&cover);
		if (ret < 0)
			return ret;
	}

	return ret;
}

/****************************************************************************
 * ShowProgress
 *
 * Updates the variables used by the progress window for drawing a progress
 * bar. Also resumes the progress window thread if it is suspended.
 ***************************************************************************/
void
ShowProgress (s32 done, s32 total)
{

    static time_t start;
	static u32 expected;

    f32 percent; //, size;
	u32 d, h, m, s;

	//first time
	if (!done) {
		start    = time(0);
		expected = 300;
	}

	//Elapsed time
	d = time(0) - start;

	if (done != total) {
		//Expected time
		if (d)
			expected = (expected * 3 + d * total / done) / 4;

		//Remaining time
		d = (expected > d) ? (expected - d) : 0;
	}

	//Calculate time values
	h =  d / 3600;
	m = (d / 60) % 60;
	s =  d % 60;

	//Calculate percentage/size
	percent = (done * 100.0) / total;

	//sprintf(prozent, "%s%0.2f%%", "Installing Game...", percent);

    sprintf(timet,"Installing Game (%0.2f%%) Time left: %d:%02d:%02d",percent, h,m,s);

	/*Update and Draw Progress Window Here*/
	//WindowPrompt(prozent, timet, 0, 0);
	
	Paint_Progress_Generic(done, total, timet); 
}


int ProgressWindow(char* title, char* msg)
{
	/*TODO Draw Window*/
	_title = title;
	_msg   = msg;
	
	int ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, ShowProgress, ONLY_GAME_PARTITION, 0);
	
	progress = 0.0;
	
	return ret;

}

void Graphic_Settings_Menu(void)
{
	bool doloop = true;
	//bool dummy = false;

	/*Render and control Settings*/
	do{

		WPAD_ScanPads();
		
		ir_t ir; // The struct for infrared
		
		WPAD_IR(WPAD_CHAN_0, &ir); // Let's get our infrared data
		wd = WPAD_Data(WPAD_CHAN_0);

		pointer.p_x = ir.sx-200;
		pointer.p_y = ir.sy-250;
		pointer.p_ang = ir.angle/2; // Set angle/2 to translate correctly

		Hover_Buttons();

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
		{
			doloop = false;
		}
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
		{
			doloop = false;
		}
		
		if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
		{
			if(Button_Select(&settingsButton, pointer.p_x, pointer.p_y))
			{
				doloop = false;
			}
			else if(Button_Select(&resetButton, pointer.p_x, pointer.p_y))
			{
				SETTINGS_Init();
			}
			else if(Button_Select(&windowdownButton, pointer.p_x, pointer.p_y))
			{
				if(SETTING_drawWindow > 1)
				{
					SETTING_drawWindow -= 1;
				}
			}
			else if(Button_Select(&windowupButton, pointer.p_x, pointer.p_y))
			{
				if(SETTING_drawWindow < 100) // Allow for user to shoot self in foot
				{
					SETTING_drawWindow += 1;
				}
			}
			else if(Button_Select(&coverTextOnButton, pointer.p_x, pointer.p_y) || Button_Select(&coverTextOffButton, pointer.p_x, pointer.p_y))
			{
				SETTING_coverText = (SETTING_coverText) ? 0 : 1;
			}
		}
		
		if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_A)
		{
			if(Button_Select(&spacingdownButton, pointer.p_x, pointer.p_y))
			{
				SETTING_coverSpacing -= 0.05;
			}
			else if(Button_Select(&spacingupButton, pointer.p_x, pointer.p_y))
			{
				SETTING_coverSpacing += 0.05;
			}
			else if(Button_Select(&angledownButton, pointer.p_x, pointer.p_y))
			{
				SETTING_coverAngle -= 1;
			}
			else if(Button_Select(&angleupButton, pointer.p_x, pointer.p_y))
			{
				SETTING_coverAngle += 1;
			}
			else if(Button_Select(&zoomdownButton, pointer.p_x, pointer.p_y))
			{
				SETTING_coverZoom -= 0.01;
			}
			else if(Button_Select(&zoomupButton, pointer.p_x, pointer.p_y))
			{
				SETTING_coverZoom += 0.01;
			}
		}
		
		Hover_Buttons();
		
		//GRRLIB_FillScreen(0x000000FF);
		//GRRLIB_DrawImg(0, 0,    gradient_texture, 0, 1, 1, 0xFFFFFFFF);
		
		/*Draw Covers*/ //PREVIEW
		draw_covers();
		
		
		GRRLIB_DrawImg(120, 60, menu_bg_texture, 0, 1, 1, 0xFFFFFFFF);
		
        GRRLIB_Printf(190, 63, tex_BMfont5, 0xFFFFFFFF, 1, "Coverflow Settings (GFX)");
		
		Button_Paint(&settingsButton);
		Button_Paint(&spacingupButton);
		Button_Paint(&spacingdownButton);
	
		Button_Paint(&zoomupButton);
		Button_Paint(&zoomdownButton);
	
		Button_Paint(&angleupButton);
		Button_Paint(&angledownButton);
	
		Button_Paint(&windowupButton);
		Button_Paint(&windowdownButton);
	
		if (SETTING_coverText)
		{
			Button_Paint(&coverTextOnButton);
		}
		else Button_Paint(&coverTextOffButton);
		
		Button_Paint(&resetButton);
	
		GRRLIB_Printf(145, 95, tex_BMfont5, 0xFFFFFFFF, 1, "Zoom:");
		GRRLIB_Printf(330, 95, tex_BMfont5, 0xFFFFFFFF, 1, "%f", SETTING_coverZoom);

		GRRLIB_Printf(145, 143, tex_BMfont5, 0xFFFFFFFF, 1, "Spacing:");
		GRRLIB_Printf(330, 143, tex_BMfont5, 0xFFFFFFFF, 1, "%f", SETTING_coverSpacing);
		
		GRRLIB_Printf(145, 191, tex_BMfont5, 0xFFFFFFFF, 1, "Angle:");
		GRRLIB_Printf(330, 191, tex_BMfont5, 0xFFFFFFFF, 1, "%f", SETTING_coverAngle);
		
		GRRLIB_Printf(145, 239, tex_BMfont5, 0xFFFFFFFF, 1, "Draw Window:");
		GRRLIB_Printf(330, 239, tex_BMfont5, 0xFFFFFFFF, 1, "%d", SETTING_drawWindow);
		
		GRRLIB_Printf(145, 287, tex_BMfont5, 0xFFFFFFFF, 1, "Game Title:");
		
		/*Draw Menu*/
		GRRLIB_DrawImg(pointer.p_x, pointer.p_y, pointer_texture, pointer.p_ang, 1, 1, 0xFFFFFFFF);
		
		GRRLIB_Render();
		
	}while(doloop);
}

void Settings_Menu(void)
{
	bool doloop = true;

	/*Render and control Settings*/
	do{

		WPAD_ScanPads();
		
		ir_t ir; // The struct for infrared
		
		WPAD_IR(WPAD_CHAN_0, &ir); // Let's get our infrared data
		wd = WPAD_Data(WPAD_CHAN_0);

		pointer.p_x = ir.sx-200;
		pointer.p_y = ir.sy-250;
		pointer.p_ang = ir.angle/2; // Set angle/2 to translate correctly

		Hover_Buttons();

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
		{
			doloop = false;
		}
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
		{
			doloop = false;
		}
		
		if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
		{
			if(Button_Select(&settingsButton, pointer.p_x, pointer.p_y))
			{
				doloop = false;
			}
			else if(Button_Select(&cheatonButton, pointer.p_x, pointer.p_y) || Button_Select(&cheatoffButton, pointer.p_x, pointer.p_y))
			{
				CFG.ocarina = (CFG.ocarina) ? 0 : 1;
			}
			else if(Button_Select(&vidtvonButton, pointer.p_x, pointer.p_y) || Button_Select(&vidtvoffButton, pointer.p_x, pointer.p_y))
			{
				CFG.vipatch = (CFG.vipatch) ? 0 : 1;
			}
			else if(Button_Select(&graphicsButton, pointer.p_x, pointer.p_y))
			{
				Graphic_Settings_Menu();
			}
			else if(Button_Select(&langdownButton, pointer.p_x, pointer.p_y))
			{
				if(CFG.language > 0)
				{
					CFG.language --;
				}
				else
				{
					CFG.language = (CFG_LANG_COUNT -1);
				}
			}
			else if(Button_Select(&langupButton, pointer.p_x, pointer.p_y))
			{
				if(CFG.language <(CFG_LANG_COUNT -1))
				{
					CFG.language ++;
				}
				else
				{
                                        CFG.language = 0;
				}
			}
			else if(Button_Select(&downloadButton, pointer.p_x, pointer.p_y)){
				if(WindowPrompt("Cover download","This operation can't be canceled, continue?", &okButton, &cancelButton)){
					batchDownloadCover();
					//Init_Covers();
				}
			}
                        else if(Button_Select(&viddownButton, pointer.p_x,pointer.p_y))
                        {
                            if(CFG.video > 0)
				{
					CFG.video --;
				}
				else
				{
					CFG.video = (CFG_VIDEO_COUNT -1);
				}
                        }
                        else if(Button_Select(&vidupButton, pointer.p_x,pointer.p_y))
                        {
                            if(CFG.video <(CFG_VIDEO_COUNT -1))
				{
					CFG.video ++;
				}
				else
				{
					CFG.video = 0;
				}
                        }

			else if(Button_Select(&themeWhiteButton, pointer.p_x, pointer.p_y) || Button_Select(&themeBlackButton, pointer.p_x, pointer.p_y))
			{
				CFG.themeblack = (CFG.themeblack) ? 0 : 1;
			}
		}
		
		Hover_Buttons();
		
		GRRLIB_FillScreen(0x000000FF);
		GRRLIB_DrawImg(120, 60, menu_bg_texture, 0, 1, 1, 0xFFFFFFFF);
		
                GRRLIB_Printf(190, 63, tex_BMfont5, 0xFFFFFFFF, 1, "Coverflow Settings");
		
		Button_Paint(&settingsButton);
		Button_Paint(&langupButton);
		Button_Paint(&langdownButton);
		Button_Paint(&vidupButton);
		Button_Paint(&viddownButton);

		Button_Paint(&graphicsButton);
	
		GRRLIB_Printf(145, 93, tex_BMfont5, 0xFFFFFFFF, 1, "Ocarina:");

		GRRLIB_Printf(145, 128, tex_BMfont5, 0xFFFFFFFF, 1, "Language:");
		GRRLIB_Printf(330, 128, tex_BMfont5, 0xFFFFFFFF, 1, "%s",languages[CFG.language]);
		GRRLIB_Printf(145, 223, tex_BMfont5, 0xFFFFFFFF, 1, "Graphics:");
		GRRLIB_Printf(145, 253, tex_BMfont5, 0xFFFFFFFF, 1, "Download missing covers");
                GRRLIB_Printf(145, 180, tex_BMfont5, 0xFFFFFFFF, 1, "VIDTV patch:");
                GRRLIB_Printf(145, 155, tex_BMfont5, 0xFFFFFFFF, 1, "Video mode:");
                GRRLIB_Printf(365, 155, tex_BMfont5, 0xFFFFFFFF, 1, "%s",vidmodes[CFG.video]);
/* Waiting on issue #31 fix
		GRRLIB_Printf(145, 303, tex_BMfont5, 0xFFFFFFFF, 1, "Theme");
*/
		
		/*Draw Menu*/
		if (CFG.ocarina)
		{
			Button_Paint(&cheatonButton);
		}
		else Button_Paint(&cheatoffButton);

		if (CFG.vipatch)
		{
			Button_Paint(&vidtvonButton);
		}
		else Button_Paint(&vidtvoffButton);

/* Waiting on issue #31 fix
		if (CFG.themeblack)
		{
			Button_Paint(&themeBlackButton);
		}
		else Button_Paint(&themeWhiteButton);
*/
		
		Button_Paint(&graphicsButton);
		Button_Paint(&downloadButton);
		
		GRRLIB_DrawImg(pointer.p_x, pointer.p_y, pointer_texture, pointer.p_ang, 1, 1, 0xFFFFFFFF);
		
		GRRLIB_Render();
		
	}while(doloop);
}


bool Menu_Install(void)
{

	if(!WindowPrompt ("Install new Game?", "Place disk in drive and hit ok.",&okButton,&cancelButton))
		return false;
	
    static struct discHdr headerdisc ATTRIBUTE_ALIGN(32);
	
	WindowPrompt ("Initializing DVD Drive", "Please Wait...",0,0);
	
	/* Disable WBFS mode */
	Disc_SetWBFS(0, NULL);
	
    int ret, choice = 0;
	char *name;
	static char buffer[MAX_CHARACTERS + 4];

	ret = Disc_Wait();
	if (ret < 0) {
		WindowPrompt ("Error reading Disc",0,&cancelButton,0);
		return false;
	}
	ret = Disc_Open();
	if (ret < 0) {
		WindowPrompt ("Could not open Disc",0,&cancelButton,0);
		return false;
	}

	ret = Disc_IsWii();
	
	if (ret < 0) {
		choice = WindowPrompt ("Not a Wii Disc","Insert a Wii Disc!",&okButton,&cancelButton);

		if (!choice) {
			return false;
		}
		else
		{
			return Menu_Install();
		}
	}
	
	Disc_ReadHeader(&headerdisc);
	name = headerdisc.title;
	if (strlen(name) < (22 + 3)) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(name, "%s", name);
		} else {
			strncpy(buffer, name,  MAX_CHARACTERS);
			strncat(buffer, "...", 3);
			sprintf(name, "%s", buffer);
	}

	ret = WBFS_CheckGame(headerdisc.id);
	if (ret) {
		WindowPrompt ("Game is already installed:",name,&cancelButton,0);
		return false;
	}
	hdd = GetHddInfo();
	if (!hdd) {
		WindowPrompt ("No HDD found!","Error!!",&cancelButton,0);
		return false;
		}

	f32 freespace, used;

	WBFS_DiskSpace(&used, &freespace);
	u32 estimation = wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION);
	f32 gamesize = ((f32) estimation)/1073741824;
	char gametxt[50];
	
	sprintf(gametxt, "Installing game %.2fGB:", gamesize);
	
	char ttext[50];
	char tsize[50];
	sprintf(ttext, "Install %s?", name);
	sprintf(tsize, "Game Size: %.2fGB", gamesize);
	
	
	if(WindowPrompt (ttext,tsize,&okButton,&cancelButton))
	{
		if (gamesize > freespace) {
			char errortxt[50];
			sprintf(errortxt, "Game Size: %.2fGB, Free Space: %.2fGB", gamesize, freespace);
			WindowPrompt("Not enough free space!",errortxt,&cancelButton, 0);
			return false;
		}
		else {
			ret = ProgressWindow(gametxt, name);
			if (ret != 0) {
				WindowPrompt ("Install error!",0,&cancelButton,0);
				return false;
			} else {
				BUFFER_ClearCovers();
				Sleep(300);
				GetEntries();
				Sleep(300);
				UpdateBufferedImages();
				Sleep(100);
				
				WindowPrompt ("Successfully installed:",name,&okButton,0);
				return true;
			}
		}
	}
	
	return false;
}

void AddGame(void)
{
	Menu_Install();
}

bool Menu_Delete(void)
{
	struct discHdr *header = NULL;
 	char gameName[31]; 
	
	/* No game list */
	if (!self.gameCnt)
		return false;

	/* Selected game */
	header = &gameList[self.gameSelected];

	if(strlen(get_title(header)) < 30) {
		sprintf(gameName, "%s", get_title(header));
	}
	else
	{
		strncpy(gameName, get_title(header), 27);
		gameName[27] = '\0';
		strncat(gameName, "...", 3);
	}

	if(WindowPrompt("Do you want to delete:", gameName, &yesButton, &noButton))
	{
		if(0 > WBFS_RemoveGame(header->id))
		{
			WindowPrompt("Delete Failed.", "Could not delete game.", &okButton, 0);
		}
		else
		{
			BUFFER_ClearCovers();
			Sleep(300);
			GetEntries();
			Sleep(300);
			UpdateBufferedImages();
			Sleep(100);
			
			WindowPrompt("Successfully deleted.", "Press Ok to Continue.", &okButton, 0);
			return true;
		}
	}
	
	return false;
}
		

bool Menu_Boot(void)
{
	#ifndef TEST_MODE
	struct discHdr *header = NULL;
	//int i = 0;
	s32 ret;

	/* No game list */
	if (!self.gameCnt)
		return false;

	/* Selected game */
	header = &gameList[self.gameSelected];

    GRRLIB_Exit();
	
	free(cover_texture.data);
	free(back_texture.data);
	free(empty_texture.data);
	free(no_disc_texture.data);
	free(current_cover_texture.data);
	free(text_font1.data);

	BUFFER_ClearCovers();
	BUFFER_KillBuffer();
	Sleep(300);
	
	/* Set WBFS mode */
	Disc_SetWBFS(WBFS_DEVICE_USB,header->id);
		
	/* Open disc */
	ret = Disc_Open();
	if (ret < 0) {
		return false;
	}

	ret = Disc_WiiBoot();
    if (ret < 0) {
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

	#endif
	
	return true;
}


int Net_Init(char *ip){
	
	s32 res;
	
    while ((res = net_init()) == -EAGAIN)
		usleep(100 * 1000); //100ms
	
    if (if_config(ip, NULL, NULL, true) < 0) {
		WindowPrompt ("ERROR!","Cannot get local IP address.", &okButton, 0);
		usleep(1000 * 1000 * 1); //1 sec
		return false;
	}
	return true;
}

void saveFile(char* imgPath, struct block file){
			
	/* save png to sd card for future use*/
			
	FILE *f;
	f = fopen(imgPath, "wb");
	if (f)
	{
		fwrite(file.data,1,file.size,f);
		fclose (f);
	}
}


void Download_Cover(struct discHdr *header, int v, int max)
{
	char imgPath[100];

	if (!header)
		return;

	if(firstTimeDownload == true){
		
		char myIP[16];
		
		sprintf(self.debugMsg, "Initializing Network");
		Paint_Progress_Generic(v, max, self.debugMsg);
		
		if(!Net_Init(myIP))
		{
			sprintf(self.debugMsg, "Error Initializing Network");
			Paint_Progress_Generic(v, max, self.debugMsg);
			WindowPrompt ("ERROR!","Error initializing network", &okButton, 0);
		}
		else
		{
			sprintf(self.debugMsg, "Network Initialized");
			Paint_Progress_Generic(v, max, self.debugMsg);
			inetOk = true;
		}

		firstTimeDownload = false;
	}

	if(inetOk) {
		//printf("\n    Network connection established.");
		/*try to download image */
			
		char url[100];
		struct block file;
	
		char region[4];
		switch(header->id[3]){
	
		case 'E':
			sprintf(region,"ntsc");
			break;

		case 'J':
			sprintf(region,"ntscj");
			break;

		case 'P':
			sprintf(region,"pal");
			break;
		}

		snprintf(imgPath, sizeof(imgPath), "%s/covers/%s.png", CFG.images_path, header->id);
		
		sprintf(self.debugMsg, "Checking presence of %s", imgPath);
		Paint_Progress_Generic(v, max,self.debugMsg);
		
		FILE *fp;
		fp = fopen(imgPath, "rb");
		if (fp)
		{
			fclose (fp);
			sprintf(self.debugMsg, "%s present, not downloading", imgPath);
			Paint_Progress_Generic(v, max,self.debugMsg);
		}
		else{
			
			/*
			if (CFG.widescreen)
				sprintf(url, "http://www.theotherzone.com/wii/widescreen/%s/%s.png", region, header->id);
			else
				sprintf(url, "http://www.theotherzone.com/wii/%s/%s.png", region, header->id);
			*/
			
			sprintf(url, "http://www.theotherzone.com/wii/resize/%s/160/224/%s.png", region, header->id);
			sprintf(self.debugMsg, "Getting %s", url);
			Paint_Progress_Generic(v, max,self.debugMsg);
		
			file = downloadfile(url);
			
			if(file.data != NULL){
				saveFile(imgPath, file);
				free(file.data);
				sprintf(self.debugMsg, "done");
			    Paint_Progress_Generic(v, max,self.debugMsg);
				//else
					//donotdownload = true;
			}
			else {
				sprintf(self.debugMsg, "some error occurred");
				Paint_Progress_Generic(v, max,self.debugMsg);
			}
		}
		
		snprintf(imgPath, sizeof(imgPath), "%s/disks/%c%c%c%c.png", CFG.images_path,  header->id[0], header->id[1], header->id[2], header->id[3]);
		sprintf(self.debugMsg, "Checking presence of %s", imgPath);
		Paint_Progress_Generic(v, max,self.debugMsg);
		
		fp = fopen(imgPath, "rb");
		if (fp)
		{
			fclose (fp);
			sprintf(self.debugMsg, "%s present, not downloading", imgPath);
			Paint_Progress_Generic(v, max,self.debugMsg);
		}
		else{
			sprintf(url, "http://www.theotherzone.com/wii/diskart/160/160/%c%c%c%c.png", header->id[0], header->id[1], header->id[2], header->id[3]);
			sprintf(self.debugMsg, "Getting %s", url);
			Paint_Progress_Generic(v, max,self.debugMsg);
			
			file = downloadfile(url);
			
			if(file.data != NULL){
				saveFile(imgPath, file);
				free(file.data);
				sprintf(self.debugMsg, "done");
			    Paint_Progress_Generic(v, max,self.debugMsg);
				//else
					//donotdownload = true;
			}
			else { //TRY WITH 3 DIGIT COVER
				
				snprintf(imgPath, sizeof(imgPath), "%s/disks/%c%c%c.png", CFG.images_path,  header->id[0], header->id[1], header->id[2]);
				sprintf(url, "http://www.theotherzone.com/wii/diskart/160/160/%c%c%c.png", header->id[0], header->id[1], header->id[2]);
				sprintf(self.debugMsg, "Getting %s", url);
				Paint_Progress_Generic(v, max,self.debugMsg);
				
				file = downloadfile(url);
			
				if(file.data != NULL){
					saveFile(imgPath, file);
					free(file.data);
					sprintf(self.debugMsg, "done");
					Paint_Progress_Generic(v, max,self.debugMsg);
				}
				else {
					sprintf(self.debugMsg, "some error occurred");
					Paint_Progress_Generic(v, max,self.debugMsg);
				}
			}
		//else
			//donotdownload = true;
		}
	}
	//refresh = true;				
} /* end download */


void batchDownloadCover(){

	int i;
	
	for(i = 0; i < self.gameCnt; i++)
	{
		struct discHdr *header = &gameList[i];
		
		if(self.array_size < MAX_COVERS)
		{
			sprintf(self.debugMsg, "Checking next cover...%s", header->id);
			Paint_Progress_Generic(i, self.gameCnt, self.debugMsg);
			Download_Cover(header, i, self.gameCnt);
			//sprintf(filepath, USBLOADER_PATH "/covers/%s.png", header->id);
		}
	}
	BUFFER_ClearCovers();
	Sleep(300);
	UpdateBufferedImages();
	Sleep(100);
	WindowPrompt ("Operation finished!","Press A to continue", &okButton, 0);
}

void initVars(){

	pointer.p_x = 0;
	pointer.p_y = 0;
	pointer.p_ang = 0;
	
	self.shift = 0;
	self.select_shift = 0;
	self.gameCnt = 0;
	self.gameSelected = 0;
	self.gameStart = 0;
	self.selected = false;
	self.animate_flip = 0.0;
	self.animate_rotate = 0.0;
	self.array_size = 0;
}

void checkDirs(){
	
	int result = 0;
	
	DIR_ITER* dir = diropen(USBLOADER_PATH);
	if (dir == NULL) {
		
		mkdir(USBLOADER_PATH, S_ISVTX);
		//int result = chdir("SD:/usb-loader/");
		result = chdir(USBLOADER_PATH);
		
		if(result == 0){
			//WindowPrompt("Cover download","result = 0", &okButton, NULL);
			mkdir("disks", S_ISVTX);
			//WindowPrompt("Cover download","result = 0", &okButton, NULL);
			mkdir("covers", S_ISVTX);
		}
		else{
			WindowPrompt("ERROR!","Can't create directories. Covers will not be saved.", &okButton, NULL);
		}
	}
	else{
	
	//	WindowPrompt("Cover download","Closing dir", &okButton, NULL);
		dirclose(dir);
		
		result = chdir(USBLOADER_PATH);
		
		if(result == 0){
			dir = diropen("disks");
	//		WindowPrompt("Cover download",USBLOADER_PATH "/disks/", &okButton, NULL);
			if(dir == NULL) {
				mkdir("disks", S_ISVTX);
			}
			else{
				dirclose(dir);
			}
			
			dir = diropen("covers");
	//		WindowPrompt("Cover download",USBLOADER_PATH "/disks/", &okButton, NULL);	
			if(dir == NULL) {
				mkdir("covers", S_ISVTX);
			}
			else{
				dirclose(dir);
			}
		}
		
		else{
			WindowPrompt("ERROR!","Can't create directories. Covers will not be saved.", &okButton, NULL);
		}
	}
}

bool LaunchGame()
{
	bool done = false;
	while(!done)
	{
		draw_covers();
		
		done = draw_selected_two(gameList, true, false);
		
		GRRLIB_Render();
	}
	
	/*Fade to black*/
	//TODO Fade to black instead of just drawing black
	GRRLIB_FillScreen(0x000000FF);
	GRRLIB_Render();
	
	GRRLIB_FillScreen(0x000000FF);
	GRRLIB_Render();
	
	Menu_Boot();
	
	return false;
}

//---------------------------------------------------------------------------------
int main( int argc, char **argv ){
//---------------------------------------------------------------------------------
	#ifndef TEST_MODE
	
	/* Load Custom IOS */
	int ret = IOS_ReloadIOS(249);
	/* Check if Custom IOS is loaded */
	if (ret < 0) {
		printf("[+] ERROR:\n");
		printf("    Custom IOS could not be loaded! (ret = %d)\n", ret);

		return 0;
	}
	#endif
	
	SETTINGS_Init();
	
	GRRLIB_Init();
    GRRLIB_FillScreen(0x000000FF);
    GRRLIB_Render();
    GRRLIB_FillScreen(0x000000FF);
    GRRLIB_Render();
	
	initVars();
	
    gradient_texture    = GRRLIB_LoadTexture(gradient_bg_png);
    loader_main_texture = GRRLIB_LoadTexture(loading_main_png);
    progress_texture    = GRRLIB_LoadTexture(progress_png);
	
	sprintf(self.debugMsg, "Loading textures");
	Paint_Progress(progress,self.debugMsg);
	
	LoadTextures();
	
	Init_Buttons(); //Button loaded first so they can be used for error msgs
	
	progress += .1;
	sprintf(self.debugMsg, "Init USB");
	Paint_Progress(progress,self.debugMsg);
	
	#ifndef TEST_MODE
	if(!init_usbfs()){
		WindowPrompt ("ERROR!","Cannot init USBFS, quitting.", &okButton, 0);
		return 0;
	}
		
	//LOAD CONFIG
	strcpy(CFG.images_path, USBLOADER_PATH);
	CFG.widescreen = 0;
	CFG.download = 1;
	//CFG.theme = 0; //BLACK
	//HARDCODED FOR NOW
	
	sprintf(self.debugMsg, "Initializing WBFS");
	Paint_Progress(progress,self.debugMsg);
	
	my_wbfsDev = WBFS_DEVICE_USB;
	
	checkDirs();

  INIT_RETRY:
	/* Initialize WBFS */
	ret = WBFS_Init(my_wbfsDev);
	
	if(ret < 0)
	{
		while(1)
		{
			WPAD_ScanPads();
			GRRLIB_DrawImg(120, 60, menu_bg_texture, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_Printf(190, 100, tex_BMfont5, 0xFFFFFFFF, 1, "USB Error - Drive not found");
			GRRLIB_Printf(190, 120, tex_BMfont5, 0xFFFFFFFF, 1, "Press A to Retry, B to Exit");
			GRRLIB_Render();
				
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
			{
				goto INIT_RETRY;
			}
			
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
			{
				GRRLIB_Exit();
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
			}
		}
	}
	
	//bool flip = true;
  USB_RETRY:
	if(!Init_Game_List())
	{
		while(1)
		{
			WPAD_ScanPads();
			GRRLIB_DrawImg(120, 60, menu_bg_texture, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_Printf(190, 100, tex_BMfont5, 0xFFFFFFFF, 1, "USB Error - Drive not found");
			GRRLIB_Printf(190, 120, tex_BMfont5, 0xFFFFFFFF, 1, "Press A to Retry, B to Exit");
			GRRLIB_Render();
				
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
			{
				goto USB_RETRY;
			}
			
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
			{
				GRRLIB_Exit();
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
			}
		}
	}
	
	#else
	self.gameCnt = 29;
	Init_Covers();
	#endif
	
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	BUFFER_InitBuffer(BUFFER_THREAD_COUNT);
	UpdateBufferedImages();
	
	int wait = 300; //ms
	float prog = 2.1/300.0;
	
	sprintf(self.debugMsg, "Initializing Threaded Image Buffer...");
	while(wait > 0)
	{
		wait--;
		progress += prog;
		Paint_Progress(progress, self.debugMsg);
		Sleep(1);
	}
	
	sprintf(self.debugMsg, "Loading User Settings...");
	Paint_Progress(progress,self.debugMsg);
	
	/*Load Saved Settings*/
	SETTINGS_Load();
	
	sprintf(self.debugMsg, "Freeing unused textures...");
	Paint_Progress(progress,self.debugMsg);
	
	free(gradient_texture.data);
	free(progress_texture.data);
	
	Sleep(300);
	
	self.selected = false;
	
	bool select_ready = false;
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	
	#ifdef TEST_MODE
	PAD_Init();
	#endif
	
	bool dragging = false;
	
	self.animate_count = 50;
	self.animate_slide_x = 0;
	
	//self.selected = true;
	//LaunchGame();
	//Sleep(10000);
	while(1) {

		WPAD_ScanPads();
		
		#ifdef TEST_MODE
		PAD_ScanPads();
		#endif
		
		ir_t ir; // The struct for infrared
		
		WPAD_IR(WPAD_CHAN_0, &ir); // Let's get our infrared data
		wd = WPAD_Data(WPAD_CHAN_0);

		pointer.p_x = ir.sx-200;
		pointer.p_y = ir.sy-250;
		pointer.p_ang = ir.angle/2; // Set angle/2 to translate correctly

		Hover_Buttons();
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
		{
			SETTINGS_Save();
			
			GRRLIB_FillScreen(0x000000FF);
			GRRLIB_Render();
			
			//BUFFER_ClearCovers();
			BUFFER_KillBuffer();
			Sleep(500);
			
			quit();
		}
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B ||
			PAD_ButtonsDown(0) & PAD_BUTTON_B)
		{
			if(self.selected && self.animate_flip >= 1.0)
			{
				self.selected = false;
			}
		}
		
		if(dragging)
		{
			if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_A)
			{
				DragSlider(pointer.p_x);
			}
			else
			{
				dragging = false;
			}
		}
				
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A ||
			PAD_ButtonsDown(0) & PAD_BUTTON_A)
		{
			//First Check Buttons
			if(Button_Select(&addButton, pointer.p_x, pointer.p_y))
			{
				AddGame();
			}
			else if(Button_Select(&settingsButton, pointer.p_x, pointer.p_y))
			{
				Settings_Menu();
			}
			else if(Button_Select(&slideButton, pointer.p_x, pointer.p_y))
			{
				dragging = true;
			}
			else
			{
				if(self.gameCnt)
				{
					if(!self.selected && self.animate_flip <= 0.0)
					{
						if(pointer.p_x < 400 && pointer.p_x > 200 &&
							pointer.p_y > 60 && pointer.p_y < 380)
						{
							if(select_ready && self.select_shift == 0.0)
							{
								self.selected = true;
								LoadCurrentCover(self.gameSelected);
							}
						}
						else if(pointer.p_x < 200 &&
							pointer.p_y > 60 && pointer.p_y < 380)
						{
							self.select_shift = (-4)*((200-pointer.p_x)/200.0);
						}
						else if(pointer.p_x > 400 &&
							pointer.p_y > 60 && pointer.p_y < 380)
						{
							self.select_shift = 5*(pointer.p_x-345.0)/280.0;
						}
					}
					
					if(self.selected && self.animate_flip == 1.0)
					{
						if(Button_Select(&loadButton, pointer.p_x, pointer.p_y))
						{
							
							SETTINGS_Save();
							
							#ifdef ANIMATE_TEST
							if(!LaunchGame())
								return 0;
							#else
							//TODO Prompt to boot game...
							if(!Menu_Boot())
							{
								self.selected = false;
								self.animate_flip = 0;
							}
							else
							{
								return 0;
							}
							#endif
						}
						else if(Button_Select(&deleteButton, pointer.p_x, pointer.p_y))
						{
							Menu_Delete();
							self.selected = false;
						}
						else if(Button_Select(&backButton, pointer.p_x, pointer.p_y))
						{
							self.selected = false;
						}
					}
				}
			}
		}

		if(!self.selected && self.animate_flip == 0)
		{
			if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_LEFT  ||
				PAD_ButtonsDown(0) & PAD_BUTTON_LEFT)
			
			{	
				select_ready = false;
					
				if(!((int)self.shift-1 <= (-1)*(COVER_COUNT/2.0)))
					self.shift -= SCROLL_SPEED;
			}
			else if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_RIGHT ||
				PAD_ButtonsDown(0) & PAD_BUTTON_RIGHT)
			{
				select_ready = false;
					
				if(!((int)self.shift+.5 >= (COVER_COUNT/2.0)))
					self.shift += SCROLL_SPEED;
			}
			else
			{
				if(abs(self.select_shift) > SCROLL_SPEED)
				{
					int mult = abs((int)self.select_shift);
					if(self.select_shift > 0)
					{
						self.select_shift -= mult*SCROLL_SPEED;
						if(!((int)self.shift-1 <= (-1)*(COVER_COUNT/2.0)))
							self.shift -= mult*SCROLL_SPEED;
					}
					else
					{
						self.select_shift += mult*SCROLL_SPEED;
						if(!((int)self.shift+.5 >= (COVER_COUNT/2.0)))
							self.shift += mult*SCROLL_SPEED;
					}
					
				}
				else if(abs(((int)self.shift * 10000.0) - (self.shift*10000.0))/10000.0 > (SCROLL_SPEED+SCROLL_SPEED/2.0))
				{
					select_ready = false;
					if((int)((int)(self.shift+0.5) - (int)self.shift) == 0)
					{
						self.shift -= SCROLL_SPEED;
					}
					else
					{
						self.shift += SCROLL_SPEED;
					}
				}
				else
				{
					self.select_shift = 0;
					self.shift = (int)self.shift;
					select_ready = true;
				
					/*Draw Game Title*/
					if(SETTING_coverText)
						draw_game_title(self.gameSelected, gameList);
				}
					
			}
		}
		
		UpdateBufferedImages();
		
		draw_covers();

		if(self.selected || self.animate_flip != 0)
		{
			#ifndef ANIMATE_TEST
			draw_selected(gameList);
			#else
			draw_selected_two(gameList, false, Button_Hover(&loadButton, pointer.p_x, pointer.p_y));
			#endif
		}
		else
		{
			DrawSlider();
			Button_Paint(&addButton);
			Button_Paint(&settingsButton);
		}

		GRRLIB_DrawImg(pointer.p_x, pointer.p_y, pointer_texture, pointer.p_ang, 1, 1, 0xFFFFFFFF);
        GRRLIB_Render();

		Sleep(1);
	}
	
    GRRLIB_Exit(); 
	
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	
	return 0;
} 