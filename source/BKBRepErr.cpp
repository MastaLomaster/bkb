/*
*	Сообщает об ошибках времени исполнения
*/

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include "BKBRepErr.h"
#include "resource.h"
#include "Internat.h"

extern HINSTANCE	BKBInst;

typedef TOBIIGAZE_API const char* (TOBIIGAZE_CALL *type_tobiigaze_get_error_message)(tobiigaze_error_code error_code);
extern type_tobiigaze_get_error_message fp_tobiigaze_get_error_message;

//============================================================================================
// Свой message box с таймаутом
//============================================================================================
static TCHAR *header;
static TCHAR *body;

static TCHAR *timeout_chars[5]={L"00",L"01",L"02",L"03",L"04"};


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
		case IDOK:
		case IDCANCEL:
			KillTimer(hdwnd,3);
			EndDialog(hdwnd,0);
			return 1;
		} // switch WM_COMMAND
	}// if WM_COMMAND 

	if (uMsg==WM_INITDIALOG)
	{
		SetWindowPos(hdwnd,NULL,100,100,0,0,SWP_NOSIZE);
		SetWindowText(hdwnd,header);
		SendDlgItemMessage(hdwnd,IDC_BODY, WM_SETTEXT, 0L, (LPARAM)body);
		SendDlgItemMessage(hdwnd,IDC_STATIC_WILLCLOSE, WM_SETTEXT, 0L, (LPARAM)Internat::Message(3,L"Закроется через"));
		SendDlgItemMessage(hdwnd,IDC_STATIC_SEC, WM_SETTEXT, 0L, (LPARAM)Internat::Message(4,L"сек."));
		SetTimer(hdwnd,3,1000,0);
		timeout_counter=5;
	}

	if (uMsg==WM_TIMER)
	{
		timeout_counter--;
		SendDlgItemMessage(hdwnd,IDC_TIMEOUT, WM_SETTEXT, 0L, (LPARAM)(timeout_chars[timeout_counter]));
		if(0>=timeout_counter)
		{
			KillTimer(hdwnd,3);
			EndDialog(hdwnd,0);
			return 1;
		}
	}
return 0;
}

int BKBMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	header=(TCHAR *)lpCaption;
	body=(TCHAR *)lpText;
	return DialogBox(BKBInst,MAKEINTRESOURCE(IDD_DIALOG_MB),hWnd,(DLGPROC)DlgSettingsWndProc);
}

//============================================================================================
// Сообщает о СИСТЕМНЫХ ошибках времени исполнения
//============================================================================================
void BKBReportError(TCHAR *SourceFile, TCHAR *FuncName, int LineNumber)
{
	DWORD res;				// Результат функции FormatMessage
	void *BKBStringError;	// Указатель на строку для получения системной ошибки
	TCHAR BKBMessage[2048];	// Это строка, в которой формируется сообщение об ошибке
	DWORD BKBLastError=GetLastError(); // Получили код системной ошибки
	
	
	if (BKBLastError!=(DWORD)0) // Получить строку, если код не равен нулю
	{
		res=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL, BKBLastError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &BKBStringError, 0, NULL );

		if(res==(DWORD)0) BKBStringError=(void *)Internat::Message(1,L"Сообщение об ошибке не найдено");
	}
	else
	{
		BKBStringError=(void *)Internat::Message(2,L"Нет системной ошибки");
	}
	
	// Сформировать строку с полным описанием ошибки
	swprintf_s(BKBMessage, _countof(BKBMessage),
			L"Module: %s\r\nFunction: %s\r\nLine number: %d\r\nSysErr: %d (%s)",
			SourceFile, FuncName, LineNumber,
			BKBLastError, (TCHAR *)BKBStringError);


	//Освобождаем память, которую выделила функция FormatMessage
	if (BKBLastError!=(DWORD)0)
	{
			LocalFree( BKBStringError );
	}

	//Печатаем сообщение об ошибке (если возможно, на экран)
	BKBMessageBox(NULL,BKBMessage,Internat::Message(0,L"BKB: сообщение об ошибке"),MB_OK|MB_ICONINFORMATION );

	//А также в файл 
	FILE *fout;
	fopen_s(&fout,"reperr.log","ab");

	time_t mytime = time(0); /* not 'long' */
	TCHAR ctbuf[1024];
	_wctime_s(ctbuf,1023,&mytime);
	fwprintf(fout,L"****\r\n%s\r\n%s\r\n", ctbuf, BKBMessage);
	fflush(fout);
	fclose(fout);
}

//============================================================================================
// Для НЕсистемных ошибок (перегружена)
// Реально не видел, где бы она использовалась
//============================================================================================
void BKBReportError(TCHAR *Error) 
{
	//Печатаем сообщение об ошибке (если возможно, на экран)
	BKBMessageBox(NULL,Error,Internat::Message(23,L"Непорядок!"),MB_OK|MB_ICONINFORMATION );
}


//============================================================================================
// Для ошибок Tobii Gaze SDK (перегружена)
//============================================================================================
void BKBReportError(tobiigaze_error_code tbg_error_code, TCHAR *SourceFile, TCHAR *FuncName, int LineNumber)
{
	TCHAR BKBMessage[1024];	// Это строка, в которой формируется сообщение об ошибке
	TCHAR ConvertASCII2W[1024];

	if (tbg_error_code)
    {
		const TCHAR *tmp_char;
		if(fp_tobiigaze_get_error_message)
		{
			MultiByteToWideChar(CP_ACP, 0, (*fp_tobiigaze_get_error_message)(tbg_error_code), -1, ConvertASCII2W, 1023);
			tmp_char=ConvertASCII2W;
		}
		else tmp_char=Internat::Message(24,L"неизвестно");

		// Сформировать строку с полным описанием ошибки
		swprintf_s(BKBMessage, _countof(BKBMessage),
			Internat::Message(25,L"Module: %s\nFunction: %s\nLine number: %d\nОшибка Tobii Gaze SDK: %d (%s)"),
			SourceFile, FuncName, LineNumber,
			tbg_error_code, 
			tmp_char);


		//Печатаем сообщение об ошибке (если возможно, на экран)
		BKBMessageBox(NULL,BKBMessage,Internat::Message(26,L"BKB:Gaze SDK: сообщение об ошибке"),MB_OK|MB_ICONINFORMATION );

		//А также в файл 
		FILE *fout;
		fopen_s(&fout,"reperr.log","ab");

		time_t mytime = time(0); /* not 'long' */
		TCHAR ctbuf[1024];
		_wctime_s(ctbuf,1023,&mytime);
		
		fwprintf(fout,L"****\n%s%s\n", ctbuf, BKBMessage);
		fflush(fout);
		fclose(fout);
	}
}
