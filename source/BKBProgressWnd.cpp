#include <Windows.h>
#include "BKBProgressWnd.h"
#include "BKBRepErr.h"
#include "Fixation.h"
#include "KeybWnd.h"
#include "ToolWnd.h"
#include "TranspWnd.h"
#include "BKBTurtle.h"

static const TCHAR *wnd_class_name=L"BKBProgress";
extern HINSTANCE BKBInst;
extern HPEN pink_pen;

HWND  BKBProgressWnd::PRhwnd=0;
int  BKBProgressWnd::percentage=0;

extern bool gBKB_2STEP_KBD_MODE;
bool flag_continuous_turtle=false; // Был ли выход из стрелки черепахи? если не был, то скорость можно постепенно наращивать

// Оконная процедура 
LRESULT CALLBACK BKBProgressWndProc(HWND hwnd,
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
/*
	case WM_SIZE:
		break;
*/
	 case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		RECT rect;
		int width, height;

		hdc=BeginPaint(hwnd,&ps);
		
		GetClientRect(hwnd,&rect);
		width=rect.right-1;
		height=rect.bottom-1;

		SelectObject(hdc, pink_pen);

		MoveToEx(hdc,2,2,NULL);
		LineTo(hdc,width-3,2);
		LineTo(hdc,width-3,height-3);
		LineTo(hdc,2,height-3);
		LineTo(hdc,2,2);
		
		if((BKBProgressWnd::percentage>0)&&(BKB_MODE_TURTLE!=Fixation::CurrentMode())) // Черепашка без прогресса
		{
			MoveToEx(hdc,15,15, NULL);
			LineTo(hdc,15+(width-30)*BKBProgressWnd::percentage/100,15);
		}
		
		if(BKBTurtle::swap_step>0) // Это возможно, только если мы непрерывно глядим на центральное окно черепашки
		{
			int i;
			for(i=0;i<BKBTurtle::swap_step;i++)
			{
				MoveToEx(hdc,rect.right-10-i*10,rect.bottom-10,NULL);
				LineTo(hdc,rect.right-10-i*10,rect.bottom-10);
			}
		}

		EndPaint(hwnd,&ps);
		break;
/*	
	 case WM_USER_MOVEWINDOW:
		 MoveWindow(hwnd,wparam,lparam,100,100,FALSE);
		 break;
*/
	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Инициализация 
//================================================================
void BKBProgressWnd::Init(HWND master_hwnd)
{
	ATOM aresult; // Для всяких кодов возврата
	
	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBProgressWndProc, 0,
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

// Теперь это есть в глобальных переменных 
//	screen_x=GetSystemMetrics(SM_CXSCREEN);
//	screen_y=GetSystemMetrics(SM_CYSCREEN);

	PRhwnd=CreateWindowEx(
	WS_EX_LAYERED|WS_EX_TOPMOST,
	//|WS_EX_CLIENTEDGE,
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

	if(NULL==PRhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	//Временно для отладки
	ShowWindow(PRhwnd, SW_SHOW);
	UpdateWindow(PRhwnd);
}


//====================================================================================================================
// Возможно, есть где нарисовать розовый прямоугольник?
//====================================================================================================================
bool BKBProgressWnd::TryToShow(int _x, int _y, int _percentage) // true = Pink Approves, продолжаем фиксацию, невзирая на дисперсию
{
	LPRECT lp_pink_rect;
	static int old_top, old_left, old_right, old_bottom;


	// Сначала ловим тулбар
	lp_pink_rect=BKBToolWnd::PinkFrame(_x, _y);
	if(NULL==lp_pink_rect)
	{
		// Черепашка
		if(BKB_MODE_TURTLE==Fixation::CurrentMode())
		{
			lp_pink_rect=BKBTurtle::PinkFrame(_x, _y); // Проверка лепестков
			if(NULL==lp_pink_rect)
			{
				flag_continuous_turtle=false; // Непрерывный взгляд на стрелку черепашки соскочил
								
				lp_pink_rect=BKBTurtle::PinkFrameCenter(_x, _y); // Проверка центрального элемента
				if(NULL==lp_pink_rect) BKBTurtle::swap_step=0; // Каким бы ни был шаг фиксации - он слетел
			}
			else
			{
				flag_continuous_turtle=true; // Единственное место, где мы подтверждаем непрерывность
				BKBTurtle::swap_step=0;
			}

		}


		// Пробуем искать в клавиатуре
		if(gBKB_2STEP_KBD_MODE&&(BKB_MODE_KEYBOARD==Fixation::CurrentMode()))
		{
			// В процессе анимации никакие фиксации не проходят
			if(BKBKeybWnd::Animate()) return false;

			lp_pink_rect=BKBKeybWnd::PinkFrame(_x, _y);
		}
	}
	else
	{
		flag_continuous_turtle=false; // Непрерывный взгляд на стрелку черепашки соскочил
		BKBTurtle::swap_step=0; // Каким бы ни был шаг фиксации - он слетел
	}
	
	if(NULL!=lp_pink_rect) // Наш клиент!
	{
		// На самом курсоре progress bar сбрасываем
		BKBTranspWnd::Progress(0);

		if((old_top==lp_pink_rect->top)&&(old_left==lp_pink_rect->left)&&(old_right==lp_pink_rect->right)&&(old_bottom==lp_pink_rect->bottom))
		{
			// Прямоугольник на старом месте, меняем только ProgressBar
			percentage=_percentage;
			InvalidateRect(PRhwnd,NULL,TRUE);
			return true; // Fixation approved
		}
		else // Новое местоположение прямоугольника
		{
			// Новое расположение прямоугольника должно остановить рост fixation_count
			// Но только если мы свалились с соседнего прямоугольника, а не из пустоты
			bool return_value=true;
			if(-1!=old_top) 	return_value=false;

			percentage=0;
			ShowWindow(PRhwnd, SW_SHOWNORMAL);
			MoveWindow(PRhwnd,lp_pink_rect->left,lp_pink_rect->top, lp_pink_rect->right-lp_pink_rect->left+1, lp_pink_rect->bottom-lp_pink_rect->top+1, TRUE);
			InvalidateRect(PRhwnd,NULL,TRUE);

			SetActiveWindow(PRhwnd);
			BringWindowToTop(PRhwnd); 

			old_top=lp_pink_rect->top;
			old_left=lp_pink_rect->left;
			old_right=lp_pink_rect->right;
			old_bottom=lp_pink_rect->bottom;

			return return_value;
		}
		
		
		
	}
	else
	{
		// На курсоре рисуем progress bar, когда его нет на других элементах
		if(-1!=old_top) _percentage=0; // При соскакивании с тулбара (или чего там ещё) процент считать нулём
		BKBTranspWnd::Progress(_percentage);

		old_top=-1; // Это чтобы после возвращения в тот же прямоугольник из пустоты, он не решил, что ничего не произошло, и вышел бы в SW_SHOWNORMAL
		ShowWindow(PRhwnd, SW_HIDE);
		

		
		return false;
	}
	
}