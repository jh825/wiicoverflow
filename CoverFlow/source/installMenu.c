#include "installMenu.h"

extern s_self self;
extern s_title* titleList;

extern int COVER_COUNT;
bool Menu_Install(){

	WPAD_Rumble(0,0);
	self.rumbleAmt = 0;
	okButton.y = 290;
	cancelButton.y = 290;
    static struct discHdr headerdisc ATTRIBUTE_ALIGN(32);
	
	if(!WindowPrompt (TX.installNewGame, TX.diskInDrive, &okButton,&cancelButton))
		return false;
		
	WindowPrompt (TX.initDVD, TX.pleaseWait,0,0);
	
	/* Disable WBFS mode */
	Disc_SetWBFS(0, NULL);
	
    int ret, choice = 0;
	char name[MAX_TITLE_LEN];
	static char buffer[MAX_CHARACTERS + 4];

	ret = Disc_Wait();
	if (ret < 0) {
		WindowPrompt (TX.errorReadDisc,0,&cancelButton,0);
		return false;
	}
	ret = Disc_Open();
	if (ret < 0) {
		WindowPrompt (TX.errorOpenDisc,0,&cancelButton,0);
		return false;
	}

	ret = Disc_IsWii();
	
	if (ret < 0) {
		choice = WindowPrompt (TX.notWiiDisc, TX.insertWiiDisc,0,&cancelButton);
/*
		
		if (!choice) {
			return false;
		}
		else
		{
			//don't like
			return Menu_Install();
		}
		*/
		return false; //
	}
	
	Disc_ReadHeader(&headerdisc);
	sprintf(name, headerdisc.title);

	if(self.usingTitlesTxt){
		char id[7];
		sprintf(id, "%s",headerdisc.id);
		getTitle(titleList, id, name);
	}
	
	
	//WindowPrompt("SUCCA", headerdisc.title, 0, &cancelButton);
	
	/*
	if (strlen(name) < (22 + 3)) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(name, "%s", name);
		} else {
			strncpy(buffer, name,  MAX_CHARACTERS);
			strncat(buffer, "...", 3);
			sprintf(name, "%s", buffer);
	}
	*/
	
	ret = WBFS_CheckGame(headerdisc.id);
	if (ret) {
		WindowPrompt (TX.alreadyInstalled, name, &cancelButton,0);
		return false;
	}
	self.hdd = GetHddInfo();
	if (!self.hdd) {
		WindowPrompt (TX.noHDD, TX.error, &cancelButton,0);
		return false;
		}

	f32 freespace, used;

	WBFS_DiskSpace(&used, &freespace);
	u32 estimation = wbfs_estimate_disc(self.hdd, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION);
	f32 gamesize = ((f32) estimation)/1073741824;
	char gametxt[50];
	
	sprintf(gametxt, TX.installingGame, gamesize);
	
	char ttext[50];
	char tsize[50];
	sprintf(ttext, "Installing game: %s", name); // FIX M074
	sprintf(tsize, TX.gameSize, gamesize);
	
	if(WindowPrompt (ttext,tsize,&okButton,&cancelButton))
	{
		if (gamesize > freespace) {
			char errortxt[50];
			sprintf(errortxt, TX.gameSizeSpace, gamesize, freespace);
			WindowPrompt(TX.noFreeSpace, errortxt, &cancelButton, 0);
			return false;
		}
		else {
			ret = ProgressWindow(self.hdd, gametxt, name);
			if (ret != 0) {
				WindowPrompt (TX.errorInstall,0,&cancelButton,0);
				return false;
			} else {
				Sleep(100);
				GetEntries();
				Sleep(300);
				InitializeBuffer(self.gameList,self.gameCnt,BUFFER_WINDOW,COVER_COUNT/2.0 +self.shift);
				Sleep(1000);
				
				WiiLight(1);
				WindowPrompt (TX.successInstall, name,&okButton,0);
				WiiLight(0);
				return true;
			}
		}
	}
	
	return false;
}