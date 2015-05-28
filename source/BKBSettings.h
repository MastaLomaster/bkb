// Здесь хранятся все настройки
#ifndef __BKB_SETTINGS
#define __BKB_SETTINGS

#include <stdio.h>

typedef struct
{
	TCHAR *stroka;
	int value;
	int button;
} BKBIntChar;

class BKBSettings
{
public:
	static void SettingsDialogue(HWND parent_hwnd=0);
	static void PrepareDialogue(HWND hdwnd); // Заполняет списки
	static void ShowLoad(HWND hdwnd); // Показываем текущие значения
	static void Screen2Load(HWND hdwnd);
	static void ActualizeLoad();
	static void SaveBKBConfig();
	static int OpenBKBConfig();
	static HWND settings_hwnd;
	static HWND parent_hwnd;
};

#endif