#include <windows.h>
#include "resource.h"

extern HINSTANCE	BKBInst;

static char *timeout_chars[5]={"00","01","02","03","04"};
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