#include <Windows.h>
#include <stdint.h> // Это для uint64_t
#include "ToolWnd.h"
#include "BKBRepErr.h"
#include "KeybWnd.h"
#include "TobiiREX.h"
#include "AirMouse.h"
#include "TranspWnd.h"
#include "Internat.h"
#include "WM_USER_messages.h"


#define BKB_TOOLBOX_WIDTH 128
#define BKB_NUM_TOOLS 9

extern HINSTANCE BKBInst;
static const TCHAR *wnd_class_name=L"BKBTool";
static const TCHAR *tool_names[BKB_NUM_TOOLS];
static BKB_MODE tool_bm[BKB_NUM_TOOLS]={BKB_MODE_LCLICK, BKB_MODE_LCLICK_PLUS, BKB_MODE_RCLICK, BKB_MODE_DOUBLECLICK, BKB_MODE_DRAG, 
	BKB_MODE_SCROLL, BKB_MODE_KEYBOARD, BKB_MODE_NONE, BKB_MODE_NONE};

int BKBToolWnd::screen_x, BKBToolWnd::screen_y;
HWND  BKBToolWnd::Tlhwnd=0;
int BKBToolWnd::current_tool=-1;
bool BKBToolWnd::left_side=false;

extern HBRUSH dkblue_brush, blue_brush;
extern int flag_using_airmouse;

void on_gaze_data_main_thread(); // определена в TobiiREX.cpp

// Оконная процедура 
LRESULT CALLBACK BKBToolWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_USER_INVALRECT: // Это приходит из другого потока
		InvalidateRect(hwnd,NULL,TRUE);
		break;

	case WM_CREATE:
		SetLayeredWindowAttributes(hwnd,NULL,255*80/100,LWA_ALPHA);
		if(2==flag_using_airmouse) BKBAirMouse::Init(hwnd);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc=BeginPaint(hwnd,&ps);
		BKBToolWnd::OnPaint(hdc);
		EndPaint(hwnd,&ps);
		break;

	case WM_DESTROY:	// Завершение программы
		if(2==flag_using_airmouse) BKBAirMouse::Halt(hwnd);
		else	BKBTobiiREX::Halt();
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		BKBAirMouse::OnTimer();
		break;

	case WM_USER_DATA_READY:
		on_gaze_data_main_thread();
		break;

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Инициализация 
//================================================================
void BKBToolWnd::Init()
{
	ATOM aresult; // Для всяких кодов возврата
	
	// 0. Заполняем названия инструментов иностранным языком
	tool_names[0]=Internat::Message(12,L"ЛЕВЫЙ");
	tool_names[1]=Internat::Message(34,L"ЛЕВЫЙ,..");
	tool_names[2]=Internat::Message(13,L"ПРАВЫЙ");
	tool_names[3]=Internat::Message(14,L"ДВОЙНОЙ");
	tool_names[4]=Internat::Message(15,L"ДРЕГ");
	tool_names[5]=Internat::Message(16,L"СКРОЛЛ");
	tool_names[6]=Internat::Message(17,L"КЛАВИШИ");
	tool_names[7]=Internat::Message(18,L"Туда-Сюда");
	tool_names[8]=Internat::Message(19,L"РЕЗЕРВ");

	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBToolWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		//Надо бы красить фон
        dkblue_brush,
		//0,
		NULL,
		wnd_class_name
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return;
	}

	screen_x=GetSystemMetrics(SM_CXSCREEN);
	screen_y=GetSystemMetrics(SM_CYSCREEN);

	Tlhwnd=CreateWindowEx(
	WS_EX_LAYERED | WS_EX_TOPMOST| WS_EX_CLIENTEDGE,
	wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	screen_x-BKB_TOOLBOX_WIDTH,0,BKB_TOOLBOX_WIDTH,screen_y, 
    0, 0, BKBInst, 0L );

	if(NULL==Tlhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	ShowWindow(Tlhwnd,SW_SHOWNORMAL);
}

//================================================================
// Рисуем окно (Из WM_PAINT или сами)
//================================================================
void BKBToolWnd::OnPaint(HDC hdc)
{
	bool release_dc=false;

	if(0==hdc)
	{
		release_dc=true;
		hdc=GetDC(Tlhwnd);
	}

	// Собственно, рисование
	// 1. Сначала подсветим рабочий инструмент
	if(current_tool>=0)
	{
		RECT rect={0,current_tool*(screen_y/BKB_NUM_TOOLS),BKB_TOOLBOX_WIDTH,(current_tool+1)*(screen_y/BKB_NUM_TOOLS)};
		FillRect(hdc,&rect,blue_brush);
	}

	// цвета подправим
	SetTextColor(hdc,RGB(255,255,255));
	SetBkColor(hdc,RGB(45,62,90));
	SelectObject(hdc,GetStockObject(WHITE_PEN));

	int i;
	for(i=0;i<BKB_NUM_TOOLS;i++)
	{
		MoveToEx(hdc,0,i*screen_y/BKB_NUM_TOOLS,NULL);
		LineTo(hdc,BKB_TOOLBOX_WIDTH-1,i*screen_y/BKB_NUM_TOOLS);
		TextOut(hdc,25,60+i*screen_y/BKB_NUM_TOOLS,tool_names[i],wcslen(tool_names[i]));
	}


	// Если брал DC - верни его
	if(release_dc) ReleaseDC(Tlhwnd,hdc);

}

//================================================================
// Возможно переключение режима
//================================================================
bool BKBToolWnd::IsItYours(POINT *pnt, BKB_MODE *bm)
{
	// Попала ли точка фиксации в границы окна?
	// Ещё не включать режим резерв (потом)
//!!! Поправить для многомониторной конфигурации!!!
	if((left_side&&(pnt->x<BKB_TOOLBOX_WIDTH)) || !left_side&&(pnt->x>screen_x-BKB_TOOLBOX_WIDTH))
	{
		// попала, определяем номер инструмента
		int tool_candidate=pnt->y/(screen_y/BKB_NUM_TOOLS);
		
		// пока восьмой выбрать нельзя
		if(tool_candidate>=8) return false;

		// Перенести панель инструментов в другую половину экрана
		if(7==tool_candidate)
		{
			if(left_side)
			{
				left_side=false;
				MoveWindow(Tlhwnd, screen_x-BKB_TOOLBOX_WIDTH,0,BKB_TOOLBOX_WIDTH,screen_y,TRUE);
			}
			else
			{
				left_side=true;
				MoveWindow(Tlhwnd, 0,0,BKB_TOOLBOX_WIDTH,screen_y,TRUE);

			}
			return true; // В частности, не выводит увеличительное стекло
		}

		// а ещё нельзя включать скролл, когда работает клавиатура (легко промахнуться и нажать его вместо клавиши)
		// ОТМЕНЕНО
		//if((BKB_MODE_KEYBOARD==*bm)&&(tool_candidate>=4)) return false; 

		// Специальные действия с клавиатурой
		if(BKB_MODE_KEYBOARD==*bm)	BKBKeybWnd::DeActivate();	// деактивировать клавиатуру
		// если этот инструмент уже был выбран, деактивируем его
		if(tool_candidate==current_tool)
		{
			current_tool=-1;
			*bm=BKB_MODE_NONE;
		}
		else // замена одного инструмента на другой
		{
			current_tool=tool_candidate;
			*bm=tool_bm[tool_candidate];
			// Специальные действия с клавиатурой
			if(BKB_MODE_KEYBOARD==*bm) BKBKeybWnd::Activate();	// активировать клавиатуру
		}

		// Пусть окно перерисует стандартная оконная процедура
		 PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
		
		//RECT rect={0,0,BKB_TOOLBOX_WIDTH,screen_y};
		//InvalidateRect(Tlhwnd,&rect,TRUE);
		return true;
	}
	else return false;
}

//================================================================
// Текущий режим отработал, сбрасывай всё
//================================================================
void BKBToolWnd::Reset(BKB_MODE *bm)
{
	current_tool=-1;
	*bm=BKB_MODE_NONE;
	// Пусть окно перерисует стандартная оконная процедура
	 PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);

}

//=======================================================================
// В режиме скролла показывает курсор только когда он попадает на тулбар
//=======================================================================
void BKBToolWnd::ScrollCursor(POINT *p)
{
	static bool mouse_inside_toolbar=true, last_mouse_inside_toolbar=true; // Для скрытия второго курсора при перемещении за область тулбара
	
	if((left_side&&(p->x<BKB_TOOLBOX_WIDTH)) || !left_side&&(p->x>screen_x-BKB_TOOLBOX_WIDTH))
	{
		// Попали в тулбокс, покажите курсор
		mouse_inside_toolbar=true;
		if(false==last_mouse_inside_toolbar) BKBTranspWnd::Show(); // Показать стрелку
		BKBTranspWnd::Move(p->x,p->y);
	}
	else
	{
		// мимо тулбара
		mouse_inside_toolbar=false;
		if(true==last_mouse_inside_toolbar) BKBTranspWnd::Hide(); // Убрать стрелку
	}
	last_mouse_inside_toolbar=mouse_inside_toolbar;
}