#include <Windows.h>
#include "BKBMetricsWnd.h"
#include "BKBRepErr.h"
#include "Internat.h"

static const TCHAR *wnd_class_name=L"BKBMetrics";
static TCHAR *text_metrics=L"Фиксация ниже красной";

extern HINSTANCE BKBInst;
extern HPEN green_pen, dkyellow_pen, red_pen, strange_pen;
extern HBRUSH dkyellow_brush;
extern int gBKB_SHOW_METRICS;
extern int gBKB_DISP_PERCENT;
extern int screenY;

HWND  BKBMetricsWnd::MTXhwnd=0;

HDC BKBMetricsWnd::memdc1, BKBMetricsWnd::memdc2;
HBITMAP BKBMetricsWnd::hbm1, BKBMetricsWnd::hbm2; 

bool BKBMetricsWnd::show=0;

// Оконная процедура 
LRESULT CALLBACK BKBMetricsWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_CLOSE:
		ShowWindow(hwnd, SW_HIDE);
		break;


	
	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Инициализация 
//================================================================
void BKBMetricsWnd::Init(HWND master_hwnd)
{
	ATOM aresult; // Для всяких кодов возврата
	RECT rect={0,0,179,179};
	
	// 0. Перевод заголовка окна
	if(Internat::Message(73,0)) text_metrics=Internat::Message(73,0); // фиксация ниже красной 

	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBMetricsWndProc, 0,
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		dkyellow_brush, 
		NULL,
		wnd_class_name
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return;
	}


	// Из-за рамки размер окна будет больше, чем 180x180
	AdjustWindowRectEx(&rect, WS_POPUP | WS_CAPTION | WS_SYSMENU, false, WS_EX_TOPMOST);

	MTXhwnd=CreateWindowEx(WS_EX_TOPMOST,
	wnd_class_name,
	text_metrics,
	// NULL, 
	// WS_POPUP | WS_CAPTION | WS_SYSMENU ,
	WS_POPUP | WS_CAPTION,
	30,30,
	rect.right-rect.left, rect.bottom-rect.top,
    //0,
	master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна
	0, BKBInst, 0L );

	if(NULL==MTXhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	//GetClientRect(MTXhwnd,&rect);

	// Создаём два DC в памяти
	HDC hdc=GetDC(MTXhwnd);
	RECT r2={0,0,180,180};

	memdc1=CreateCompatibleDC(hdc);
	memdc2=CreateCompatibleDC(hdc);

	hbm1=CreateCompatibleBitmap(hdc,180,180);
	hbm2=CreateCompatibleBitmap(hdc,180,180);

	SelectObject(memdc1,hbm1);
	SelectObject(memdc2,hbm2);

	FillRect(memdc2,&r2,dkyellow_brush); // Побелим холст
	ReleaseDC(MTXhwnd,hdc);

	//Временно для отладки
	Show(gBKB_SHOW_METRICS);
	//UpdateWindow(MTXhwnd);
}

//================================================================
// Добавляет в график очередной отсчёт дисперсии
// Временно считаем 100%=0.3 высоты экрана
//================================================================
void BKBMetricsWnd::OnTick(float dispersion)
{
	LONG y, y_max;

	// 0. Если окно невидимо, возврат
	if(!show) return;

	// Максимум 30% от высоты экрана
	y_max=screenY*3/10;

	// 1. Переведём полученный процент в координату Y
	if(dispersion<0.0f) y=180;
	else if(dispersion>y_max) y=0;
	else y=180-180*dispersion/y_max;

	// 2. Копируем второй слой в первый со сдвигом влево на один пиксел
	BitBlt(memdc1,0,0,179,180,memdc2,1,0,SRCCOPY);

	// 3. Дорисовываем последнее значение из percent
	if(dispersion>=0) // Глаза были видны
	{
		SelectObject(memdc1,dkyellow_pen); // Сначала зачёркиваем белым то, что было нарисовано раньше
		MoveToEx(memdc1,179,179,NULL);
		LineTo(memdc1,179,-1); 
		SelectObject(memdc1,green_pen);
		MoveToEx(memdc1,179,179,NULL);
		LineTo(memdc1,179,y);
	}
	else // Глаза не были видны, рисуем серую линию
	{
		SelectObject(memdc1,strange_pen); // Сначала зачёркиваем белым то, что было нарисовано раньше
		MoveToEx(memdc1,179,179,NULL);
		LineTo(memdc1,179,-1); 
	}
	

	// 4. Возвращаем новую картинку во второй слой 
	BitBlt(memdc2,0,0,180,180,memdc1,0,0,SRCCOPY);

	// 5. Отрисовываем тут всякие оси-надписи
	int i;
	for(i=10;i<=25;i+=5)
	{
		y=180-180*i/30;
		if(i==gBKB_DISP_PERCENT) SelectObject(memdc1,red_pen);
		else SelectObject(memdc1,strange_pen);
		MoveToEx(memdc1,0,y,NULL);
		LineTo(memdc1,180,y);
	}

	// 6. Выкладываем на экран
	HDC hdc=GetDC(MTXhwnd);
	BitBlt(hdc,0,0,180,180,memdc1,0,0,SRCCOPY);
	ReleaseDC(MTXhwnd,hdc);
}

//================================================================
// Показывает или прячет окно метрик
//================================================================
void BKBMetricsWnd::Show(bool _show)
{
	if(_show) // покажите нам окно
	{
		if(!show) // Было спрятано, надо почистить старый график
		{
			RECT r2={0,0,180,180};
			FillRect(memdc2,&r2,dkyellow_brush); // Побелим холст
		}
		ShowWindow(MTXhwnd, SW_SHOW);
	}
	else
	{
		ShowWindow(MTXhwnd, SW_HIDE);
	}

	show=_show;
}