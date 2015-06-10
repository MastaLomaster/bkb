#include <Windows.h>
#include "TranspWnd.h"
#include "BKBRepErr.h"
#include "WM_USER_messages.h"
#include "ToolWnd.h"
#include "Fixation.h"


static const TCHAR *wnd_class_name=L"BKBTransp";
extern HINSTANCE BKBInst;
extern HPEN dkyellow_pen, pink_pen;
extern bool flag_Activemouse;

bool BKBTranspWnd::flag_show_transp_window=true;

int BKBTranspWnd::screen_x, BKBTranspWnd::screen_y;
HWND  BKBTranspWnd::Trhwnd=0;
int BKBTranspWnd::last_progress=0, BKBTranspWnd::progress=0;

static const RECT clean_rect={5,5,95,15}; // Область стирания

// Оконная процедура 
LRESULT CALLBACK BKBTranspWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
		case WM_CREATE:
			// Содрано из интернета - так мы делаем окно прозрачным в белых его частях
			// Suppose the window background color is white (255,255,255).
            // Call the SetLayeredWindowAttributes when create the window.
            SetLayeredWindowAttributes(hwnd,RGB(255,255,255),NULL,LWA_COLORKEY);
            break;

	 case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc=BeginPaint(hwnd,&ps);
		
		if(flag_Activemouse) SelectObject(hdc,dkyellow_pen);
		else SelectObject(hdc,(HPEN)BLACK_PEN);

		MoveToEx(hdc,100,100,NULL);
		LineTo(hdc,49,49);
		MoveToEx(hdc,49,55,NULL);
		LineTo(hdc,49,49);
		LineTo(hdc,55,49);
		
		// Рисуем progress bar
		// Стираем, только если 0==progress
		if(0==BKBTranspWnd::progress)
		{
			FillRect(hdc, &clean_rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
		}
		else
		{
			SelectObject(hdc,pink_pen);
			MoveToEx(hdc,10,10,NULL);
			LineTo(hdc,10+80*BKBTranspWnd::progress/100,10);
		}

		EndPaint(hwnd,&ps);
		break;
	
	 case WM_USER_MOVEWINDOW:
		 MoveWindow(hwnd,wparam,lparam,100,100,FALSE);
		 break;

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Инициализация 
//================================================================
void BKBTranspWnd::Init(HWND master_hwnd)
{
	ATOM aresult; // Для всяких кодов возврата
	

	// 0.  Для аэромыши вообще его не создаём!
	if(!flag_show_transp_window)
	{
		Trhwnd=0;
		return;
	}

	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBTranspWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		(HBRUSH)GetStockObject(WHITE_BRUSH), 
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

	Trhwnd=CreateWindowEx(
	WS_EX_LAYERED|WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	//WS_EX_LAYERED|WS_EX_TOPMOST,
	wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	//100,100, // Не здесь ли крылась мерзкая ошибка, когда окно с курсором рисовалось в стороне?? Нет, похоже это было из-за HighDPI
	0,0,
	100,100, 
    //0,
	master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна
	0, BKBInst, 0L );

	if(NULL==Trhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	Show();
	UpdateWindow(Trhwnd);
}

void BKBTranspWnd::Move(int x, int y)
{
	// Это другой поток, а мы ждать не будем
	if(flag_show_transp_window)
		PostMessage(Trhwnd, WM_USER_MOVEWINDOW, x-50, y-50);
	//MoveWindow(Trhwnd,x-50,y-50,100,100,FALSE);
}

void BKBTranspWnd::Show()
{
	if(flag_show_transp_window)
		ShowWindow(Trhwnd,SW_SHOWNORMAL); 
}

void BKBTranspWnd::Hide()
{
	if(flag_show_transp_window)
		ShowWindow(Trhwnd,SW_HIDE); 
}

void BKBTranspWnd::ToTop()	
{ 
	if(flag_show_transp_window)
	{
		SetActiveWindow(Trhwnd);
		BringWindowToTop(Trhwnd); 
	}
	else
	{
		// Из-за отсутствия этого панель задач перекрывала панель инструментов 
		SetActiveWindow(BKBToolWnd::GetHwnd());
		BringWindowToTop(BKBToolWnd::GetHwnd()); 
	}
}

void BKBTranspWnd::Progress(int _progress)
{
	if(flag_show_transp_window)
	{
		if(!(
			(BKB_MODE_LCLICK==Fixation::CurrentMode())||
			(BKB_MODE_LCLICK_PLUS==Fixation::CurrentMode())||
			(BKB_MODE_RCLICK==Fixation::CurrentMode())||
			(BKB_MODE_DOUBLECLICK==Fixation::CurrentMode())||
			(BKB_MODE_DOUBLECLICK_PLUS==Fixation::CurrentMode())||
			(BKB_MODE_DRAG==Fixation::CurrentMode()) 
			))
		{
			progress=0; // Каким бы ни был _progress в других режимах, он не должен влиять на наш last_progress
		}
		else
		{
			progress=_progress;
			if(progress<0) progress=0;
			if(progress>100) progress=100;
		}

		if(last_progress!=progress)
			InvalidateRect(Trhwnd,NULL,false);
		last_progress=progress;
	}
}