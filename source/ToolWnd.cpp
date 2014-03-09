#include <Windows.h>
#include <stdint.h> // Это для uint64_t
#include "ToolWnd.h"
#include "BKBRepErr.h"
#include "KeybWnd.h"
#include "TobiiREX.h"
#include "AirMouse.h"

#define BKB_TOOLBOX_WIDTH 128
#define BKB_NUM_TOOLS 8

extern HINSTANCE BKBInst;
static const char *wnd_class_name="BKBTool";
static const char *tool_names[BKB_NUM_TOOLS]={"ЛЕВЫЙ","ПРАВЫЙ","ДВОЙНОЙ","ДРЕГ","СКРОЛЛ","КЛАВИШИ","Туда-Сюда","РЕЗЕРВ"};
static BKB_MODE tool_bm[BKB_NUM_TOOLS]={BKB_MODE_LCLICK, BKB_MODE_RCLICK, BKB_MODE_DOUBLECLICK, BKB_MODE_DRAG, 
	BKB_MODE_SCROLL, BKB_MODE_KEYBOARD, BKB_MODE_NONE, BKB_MODE_NONE};

int BKBToolWnd::screen_x, BKBToolWnd::screen_y;
HWND  BKBToolWnd::Tlhwnd;
int BKBToolWnd::current_tool=-1;
bool BKBToolWnd::left_side=false;

extern HBRUSH dkblue_brush, blue_brush;
extern int flag_using_airmouse;

// Оконная процедура 
LRESULT CALLBACK BKBToolWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
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
		TEXT(wnd_class_name)
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__FILE__,"RegisterClass (",__LINE__);
		return;
	}

	screen_x=GetSystemMetrics(SM_CXSCREEN);
	screen_y=GetSystemMetrics(SM_CYSCREEN);

	Tlhwnd=CreateWindowEx(
	WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	TEXT(wnd_class_name),
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	screen_x-BKB_TOOLBOX_WIDTH,0,BKB_TOOLBOX_WIDTH,screen_y, 
    0, 0, BKBInst, 0L );

	if(NULL==Tlhwnd)
	{
		BKBReportError(__FILE__,"CreateWindow",__LINE__);
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
		TextOut(hdc,25,60+i*screen_y/BKB_NUM_TOOLS,tool_names[i],(int)strlen(tool_names[i]));
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
	if((left_side&&(pnt->x<BKB_TOOLBOX_WIDTH)) || !left_side&&(pnt->x>screen_x-BKB_TOOLBOX_WIDTH))
	{
		// попала, определяем номер инструмента
		int tool_candidate=pnt->y/(screen_y/BKB_NUM_TOOLS);
		
		// пока восьмой выбрать нельзя
		if(tool_candidate>=7) return false;

		// Перенести панель инструментов в другую половину экрана
		if(6==tool_candidate)
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
		if((BKB_MODE_KEYBOARD==*bm)&&(tool_candidate>=4)) return false; 

		// если этот инструмент уже был выбран, деактивируем его
		if(tool_candidate==current_tool)
		{
			// Специальные действия с клавиатурой
			if(BKB_MODE_KEYBOARD==*bm)	BKBKeybWnd::DeActivate();	// деактивировать клавиатуру
			
			current_tool=-1;
			*bm=BKB_MODE_NONE;
		}
		else // замена одного инструмента на другой
		{
			// Специальные действия с клавиатурой
			if(BKB_MODE_KEYBOARD==*bm) BKBKeybWnd::DeActivate();	// деактивировать клавиатуру
	
			current_tool=tool_candidate;
			*bm=tool_bm[tool_candidate];

			if(BKB_MODE_KEYBOARD==*bm) BKBKeybWnd::Activate();	// активировать клавиатуру
		}

		// Пусть окно перерисует стандартная оконная процедура
		RECT rect={0,0,BKB_TOOLBOX_WIDTH,screen_y};
		InvalidateRect(Tlhwnd,&rect,TRUE);
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
	RECT rect={0,0,BKB_TOOLBOX_WIDTH,screen_y};
	InvalidateRect(Tlhwnd,&rect,TRUE);
}