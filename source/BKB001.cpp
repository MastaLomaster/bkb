#include <windows.h>
#include "BKBRepErr.h"
#include "TobiiREX.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "AirMouse.h"
#include "BKBgdi.h"

// Прототип WndProc решил в .h не включать
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
// И вот это тоже
int StartupDialog();

// Глобальные переменные, которые могут потребоваться везде
char*		BKBAppName="Клавиатура и мышь для управления глазами : сборка B";
HINSTANCE	BKBInst;
HWND		BKBhwnd;
bool flag_using_airmouse;

// Имя класса окна
static const char BKBWindowCName[]="BKB0B"; 

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE,LPSTR cline,INT)
// Командную строку не обрабатываем
{
	ATOM aresult; // Для всяких кодов возврата
	BOOL boolresult;
	MSG msg; // Сообщение

	// Сразу делаем Instance доступной всем
	BKBInst=hInst;

	// Что будем использовать?
	flag_using_airmouse=StartupDialog();

	if(flag_using_airmouse)
	{
		// Запускаем эмуляцию работы устройства Tobii аэромышью
		// Это сделаем в WM_CREATE ToolBar'a
	}
	else
	{
		// Инициализируем работу с Tobii REX
		// Если произошла ошибка, переходим на работу с [аэро]мышью
		flag_using_airmouse=BKBTobiiREX::Init();
	}

	// Кисти-фонты
	BKBgdiInit();

	// Создаем окна
	BKBMagnifyWnd::Init(); // Увеличительное
	BKBToolWnd::Init(); // Инструменты с правой стороны
	BKBKeybWnd::Init(); // Клавиатура
	BKBTranspWnd::Init(); // Прозрачное окно

	//Цикл обработки сообщений
	while(GetMessage(&msg,NULL,0,0)) 
    {
		TranslateMessage( &msg );
        DispatchMessage( &msg );
	}// while !WM_QUIT

	// Чистим за собой

	// Кисти-фонты
	BKBgdiHalt();

	if(!flag_using_airmouse) BKBTobiiREX::Halt();

	return 0;
}