#include <Windows.h>
#include "BKBTurtle.h"
#include "BKBRepErr.h"
#include "TranspWnd.h"
#include "ToolWnd.h"

//bool BKBTurtle::flag_right_bottom=0;
bool BKBTurtle::flag_right_bottom=1;
HWND BKBTurtle::hwndCenter, BKBTurtle::hwndArrow[4]; // Nord,West,South,East
// смещение всей конструкции 
int BKBTurtle::pos_x=0, BKBTurtle::pos_y=0;
int BKBTurtle::element_size=100;
POINT BKBTurtle::p={100,100};

HDC BKBTurtle::memdc=0;
HBITMAP BKBTurtle::hbm=0;

bool BKBTurtle::center_visible=true;
int BKBTurtle::swap_step=0;

static const TCHAR *wnd_class_name1=L"BKBTurtle1";
static const TCHAR *wnd_class_name2=L"BKBTurtle2";


extern HINSTANCE BKBInst;
extern HBRUSH dkblue_brush;
extern int screenX, screenY;
extern int gBKB_TOOLBOX_WIDTH;
extern bool flag_continuous_turtle; // Был ли выход из стрелки черепахи? если не был, то скорость можно постепенно наращивать

extern int tracking_device;
extern int NOTKBD_FIXATION_LIMIT;
extern HCURSOR hCursor;


//=================================================================================
// Две оконных функции, одна - для центрального окна, другая - для окон-стрелок
//=================================================================================
LRESULT CALLBACK BKBTurtleCenterWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc=BeginPaint(hwnd,&ps);
		BKBTurtle::OnPaintCenter(hdc);
		EndPaint(hwnd,&ps);
		break;

	case WM_CLOSE:
		// Нельзя!!!
		//ShowWindow(hwnd, SW_HIDE);
		break;

	case WM_SIZE:
		hdc=GetDC(hwnd);

		// Убираем DC и битмап
		if(BKBTurtle::memdc!=0) DeleteDC(BKBTurtle::memdc);
		if(BKBTurtle::hbm!=0) DeleteObject(BKBTurtle::hbm);
		
		// Воссоздаём DC и битмап
		BKBTurtle::memdc=CreateCompatibleDC(hdc);
		BKBTurtle::hbm=CreateCompatibleBitmap(hdc,LOWORD(lparam),HIWORD(lparam));
		SelectObject(BKBTurtle::memdc,BKBTurtle::hbm);
	
		ReleaseDC(hwnd,hdc);
		InvalidateRect(hwnd,NULL,TRUE); 

		break;

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}


LRESULT CALLBACK BKBTurtleArrowWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
		SetLayeredWindowAttributes(hwnd,NULL,255*60/100,LWA_ALPHA);
		//last_transparency=60;
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc=BeginPaint(hwnd,&ps);
		//BKBTurtle::OnPaintArrowWnd(hdc);
		EndPaint(hwnd,&ps);
		break;
	
	case WM_CLOSE:
		// Нельзя!!!
		//ShowWindow(hwnd, SW_HIDE);
		break;

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Инициализация 
//================================================================
void BKBTurtle::Init(HWND master_hwnd)
{
	ATOM aresult; // Для всяких кодов возврата

	// 1. Центральное окно
	//  Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBTurtleCenterWndProc, 0,
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		//(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL, 
		NULL,
		wnd_class_name1
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return;
	}

	hwndCenter=CreateWindowEx(WS_EX_TOPMOST,
	wnd_class_name1,
	NULL, 
	WS_POPUP ,
	300,300,
	200, 200,
    master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна
	0, BKBInst, 0L );

	if(NULL==hwndCenter)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	// Временно для отладки
	// ShowWindow(hwndCenter, SW_SHOW);

	// 2. Окна-срелки
	//  Регистрация класса окна
	WNDCLASS wcl2={CS_HREDRAW | CS_VREDRAW, BKBTurtleArrowWndProc, 0,
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		dkblue_brush,
		//NULL, 
		NULL,
		wnd_class_name2
	};

	aresult=::RegisterClass(&wcl2); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return;
	}

	for(int i=0;i<4;i++)
	{
		hwndArrow[i]=CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
		wnd_class_name2,
		NULL, 
		WS_POPUP ,
		350+50*i,350+50*i,
		200, 200,
		master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна
		0, BKBInst, 0L );

		if(NULL==hwndArrow[i])
		{
			BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
		}

		// Временно для отладки
		// ShowWindow(hwndArrow[i], SW_SHOW);
	}
}

//================================================================
// Показать или спрятать все окна черепашки разом
//================================================================
void BKBTurtle::Show(int nCmdShow)
{
	ShowWindow(hwndCenter, nCmdShow);
	for(int i=0;i<4;i++)
	{
		ShowWindow(hwndArrow[i], nCmdShow);
	}
	if(SW_SHOW==nCmdShow) center_visible=true;
	MoveArrow(0,0); // Скорректировать положение курсора
}

//================================================================
// Показать или спрятать все окна черепашки разом
//================================================================
void BKBTurtle::Place()
{
	// Размер элемента: меньшее из двух: высоты экрана/4 или остатка от ширины экрана без тулбара /5
	element_size=screenY/4;
	if((screenX-gBKB_TOOLBOX_WIDTH)/5<element_size) element_size=(screenX-gBKB_TOOLBOX_WIDTH)/5;


	// оставшееся пространство по ширине экрана оформляем как отступы справа и слева
	pos_x=(screenX-gBKB_TOOLBOX_WIDTH-5*element_size)/2;
	pos_y=(screenY-element_size*4)/2;

	// Когда тулбар слева - надо подвинуться
	if(BKBToolWnd::LeftSide()) 
		pos_x+=gBKB_TOOLBOX_WIDTH;

	// Если мы смещены в правый нижний угол экрана, то:
	pos_x+=flag_right_bottom*2*element_size;
	pos_y+=flag_right_bottom*element_size;

	// Таперича располагаем окна как надо
	MoveWindow(hwndArrow[0],pos_x+element_size,pos_y,element_size,element_size,TRUE); // North
	MoveWindow(hwndArrow[1],pos_x,pos_y+element_size,element_size,element_size,TRUE); // West
	MoveWindow(hwndArrow[2],pos_x+element_size,pos_y+2*element_size,element_size,element_size,TRUE); // South
	MoveWindow(hwndArrow[3],pos_x+2*element_size,pos_y+element_size,element_size,element_size,TRUE); // East

	MoveWindow(hwndCenter,pos_x+element_size,pos_y+element_size,element_size,element_size,TRUE); // Center

	MoveArrow(0,0); // Скорректировать положение курсора
}

static RECT pink_rect;
//================================================================
// Розовая обводка и перемещение курсора
//================================================================
LPRECT BKBTurtle::PinkFrame(int _x, int _y)
{
	POINT pnt={_x,_y};
	RECT crect;
	int i;

	// Проверка всех четырёх
	for(i=0;i<4;i++)
	{
		GetClientRect(hwndArrow[i],&crect);
		pnt.x=_x;
		pnt.y=_y;
		ScreenToClient(hwndArrow[i],&pnt);
		if((pnt.x>=0)&&(pnt.x<crect.right)&&(pnt.y>=0)&&(pnt.y<crect.bottom))
		{
			switch (i)
			{
			case 0: // North
				pink_rect.left=pos_x+element_size;
				pink_rect.top=pos_y;
				MoveArrow(0,-1);
				break;

			case 1: // West
				pink_rect.left=pos_x;
				pink_rect.top=pos_y+element_size;
				MoveArrow(-1,0);
				break;

			case 2: // South
				pink_rect.left=pos_x+element_size;
				pink_rect.top=pos_y+2*element_size;
				MoveArrow(0,1);
				break;

			case 3: // East
				pink_rect.left=pos_x+2*element_size;
				pink_rect.top=pos_y+element_size;
				MoveArrow(1,0);
				break;

			default: // Этого не может быть
				return NULL;
			}

			pink_rect.right=pink_rect.left+element_size;
			pink_rect.bottom=pink_rect.top+element_size;

			return &pink_rect;
		} // попали
	} // for i


	return NULL;
}

//================================================================
// Розовая обводка центра
//================================================================
LPRECT BKBTurtle::PinkFrameCenter(int _x, int _y)
{
	// Попала ли точка в границы окна?
	POINT pnt={_x,_y};
	RECT crect;
	GetClientRect(hwndCenter,&crect);
	ScreenToClient(hwndCenter,&pnt);
	if((pnt.x>=0)&&(pnt.x<crect.right)&&(pnt.y>=0)&&(pnt.y<crect.bottom))
	{
		pink_rect.left=pos_x+element_size;
		pink_rect.top=pos_y+element_size;
		pink_rect.right=pink_rect.left+element_size;
		pink_rect.bottom=pink_rect.top+element_size;

		return &pink_rect;
	}
	else return NULL;
}

//================================================================
// Передвижение курсора (или окна)
//================================================================
static int cursor_speed=5;
void BKBTurtle::MoveArrow(int dx, int dy)
{
	static int continuous_count=0;
	continuous_count++;

	if(!flag_continuous_turtle) continuous_count=0; // пришли сюда только что 

	if(continuous_count<NOTKBD_FIXATION_LIMIT/5) cursor_speed=0; // Начинаем движение не сразу
	else if(continuous_count<NOTKBD_FIXATION_LIMIT) cursor_speed=1;
	else if(continuous_count>NOTKBD_FIXATION_LIMIT) { cursor_speed=5; continuous_count=NOTKBD_FIXATION_LIMIT; }
	
		
	// При аэромыши - берём координаты прозрачного окна (с учётом HighDPI)
	// !!! Потом проверить на windows 8.1
	// При других трекерах - координаты курсора

	if(2==tracking_device) BKBTranspWnd::GetPos(&p);
	else GetCursorPos(&p);

	p.x+=dx*cursor_speed;
	p.y+=dy*cursor_speed;

// !!! Здесь запретить уезжать за границы экрана более, чем на 50 пикселов (8.1 - проверить!!!)
	if(p.x<0) p.x=0;
	if(p.x>screenX) p.x=screenX;
	if(p.y<0) p.y=0;
	if(p.y>screenY) p.y=screenY;
		
	if(2==tracking_device) BKBTranspWnd::Move(p.x, p.y);
	else SetCursorPos(p.x, p.y);

//!!! Тут надо при определённых условиях прятать или показывать окно
	if((p.x>pos_x+element_size-element_size/2)&&(p.y>pos_y+element_size-element_size/2)&&(p.x<pos_x+2*element_size+element_size/2)&&(p.y<pos_y+2*element_size+element_size/2)) // опасная зона
	{
		// окно мешает
		if(center_visible)
		ShowWindow(hwndCenter, SW_HIDE);
		center_visible=false;
	}
	else // окно видимо
	{
		if(!center_visible)
		ShowWindow(hwndCenter, SW_SHOW);
		center_visible=true;
	}

	// А теперь сделаем копирование экрана в наш загашник!
	InvalidateRect(hwndCenter,NULL,FALSE);
}

//===================================================================================================
// Копирует содержимое экрана под курсором в центральное окно
//===================================================================================================
void BKBTurtle::OnPaintCenter(HDC hdc)
{
	if(BKBTurtle::memdc)
	{
		HDC ScreenDC=GetDC(NULL); // Получаем DC экрана
		RECT r={0,0,element_size,element_size};

		FillRect(memdc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));

		BitBlt(memdc,0,0,element_size,element_size,ScreenDC,p.x-element_size/2,p.y-element_size/2,SRCCOPY);

		if(2!=tracking_device)
		{
			// Рисуем курсор (стандартный)
			DrawIcon(memdc,element_size/2,element_size/2,hCursor);
		}

		BitBlt(hdc,0,0,element_size,element_size,memdc,0,0,SRCCOPY);

		ReleaseDC(NULL,ScreenDC);
	}
}

//===================================================================================================
// Интересует только попадание в центральное окно
//===================================================================================================
bool BKBTurtle::IsItYours(POINT *_pnt)
{
	// Попала ли точка в границы окна?
	POINT pnt=*_pnt;
	RECT crect;
	GetClientRect(hwndCenter,&crect);
	ScreenToClient(hwndCenter,&pnt);
	if((pnt.x>=0)&&(pnt.x<crect.right)&&(pnt.y>=0)&&(pnt.y<crect.bottom))
	{
		if(swap_step<2) // Добавть точку на экран
		{
			swap_step++;
		}
		else // Можно менять расположение
		{
			swap_step=0;

			if(flag_right_bottom) flag_right_bottom=0;
			else flag_right_bottom=1;

			Place();
		}
		InvalidateRect(hwndCenter,NULL,FALSE);
		return true;
	}

	return false;
}