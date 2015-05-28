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

// Для установки хука на мышь
static HHOOK handle;

// Прототип WndProc решил в .h не включать
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
// И вот это тоже
int StartupDialog();

// Глобальные переменные, которые могут потребоваться везде
//TCHAR		BKBAppName=L"Клавиатура и мышь для управления глазами : сборка D / Keyboard & Mouse control with the eyes : Release D";
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
		if(BKBTET::Init()) tracking_device=2; // пробуем с TheEyeTribe (запоминаем, что гасить в конце)
		else break; // всё прошло успешно
		
	case 2: // просто мышь... ничего здесь не делаем
		// Запускаем эмуляцию работы устройства Tobii аэромышью
		// Это сделаем в WM_CREATE ToolBar'a
		break;
	}

	// Кисти-фонты, screen_scale
	BKBgdiInit();
	// Звуки
	BKBClick::Init();

	// Создаем окна
	//BKBMagnifyWnd::Init(); // Увеличительное
	HWND master_hwnd=BKBToolWnd::Init(); // Инструменты с правой стороны
	BKBMagnifyWnd::Init(master_hwnd); // Увеличительное
	BKBKeybWnd::Init(master_hwnd); // Клавиатура
	BKBTranspWnd::Init(master_hwnd); // Прозрачное окно
	BKBProgressWnd::Init(master_hwnd); // Окно прогресса

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
	}

	Internat::Unload();

	return 0;
}