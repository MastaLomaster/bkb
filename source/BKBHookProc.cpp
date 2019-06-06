#include <Windows.h>
#include "BKBHookProc.h"

bool volatile BKB_MBUTTON_PRESSED=false;
extern bool my_own_click;
extern int gBKB_MBUTTONFIX;
DWORD last_mouse_time=0;

//====================================================================================
// Собственно, хук (уже второй)
//====================================================================================
LRESULT  CALLBACK HookProc2(int disabled,WPARAM wParam,LPARAM lParam) 
{
	if (!disabled&&!my_own_click)
	{
		MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
		if (pMouseStruct != NULL)
		{
			if(WM_MOUSEWHEEL!=wParam) last_mouse_time=timeGetTime(); // скролл приводил к иллющии работы ассистента

			switch(wParam)
			{
			case WM_MBUTTONDOWN:
				if(0!=gBKB_MBUTTONFIX) // В режиме 0 - только настоящие фиксации, мышь игнорируем
				{
					BKB_MBUTTON_PRESSED=true;
					return 1; // Перехватываем, не даём превратиться в реальное нажатие
				}
				
				break;

			case WM_MBUTTONUP:
				if(0!=gBKB_MBUTTONFIX) // В режиме 0 - только настоящие фиксации, мышь игнорируем
				{
					BKB_MBUTTON_PRESSED=false; // сбрасывается в ongazedata, но подстрахуемся
					return 1; // Перехватываем, не даём превратиться в реальное отпускание
				}
				break;
			}
		}
	}
	return CallNextHookEx(NULL,disabled,wParam,lParam);
}