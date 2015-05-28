#include <Windows.h>
#include "MagnifyWnd.h"
#include "BKBRepErr.h"
#include "TranspWnd.h"

#include <stdio.h> 
char debug_buffer[4096];

extern double screen_scale;

#define MAGNIFY_WINDOW_SIZE 400 // по хорошему должнo делиться на 2*MAGNIFY_FACTOR
#define MAGNIFY_FACTOR 4 // Во сколько раз увеличиваем

extern HINSTANCE BKBInst;

static const TCHAR *wnd_class_name=L"BKBMagnify";

bool  BKBMagnifyWnd::mgf_visible=false; // Признак того, что окно сейчас висит на экране
HWND  BKBMagnifyWnd::Mghwnd;
int BKBMagnifyWnd::x_size, BKBMagnifyWnd::y_size, BKBMagnifyWnd::size_amendment;
int BKBMagnifyWnd::screen_x, BKBMagnifyWnd::screen_y;
int BKBMagnifyWnd::midpoint_x, BKBMagnifyWnd::midpoint_y;

// Оконная процедура (вырожденная)
LRESULT CALLBACK BKBMagnifyWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	return DefWindowProc(hwnd,message,wparam,lparam);
}

//================================================================
// Инициализация 
//================================================================
void BKBMagnifyWnd::Init(HWND master_hwnd)
{
	ATOM aresult; // Для всяких кодов возврата
	RECT rect;
	rect.top=0; rect.left=0; rect.right=MAGNIFY_WINDOW_SIZE-1; rect.bottom=MAGNIFY_WINDOW_SIZE-1;

	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBMagnifyWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		//Уже не надо красить фон
        //(HBRUSH)GetStockObject(DKGRAY_BRUSH),
		0,
		NULL,
		wnd_class_name
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return;
	}

	// Из-за рамки размер окна будет больше, чем 400x400
	AdjustWindowRectEx(&rect, WS_POPUP, false, WS_EX_TOPMOST|WS_EX_CLIENTEDGE);
	
	Mghwnd=CreateWindowEx(
	WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	100,700,
	rect.right-rect.left, rect.bottom-rect.top,
	//MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE, 
    //0,
	master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна
	0, BKBInst, 0L );

	if(NULL==Mghwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}


	// Определяем размер пользовательской области окна (отовизм, и так ясно, что 400x400)
	RECT rect2;
	GetClientRect(Mghwnd,&rect2);
	x_size=rect2.right-1;
	y_size=rect2.bottom-1;
	size_amendment=rect.left; // рамка окна отъедает пиксели (у меня 2 штуки с каждого края)

	screen_x=GetSystemMetrics(SM_CXSCREEN);
	screen_y=GetSystemMetrics(SM_CYSCREEN);
	
	//ShowWindow(Mghwnd,SW_SHOWNORMAL);
}

//=========================================================================================
// Если окно невидимо, то показать его. Если видимо, то уточнить координаты точки.
// Возвращает true, если мы попали в окно, и координаты точки уточнены засчёт увеличения
//=========================================================================================
bool BKBMagnifyWnd::FixPoint(POINT *pnt)
{
	if(mgf_visible) // Увеличительное окно открыто
	{
		// прячем окно в любом случае
		//mgf_visible=false;
		//ShowWindow(Mghwnd,SW_HIDE);

		// а попали ли мы в открытое окно?
		POINT local_point=*pnt;
		// Для отладки
		POINT p1,p2;
		//POINT p3;
		p1=local_point;
		ScreenToClient(Mghwnd,&local_point); // Координаты в клиентском окне
		p2=local_point;

		// прячем окно в любом случае
		mgf_visible=false;
		ShowWindow(Mghwnd,SW_HIDE);

		if((local_point.x>=0)&&(local_point.x<x_size)&&
			(local_point.y>=0)&&(local_point.y<y_size))
		{
			// Уточняем координаты для клика
			// Насколько точка удалена от середины - делим на увеличение и добавляем к midpoint
			pnt->x=midpoint_x+(local_point.x-x_size/2)/MAGNIFY_FACTOR;
			pnt->y=midpoint_y+(local_point.y-y_size/2)/MAGNIFY_FACTOR;
			// Кликайте на здоровье
			//p3=*pnt;
			//sprintf(debug_buffer,"До увеличения (экранные): %d %d\nВ окне увеличения (локальные): %d %d\nУточненные: %d %d",p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
			//MessageBox(Mghwnd,debug_buffer,"отладка",MB_OK);
			return true;
		}
		else
		{
			return false; // Кликать не надо
		}
		
	}
	else // окно невидимо, нужно его открыть
	{
		//screen_scale=1.0/1.25;

		// точка в середине окна
		midpoint_x=pnt->x; midpoint_y=pnt->y;
/*
		// В точке, которая передана параметром, должна быть видна середина окна
		// Если точка слишком близко к краю, корректируем её
		// левый край
		if(midpoint_x<MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_x=MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;
		if(midpoint_y<MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_y=MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;
		// правый край
		if(midpoint_x>screen_x-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_x=screen_x-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;
		if(midpoint_y>screen_y-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_y=screen_y-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;
*/
		// Двигаем окно
		int x_pos=midpoint_x-MAGNIFY_WINDOW_SIZE/2+size_amendment;
		int y_pos=midpoint_y-MAGNIFY_WINDOW_SIZE/2+size_amendment;

/*		// Окно не должно выезжать за экран
		if(x_pos<0) x_pos=0;
		if(x_pos>screen_x-MAGNIFY_WINDOW_SIZE) x_pos=screen_x-MAGNIFY_WINDOW_SIZE;
		
		if(y_pos<0) y_pos=0;
		if(y_pos>screen_y-MAGNIFY_WINDOW_SIZE) y_pos=screen_y-MAGNIFY_WINDOW_SIZE;
*/		
		// Реально окно несколько больше (у меня на 4 пиксела!), потом исправить это!!!
		MoveWindow(Mghwnd,x_pos,y_pos,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE, FALSE); // Последний параметр проверить

		// Делаем увеличенную копию фрагмента экрана

		// В этом месте нужно убрать окно с псевдокурсором
		BKBTranspWnd::Hide();

		HDC ScreenDC=GetDC(NULL); // Получаем DC экрана
		HDC MagBmpDC=CreateCompatibleDC(ScreenDC); // Создаём совместимый DC
		HBITMAP MagBmp=CreateCompatibleBitmap(ScreenDC,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE); // Создаем в нём битмап нужного размера
		HGDIOBJ OldBmp=SelectObject(MagBmpDC,MagBmp); // Выбираем битмап в DC (но сохраняем старый!!!)

		// Для отладки рисовал белый квадратик
		//RECT rrr={20,20,50,50};
		//FillRect(MagBmpDC,&rrr,(HBRUSH)GetStockObject(WHITE_BRUSH));

		// Копируем часть экрана в битмап (с увеличением)
	/*	StretchBlt(MagBmpDC,0,0,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE,ScreenDC,
			midpoint_x-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2,
			midpoint_y-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2,
			MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR,MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR,
			SRCCOPY); */

		StretchBlt(MagBmpDC,0,0,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE,ScreenDC,
			(midpoint_x-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2)/screen_scale,
			(midpoint_y-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2)/screen_scale,
			(MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR)/screen_scale,
			(MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR)/screen_scale,
			SRCCOPY);

		//screen_scale

		// делаем окно видимым (теперь это в самом конце)
		mgf_visible=true;
		ShowWindow(Mghwnd,SW_SHOWNORMAL);

		// Возвращаем окно с псевдокурсором
		BKBTranspWnd::Show();

		
		// Вернуть прозрачное окно наверх!!! (Теперь это делается после любого Fixation)
		//BKBTranspWnd::ToTop();

		// Выплёскиваем увеличенную часть экрана в окно
		HDC MgWindowDC=GetDC(Mghwnd);
		// Не забываем про рамку окна! Хоть это и пара пикселов, но всё же...
		//BitBlt(MgWindowDC,0,0,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE,MagBmpDC,0,0,SRCCOPY);
		BitBlt(MgWindowDC,0,0,x_size,y_size,MagBmpDC,
			(MAGNIFY_WINDOW_SIZE-x_size)/2,
			(MAGNIFY_WINDOW_SIZE-y_size)/2,
			SRCCOPY);
		

		ReleaseDC(Mghwnd,MgWindowDC);

		


		SelectObject(MagBmpDC,OldBmp); // Освобождаем наш битмап из DC
		DeleteObject(MagBmp); // Теперь наш битмап можно спокойно стереть
		DeleteDC(MagBmpDC); // Туда же и совместимый DC
		ReleaseDC(NULL,ScreenDC);

		
	}

	return false; // Точке не передано уточненное значение
}