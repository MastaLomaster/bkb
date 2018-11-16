#include <windows.h>
#include "BKBRepErr.h"
#include "Internat.h"
#include "TobiiREX.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "AirMouse.h"
#include "BKBgdi.h"
#include "TET.h"
#include "Click.h"
#include "BKBSettings.h"
#include "BKBHookProc.h"
#include "BKBProgressWnd.h"
#include "BKBMetricsWnd.h"
#include "BKBTurtle.h"
#include "GP3.h"
#include "Grid.h"
#include "Fixation.h"
#include "MyGaze.h"
#include "SerialComm.h"

extern FILE *debug_fout;
extern int transparency;
extern int gBKB_GRID_WHEELCHAIR;

// Для установки хука на мышь
static HHOOK handle;

// Прототип WndProc решил в .h не включать
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
// И вот это тоже
int StartupDialog();

// Глобальные переменные, которые могут потребоваться везде
//TCHAR		BKBAppName=L"Клавиатура и мышь для управления глазами : сборка E / Keyboard & Mouse control with the eyes : Release E";
HINSTANCE	BKBInst;
HWND		BKBhwnd;
int tracking_device;

// Имя класса окна
//static const char BKBWindowCName[]="BKB0B"; 

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE,LPSTR cline,INT)
// Командную строку не обрабатываем
{
	MSG msg; // Сообщение

	// Сразу делаем Instance доступной всем
	BKBInst=hInst;

	// Если есть другой язык - загружаем его
	Internat::LoadMessages();
	BKBKeybWnd::Load(); // Если есть клавиатура - загружаем её
	BKBSettings::OpenBKBConfig(); // Пробуем прочитать параметры
	BKBSettings::ActualizeLoad(); // Прочитанные параметры считаем верными и пускаем их в бой

	// Что будем использовать?
	tracking_device=StartupDialog();

	switch(tracking_device)
	{
	case 0: // Tobii
		// Инициализируем работу с Tobii REX
		// Если произошла ошибка, переходим на работу с [аэро]мышью
		if(BKBTobiiREX::Init()) tracking_device=1; // пробуем с TheEyeTribe (запоминаем, что гасить в конце)
		else break; // всё прошло успешно

	case 1: // TheEyeTribe
		if(BKBTET::Init()) tracking_device=3; // пробуем с GP3 (запоминаем, что гасить в конце)
		else break; // всё прошло успешно
		
	case 3: // Gazepoint GP3
		if(BKBGP3::Init()) tracking_device=2; // сваливаемся на мышь (запоминаем, что гасить в конце)
		//if(BKBGP3::Init()) tracking_device=4; // сваливаемся на myGaze (запоминаем, что гасить в конце)
		else break; // всё прошло успешно
/*
	case 4: // myGaze
		if(BKBMyGaze::Init()) tracking_device=2; // сваливаемся на мышь (запоминаем, что гасить в конце)
		else break; // всё прошло успешно
*/
	case 2: // просто мышь... ничего здесь не делаем
		// Запускаем эмуляцию работы устройства Tobii аэромышью
		// Это сделаем в WM_CREATE ToolBar'a
		break;
	}

	// При необходимости меняем режим на GRID
	if(1==gBKB_GRID_WHEELCHAIR)
	{
		Fixation::SetGridMode();
		transparency=100;
	}
	else if(2==gBKB_GRID_WHEELCHAIR)
	{
		Fixation::SetWheelChairMode();
	}

	// Кисти-фонты, screen_scale
	BKBgdiInit();
	// Звуки
	BKBClick::Init();
	// Grid
	BKBGrid::Load();

	// Создаем окна
	//BKBMagnifyWnd::Init(); // Увеличительное
	HWND master_hwnd=BKBToolWnd::Init(); // Инструменты с правой стороны
	BKBMagnifyWnd::Init(master_hwnd); // Увеличительное
	BKBKeybWnd::Init(master_hwnd); // Клавиатура
	BKBTranspWnd::Init(master_hwnd); // Прозрачное окно
	BKBProgressWnd::Init(master_hwnd); // Окно прогресса
	BKBMetricsWnd::Init(master_hwnd); // Окно метрик
	BKBTurtle::Init(master_hwnd); // Окна черепашки
	BKBTurtle::Place();
	//BKBTurtle::Show(SW_SHOW);


	// Инициализируем работу хука
	handle = SetWindowsHookEx(WH_MOUSE_LL, 
									HookProc2, 
                                 GetModuleHandle(NULL), 
                                 NULL);

	//Цикл обработки сообщений
	while(GetMessage(&msg,NULL,0,0)) 
    {
		TranslateMessage( &msg );
        DispatchMessage( &msg );
	}// while !WM_QUIT

	// Чистим за собой
	UnhookWindowsHookEx(handle);

	// Звуки
	BKBClick::Halt();
	// Кисти-фонты
	BKBgdiHalt();

	switch(tracking_device)
	{
	case 0:
		BKBTobiiREX::Halt();
		break;

	case 1:
		BKBTET::Halt();
		break;

	case 3:
		BKBGP3::Halt();
		break;

	case 4:
		BKBMyGaze::Halt();
		break;
	}

	Internat::Unload();

#ifdef _DEBUG
	if(debug_fout)
	{
		fflush(debug_fout);
		fclose(debug_fout);
	}
#endif

	BKBSerial::Halt(); // На всякий случай

	return 0;
}