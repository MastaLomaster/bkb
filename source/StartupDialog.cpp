#include <windows.h>
#include "resource.h"
#include "Internat.h"
#include "BKBSettings.h"
#include "Grid.h"

extern HINSTANCE	BKBInst;

static TCHAR *timeout_chars[5]={L"00",L"01",L"02",L"03",L"04"};



// Усиление мыши на выбор, определены в BKBSettings.cpp (к сожалению, не работает extern const)
const int BKB_MOUSE_MULTIPLIERS=7;
extern BKBIntChar dlg_mouse_x_multiplier[BKB_MOUSE_MULTIPLIERS],dlg_mouse_y_multiplier[BKB_MOUSE_MULTIPLIERS], dlg_grid_wheelchair[3]; 
extern int dlg_current_mouse_x_multiplier, dlg_current_mouse_y_multiplier, dlg_current_grid_wheelchair;

extern int g_BKB_MOUSE_X_MULTIPLIER, g_BKB_MOUSE_Y_MULTIPLIER; // Определены в AirMouse.cpp
extern int gBKB_GRID_WHEELCHAIR;
//==================================================================
// Крохотная функция считывания параметров
//==================================================================
static void GetSettings(HWND hdwnd)
{
	dlg_current_mouse_x_multiplier=SendDlgItemMessage(hdwnd,IDC_COMBO_X_MULTIPLIER, CB_GETCURSEL, 0, 0L);
	g_BKB_MOUSE_X_MULTIPLIER=dlg_mouse_x_multiplier[dlg_current_mouse_x_multiplier].value;
	dlg_current_mouse_y_multiplier=SendDlgItemMessage(hdwnd,IDC_COMBO_Y_MULTIPLIER, CB_GETCURSEL, 0, 0L);
	g_BKB_MOUSE_Y_MULTIPLIER=dlg_mouse_y_multiplier[dlg_current_mouse_y_multiplier].value;	

	dlg_current_grid_wheelchair=SendDlgItemMessage(hdwnd,IDC_COMBO_GRID_WHEELCHAIR, CB_GETCURSEL, 0, 0L);
	gBKB_GRID_WHEELCHAIR=dlg_grid_wheelchair[dlg_current_grid_wheelchair].value;	
}

//===================================================================
// Диалог выбора устройства
//===================================================================
static BOOL CALLBACK DlgSettingsWndProc(HWND hdwnd,
						   UINT uMsg,
						   WPARAM wparam,
						   LPARAM lparam )
{
	static int timeout_counter;
	static bool f_settings_changed=false; // Если поменялись настройки grid, нужно сохранить в файле конфигурации

if (uMsg==WM_COMMAND)
	{
	switch (LOWORD(wparam))
		{
		case IDCANCEL: // Не случилось. [Аэро]Мышь
			KillTimer(hdwnd,2);
			GetSettings(hdwnd);
			if(f_settings_changed) BKBSettings::SaveBKBConfig();
			EndDialog(hdwnd,2);
			return 1;

		case IDOK: 	//Хорошо! Tobii!
			KillTimer(hdwnd,2);
			GetSettings(hdwnd);
			if(f_settings_changed) BKBSettings::SaveBKBConfig();
			EndDialog(hdwnd,0);
			return 1;

		case IDOK2: 	//Хорошо! TheEyeTribe!
			KillTimer(hdwnd,2);
			GetSettings(hdwnd);
			if(f_settings_changed) BKBSettings::SaveBKBConfig();
			EndDialog(hdwnd,1);
			return 1;

		case IDOK3: 	//Хорошо! Gazepoint GP3!
			KillTimer(hdwnd,2);
			GetSettings(hdwnd);
			if(f_settings_changed) BKBSettings::SaveBKBConfig();
			EndDialog(hdwnd,3);
			return 1;

		case IDOK4: 	//Хорошо! myGaze!
			KillTimer(hdwnd,2);
			GetSettings(hdwnd);
			if(f_settings_changed) BKBSettings::SaveBKBConfig();
			EndDialog(hdwnd,4);
			return 1;

		case IDC_COMBO_X_MULTIPLIER:
		case IDC_COMBO_Y_MULTIPLIER:
			SendDlgItemMessage(hdwnd,IDC_TIMEOUT, WM_SETTEXT, 0L, (LPARAM)Internat::Message(45,L"(ждём)"));
			KillTimer(hdwnd,2); // Если тронули настройки - таймер останавливается
			break;

		case IDC_BUTTON_SETTINGS:
			KillTimer(hdwnd,2); // Если тронули настройки - таймер останавливается
			GetSettings(hdwnd); // поправим настройки
			BKBSettings::SettingsDialogue(hdwnd);
			break;
			
		case IDC_COMBO_GRID_WHEELCHAIR:
			SendDlgItemMessage(hdwnd,IDC_TIMEOUT, WM_SETTEXT, 0L, (LPARAM)Internat::Message(45,L"(ждём)"));
			KillTimer(hdwnd,2); // Если тронули настройки - таймер останавливается
			f_settings_changed=true;
			break;

		} // switch WM_COMMAND

	
	}// if WM_COMMAND 

	if (uMsg==WM_INITDIALOG)
	{
		int i;

		SetWindowText(hdwnd,Internat::Message(8,L"Что используем?"));

		SendDlgItemMessage(hdwnd,IDC_STATIC_STARTTOBII5SEC, WM_SETTEXT, 0L, (LPARAM)Internat::Message(5,L"Автоматически попробуем запустить трекер Tobii через"));
		SendDlgItemMessage(hdwnd,IDC_STATIC_THENTTE, WM_SETTEXT, 0L, (LPARAM)Internat::Message(6,L"сек., затем TheEyeTribe, затем Gazepoint GP3, затем myGaze,"));
		SendDlgItemMessage(hdwnd,IDC_STATIC_ALLFAIL, WM_SETTEXT, 0L, (LPARAM)Internat::Message(7,L"а уж если и это не получится, то просто мышь (аэро)."));
		
		SendDlgItemMessage(hdwnd,IDOK, WM_SETTEXT, 0L, (LPARAM)Internat::Message(9,L"Трекер Tobii Eye Tracker 4C или EyeX или REX"));
		SendDlgItemMessage(hdwnd,IDOK2, WM_SETTEXT, 0L, (LPARAM)Internat::Message(10,L"Трекер TheEyeTribe"));
		SendDlgItemMessage(hdwnd,IDCANCEL, WM_SETTEXT, 0L, (LPARAM)Internat::Message(11,L"АэроМышь"));
		
		// Перевод текста про усилители мыши
		if(Internat::Message(39,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_MOVEHARD, WM_SETTEXT, 0L, (LPARAM)Internat::Message(39,0)); // Усилитель мыши для тех, кому трудно сильно поворачивать голову с аэромышью
		if(Internat::Message(40,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_X_MULTIPLIER, WM_SETTEXT, 0L, (LPARAM)Internat::Message(40,0)); // Усиление по горизонтали:
		if(Internat::Message(41,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_Y_MULTIPLIER, WM_SETTEXT, 0L, (LPARAM)Internat::Message(41,0)); // Усиление по вертикали:
		if(Internat::Message(42,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_NOTES1, WM_SETTEXT, 0L, (LPARAM)Internat::Message(42,0)); // * Диалог настроек также вызывается
		if(Internat::Message(43,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_NOTES2, WM_SETTEXT, 0L, (LPARAM)Internat::Message(43,0)); // комбинацией клавиш Fn + TAB на клавиатуре программы
		if(Internat::Message(62,0)) SendDlgItemMessage(hdwnd,IDC_BUTTON_SETTINGS, WM_SETTEXT, 0L, (LPARAM)Internat::Message(62,0)); // Все настройки
		if(Internat::Message(86,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_OPERATION_MODE, WM_SETTEXT, 0L, (LPARAM)Internat::Message(86,0)); // Режим работы
		
		
		// Строка в списке возможных значений
		if(Internat::Message(44,0)) 
		{
			dlg_mouse_x_multiplier[0].stroka=Internat::Message(44,0); // не усиливать
			dlg_mouse_y_multiplier[0].stroka=Internat::Message(44,0); // не усиливать
		}
		
		// Заполняем списки усилителей мыши
		for(i=0;i<BKB_MOUSE_MULTIPLIERS;i++)
		{
			SendDlgItemMessage(hdwnd,IDC_COMBO_X_MULTIPLIER, CB_ADDSTRING, 0, (LPARAM)(dlg_mouse_x_multiplier[i].stroka));
			SendDlgItemMessage(hdwnd,IDC_COMBO_Y_MULTIPLIER, CB_ADDSTRING, 0, (LPARAM)(dlg_mouse_y_multiplier[i].stroka));
		}
		// считанные из файла значения
		SendDlgItemMessage(hdwnd,IDC_COMBO_X_MULTIPLIER, CB_SETCURSEL, dlg_current_mouse_x_multiplier, 0L);
		SendDlgItemMessage(hdwnd,IDC_COMBO_Y_MULTIPLIER, CB_SETCURSEL, dlg_current_mouse_y_multiplier, 0L);

		// Список режимов GRID/WHEELCHAIR
		SendDlgItemMessage(hdwnd,IDC_COMBO_GRID_WHEELCHAIR, CB_ADDSTRING, 0,  (LPARAM)Internat::Message(84,L"Обычный"));
		SendDlgItemMessage(hdwnd,IDC_COMBO_GRID_WHEELCHAIR, CB_ADDSTRING, 0,  (LPARAM)Internat::Message(80,L"Режим таблицы (GRID) - для детей"));
		SendDlgItemMessage(hdwnd,IDC_COMBO_GRID_WHEELCHAIR, CB_ADDSTRING, 0,  (LPARAM)Internat::Message(85,L"Управление моторизованной коляской через порт COM9 (посылает символы L,F,R,S,B)"));

		// считанные из файла значения
		SendDlgItemMessage(hdwnd,IDC_COMBO_GRID_WHEELCHAIR, CB_SETCURSEL, dlg_current_grid_wheelchair, 0L);


		SetWindowPos(hdwnd,NULL,50,50,0,0,SWP_NOSIZE);
		SetTimer(hdwnd,2,1000,0);
		timeout_counter=5;

	}

	if (uMsg==WM_TIMER)
	{
		timeout_counter--;
		SendDlgItemMessage(hdwnd,IDC_TIMEOUT, WM_SETTEXT, 0L, (LPARAM)(timeout_chars[timeout_counter]));
		if(0>=timeout_counter)
		{
			KillTimer(hdwnd,2);
			GetSettings(hdwnd);
			EndDialog(hdwnd,0);
			return 1;
		}
	}

return 0;
}

int StartupDialog()
{
	return DialogBox(BKBInst,MAKEINTRESOURCE(IDD_DIALOG1),NULL,(DLGPROC)DlgSettingsWndProc);
}