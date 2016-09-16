#pragma once

struct CachedAssets_t
{
	void *scrollBarArrowUp;		// Material*
	void *scrollBarArrowDown;	// Material*
	void *scrollBarArrowLeft;	// Material*
	void *scrollBarArrowRight;	// Material*
	void *scrollBar;		// Material*
	void *scrollBarThumb;	// Material*
	void *sliderBar;		// Material*
	void *sliderThumb;		// Material*
	void *whiteMaterial;	// Material*
	void *cursor;			// Material*
	void *dvarlistArrowLeft;	// Material*
	void *dvarlistArrowRight;	// Material*
	void *line;				// Material*
	void *difficulty[4];	// Material*
	void *bigFont;		// Font_s*
	void *smallFont;	// Font_s*
	void *consoleFont;	// Font_s*
	void *boldFont;		// Font_s*
	void *textFont;		// Font_s*
	void *extraBigFont;	// Font_s*
	void *itemFocusSound; // snd_alias_list_t*
};

struct __declspec(align(2)) mapInfo
{
	char mapName[32];
	char mapLoadName[24];
	char mapNameCaps[32];
	char imageName[42];
	char gap_82[142];
	void *levelShot; // Material*
	int active;
};

struct __declspec(align(2)) sharedUiInfo_t
{
	CachedAssets_t assets;
	char gap_60[6052];
	int mapCount;
	char gap_1808[4];
	mapInfo mapList[128];
	char gap_tmp[1];
};

extern sharedUiInfo_t * const sharedUiInfo;

bool __cdecl UI_LoadModArenas();

typedef int (__cdecl* UI_ParseInfos_t)(const char *buf, int max, char **infos);
static UI_ParseInfos_t UI_ParseInfos = (UI_ParseInfos_t)0x005EF550;

typedef const char *__cdecl UI_SafeTranslateString_t(const char *reference);
static UI_SafeTranslateString_t* UI_SafeTranslateString = (UI_SafeTranslateString_t*)0x006664D0;

typedef void __cdecl Live_UpdateUiPopup_t(int unk, const char *popupname);
static Live_UpdateUiPopup_t* Live_UpdateUiPopup = (Live_UpdateUiPopup_t*)0x005C8FE0;

typedef void (__cdecl* UI_SetLoadingScreenMaterial_t)(const char *name);
static UI_SetLoadingScreenMaterial_t UI_SetLoadingScreenMaterial = (UI_SetLoadingScreenMaterial_t)0x005C6F70;

//
// Functionally the same as UI_SelectedMap except it returns mapLoadName
//
const char* __cdecl UI_SelectedMap_LoadName(int index, int* actual);

void mfh_UI_FeederSelection_AllMaps();