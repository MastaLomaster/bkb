#include <Windows.h>
#include "TranspWnd.h"
#include "BKBRepErr.h"

static const char *wnd_class_name="BKBTransp";
extern HINSTANCE BKBInst;

bool BKBTranspWnd::flag_show_transp_window=true;

int BKBTranspWnd::screen_x, BKBTranspWnd::screen_y;
HWND  BKBTranspWnd::Trhwnd=0;

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
		
		MoveToEx(hdc,100,100,NULL);
		LineTo(hdc,49,49);
		MoveToEx(hdc,49,55,NULL);
		LineTo(hdc,49,49);
		LineTo(hdc,55,49);
		
		EndPaint(hwnd,&ps);
		break;
		

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Инициализация 
//================================================================
void BKBTranspWnd::Init()
{
	ATOM aresult; // Для всяких кодов возврата
	
	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBTranspWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		(HBRUSH)GetStockObject(WHITE_BRUSH), 
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

	Trhwnd=CreateWindowEx(
	WS_EX_LAYERED|WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	//WS_EX_LAYERED|WS_EX_TOPMOST,
	TEXT(wnd_class_name),
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	//100,100, // Вот где крылась мерзкая ошибка, когда окно с курсором рисовалось в стороне !!!
	0,0,
	100,100, 
    0, 0, BKBInst, 0L );

	if(NULL==Trhwnd)
	{
		BKBReportError(__FILE__,"CreateWindow",__LINE__);
	}

	Show();
	UpdateWindow(Trhwnd);
}

void BKBTranspWnd::Move(int x, int y)
{
	MoveWindow(Trhwnd,x-50,y-50,100,100,FALSE);
}

void BKBTranspWnd::Show()
{
	if(flag_show_transp_window)
		ShowWindow(Trhwnd,SW_SHOWNORMAL); 
}

void BKBTranspWnd::Hide()
{
	ShowWindow(Trhwnd,SW_HIDE); 
}

	