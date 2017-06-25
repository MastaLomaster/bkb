#include <Windows.h>
#include "AirMouse.h"
#include "TranspWnd.h"
#include "ToolWnd.h"
#include "TobiiREX.h"

int g_BKB_MOUSE_X_MULTIPLIER=20, g_BKB_MOUSE_Y_MULTIPLIER=30; // усилитель мыши для тех, кому трудно поворачивать голову

// Заголовочные файлы из Tobii Gaze SDK - избавились
//#include "tobiigaze_error_codes.h"
//#include "tobiigaze.h"
//#include "tobiigaze_config.h" - такой файл был в Gaze SDK 2.0

// Прототип callback-функции из TobiiREX.cpp
void on_gaze_data(const toit_gaze_data* gazedata, void *user_data);

extern int screenX, screenY, mouscreenX, mouscreenY;

// Прототип перехватчика мыши (описан ниже)
LRESULT  CALLBACK HookProc(int disabled,WPARAM wParam,LPARAM lParam) ;
static HHOOK handle=0;
static bool hook_initialized=false;
bool skip_mouse_hook=false;
static LONG last_x,last_y;

//============================================================================
// Запускает таймер, по которому якобы приходят координаты курсора от Tobii
// 10 раз в секунду
//============================================================================
int BKBAirMouse::Init(HWND hwnd)
{
	SetTimer(hwnd,1,25,NULL);

	// Не показывать прозрачное окно, ибо курсор сам движется
#ifdef _DEBUG
	BKBTranspWnd::flag_show_transp_window=true;
#else
	BKBTranspWnd::flag_show_transp_window=false;
#endif

	// Для тех, кому трудно поворачивать голову
	// Теперь включаем всегда, а контролируем параметры внутри хука HookProc
	//if((g_BKB_MOUSE_X_MULTIPLIER!=10)||(g_BKB_MOUSE_Y_MULTIPLIER!=10))
	{
		if(!handle)
		{
			handle = SetWindowsHookEx(WH_MOUSE_LL, 
				HookProc, 
				GetModuleHandle(NULL), 
				NULL);
		}
	}

	return 0;
}

//============================================================================
// Убивает таймер, по которому якобы приходят координаты курсора от Tobii
// 10 раз в секунду
//============================================================================
int BKBAirMouse::Halt(HWND hwnd)
{
	KillTimer(hwnd,1);

	// Для тех, кому трудно поворачивать голову
	// Теперь выключаем всегда, а контролируем параметры внутри хука HookProc
	// if((g_BKB_MOUSE_X_MULTIPLIER!=10)||(g_BKB_MOUSE_Y_MULTIPLIER!=10))
	{
		if(handle)
		{
			UnhookWindowsHookEx(handle);
			handle = 0;
		}
	}

	return 0;
}

//============================================================================
// При срабатывании таймера имитирует сигнал от Tobii REX
//============================================================================
void BKBAirMouse::OnTimer()
{
	toit_gaze_data gd;

	POINT p;
	if(0==GetCursorPos(&p))
	{
		//OutputDebugString("Bad cursor position\n");
	}
	else
	{
//#ifdef BELYAKOV
//		BKBToolWnd::SleepCheck(&p);
//#endif
#ifdef _DEBUG
		// Имитируем нечёткое определение направления взгляда
#define BKB_EYE_RANDOM 300 
		p.x+=-BKB_EYE_RANDOM+(2*BKB_EYE_RANDOM*rand()/RAND_MAX);
		p.y+=-BKB_EYE_RANDOM+(2*BKB_EYE_RANDOM*rand()/RAND_MAX);
#endif

		//gd.tracking_status = TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED;
		gd.toit_status = 1;
		//gd.left.gaze_point_on_display_normalized.x=p.x/(double)screenX;
		//gd.left.gaze_point_on_display_normalized.y=p.y/(double)screenY;
		gd.left.bingo.x=p.x/(double)screenX;
		gd.left.bingo.y=p.y/(double)screenY;

		gd.right.bingo.x=gd.left.bingo.x;
		gd.right.bingo.y=gd.left.bingo.y;

		gd.timestamp=1000UL*timeGetTime(); // Используется скроллом

		on_gaze_data(&gd, NULL);
	}
}

//====================================================================================
// Собственно, хук 
//====================================================================================
LRESULT  CALLBACK HookProc(int disabled,WPARAM wParam,LPARAM lParam) 
{
	INPUT input={0};
	
    if ((!disabled)&&((g_BKB_MOUSE_X_MULTIPLIER!=10)||(g_BKB_MOUSE_Y_MULTIPLIER!=10)))
	{
		MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
		if (pMouseStruct != NULL)
		{
			switch(wParam)
			{
				case WM_MOUSEMOVE:
					if(hook_initialized)
					{
						// Пропускаем всё, что создали сами
						if(skip_mouse_hook)
						{
							//swprintf_s(debug_buf,_countof(debug_buf),L"%ld %ld\r\n",pMouseStruct->pt.x,pMouseStruct->pt.y);
							//OutputDebugString(debug_buf);
						}
						else
						{
							skip_mouse_hook=true;
							// Собственно, удвоение движение мыши
							
							input.type=INPUT_MOUSE;
							input.mi.dx=g_BKB_MOUSE_X_MULTIPLIER*(pMouseStruct->pt.x-last_x)/10;
							input.mi.dy=g_BKB_MOUSE_Y_MULTIPLIER*(pMouseStruct->pt.y-last_y)/10;
							//input.mi.dx=10;
							//input.mi.dy=10;
							input.mi.mouseData=0; // Нужно для всяких колёс прокрутки 
							//input.mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
							input.mi.dwFlags=MOUSEEVENTF_MOVE;
							input.mi.time=0;
							input.mi.dwExtraInfo=0;
							
							//swprintf_s(debug_buf,_countof(debug_buf),L"%ld %ld %ld %ld\r\n",pMouseStruct->pt.x,pMouseStruct->pt.y,input.mi.dx,input.mi.dy);
							//OutputDebugString(debug_buf);

							//
							//last_x=pMouseStruct->pt.x+input.mi.dx; last_y=pMouseStruct->pt.y+input.mi.dy;
							
							// Вынесли перед SendInput, хотя могли бы вообще убрать
							//last_x=pMouseStruct->pt.x; last_y=pMouseStruct->pt.y;
							SendInput(1,&input,sizeof(INPUT));

							skip_mouse_hook=false;
							
							hook_initialized=true;
							return 1;
						}
					}

				
					last_x=pMouseStruct->pt.x; last_y=pMouseStruct->pt.y;
					hook_initialized=true;
					if(last_x<0) last_x=0; if(last_y<0) last_y=0;

					if(last_x>=mouscreenX) last_x=mouscreenX-1;
					if(last_y>=mouscreenY) last_y=mouscreenY-1;
					break;
			}
		}
	}

	return CallNextHookEx(NULL,disabled,wParam,lParam);
}


