#include <windows.h>
#include "resource.h"
#include "Internat.h"

extern HINSTANCE	BKBInst;

static TCHAR *timeout_chars[5]={L"00",L"01",L"02",L"03",L"04"};
//===================================================================
// Диалог выбора устройства
//===================================================================
static BOOL CALLBACK DlgSettingsWndProc(HWND hdwnd,
						   UINT uMsg,
						   WPARAM wparam,
						   LPARAM lparam )
{
	static int timeout_counter;

if (uMsg==WM_COMMAND)
	{
	switch (LOWORD(wparam))
		{
		case IDCANCEL: // Не случилось. [Аэро]Мышь
			KillTimer(hdwnd,2);
			EndDialog(hdwnd,2);
			return 1;

		case IDOK: 	//Хорошо! Tobii!
			KillTimer(hdwnd,2);
			EndDialog(hdwnd,0);
			return 1;

		case IDOK2: 	//Хорошо! TheEyeTribe!
			KillTimer(hdwnd,2);
			EndDialog(hdwnd,1);
			return 1;
		} // switch WM_COMMAND

	
	}// if WM_COMMAND 

	if (uMsg==WM_INITDIALOG)
	{
		SetWindowText(hdwnd,Internat::Message(8,L"Что используем?"));
		SendDlgItemMessage(hdwnd,IDC_STATIC_STARTTOBII5SEC, WM_SETTEXT, 0L, (LPARAM)Internat::Message(5,L"Автоматически попробуем запустить трекер Tobii через"));
		SendDlgItemMessage(hdwnd,IDC_STATIC_THENTTE, WM_SETTEXT, 0L, (LPARAM)Internat::Message(6,L"сек., затем TheEyeTribe,"));
		SendDlgItemMessage(hdwnd,IDC_STATIC_ALLFAIL, WM_SETTEXT, 0L, (LPARAM)Internat::Message(7,L"а уж если и это не получится, то просто мышь (аэро)."));
		
		SendDlgItemMessage(hdwnd,IDOK, WM_SETTEXT, 0L, (LPARAM)Internat::Message(9,L"Трекер Tobii REX или EyeX"));
		SendDlgItemMessage(hdwnd,IDOK2, WM_SETTEXT, 0L, (LPARAM)Internat::Message(10,L"Трекер TheEyeTribe"));
		SendDlgItemMessage(hdwnd,IDCANCEL, WM_SETTEXT, 0L, (LPARAM)Internat::Message(11,L"АэроМышь"));
		
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