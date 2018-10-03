#include <Windows.h>
#include <stdio.h> // Это для отладки 
#include "Fixation.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "KeybWnd.h"
#include "WM_USER_messages.h"
#include "BKBTurtle.h"

//static char debug_buf[4096]; 

extern bool skip_mouse_hook; // Не удлиннять движения мыши, если мы их сами запрограммировали

bool my_own_click=false; // Чтобы хук не перехватывал свои собственные клики

BKB_MODE Fixation::BKB_Mode=BKB_MODE_NONE;
bool Fixation::drag_in_progress=false;

//==============================================================================================
// Взгляд зафиксировался
// Возвращает true, если взгляд зафиксировался где надо , и можно переходить к следующему шагу
//==============================================================================================
bool Fixation::Fix(POINT p)
{

	// Это чтобы случайно не переключить режим в середине дрега
	static bool drag_in_progress=false;

	// Смотря какой режим выбран
	switch (BKB_Mode)
	{
	// Сначала все мышиные с увеличением
	case BKB_MODE_LCLICK: // Щелчок левой кнопкой мыши
	case BKB_MODE_LCLICK_PLUS: // Повторяющийся щелчок левой кнопкой мыши
	case BKB_MODE_RCLICK: // Щелчок правой кнопкой мыши
	//case BKB_MODE_RCLICK_PLUS: // Повторяющийся щелчок правой кнопкой мыши
	case BKB_MODE_DOUBLECLICK: // Двойной щелчок
	case BKB_MODE_DOUBLECLICK_PLUS: // Повторяющийся двойной щелчок
	case BKB_MODE_DRAG: // Ну, дрег

		// Если окно уже показано, получить координаты на экране в этом окне
		// Переключение режима с открытым окном Magnify невозможно
		if(BKBMagnifyWnd::IsVisible()||BKBToolWnd::tool_modifier[3]) // 3-й параметр меню - работать без зума
		{
			bool bresult=false;
			if(BKBMagnifyWnd::IsVisible()) bresult=BKBMagnifyWnd::FixPoint(&p); // попали в окно, координаты точки уточнены при увеличении
			else bresult=!BKBToolWnd::IsItYours(&p, &BKB_Mode); // продолжаем, только если без зума не попали в тулбар

			if(bresult)
			{
				switch (BKB_Mode) // О! Опять!
				{
					case BKB_MODE_LCLICK: // Щелчок левой кнопкой мыши
						LeftClick(p);
						BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то, когда нет повтора
						//if(!BKBToolWnd::tool_modifier[0]) BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то, когда нет повтора
						break;

					case BKB_MODE_LCLICK_PLUS: // Повторяющийся щелчок левой кнопкой мыши
						LeftClick(p);
						break; 

					case BKB_MODE_RCLICK: // Щелчок правой кнопкой мыши
						RightClick(p);
						// Теперь после правого клика автоматически включается левый повторяющийся
						BKB_Mode=BKB_MODE_LCLICK_PLUS;
						BKBToolWnd::SetCurrentTool(BKB_Mode);
						// Пусть окно перерисует стандартная оконная процедура
						PostMessage(BKBToolWnd::GetHwnd(), WM_USER_INVALRECT, 0, 0);
						//BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то, когда нет повтора
						//if(!BKBToolWnd::tool_modifier[0]) BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то, когда нет повтора
						break;

					/*case BKB_MODE_RCLICK_PLUS: // Повторяющийся щелчок правой кнопкой мыши
						RightClick(p);
						break; */

					case BKB_MODE_DOUBLECLICK: // Двойной щелчок
						DoubleClick(p);
						BKBToolWnd::Reset(&BKB_Mode);
						//if(!BKBToolWnd::tool_modifier[0]) BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то, когда нет повтора
						break;

					case BKB_MODE_DOUBLECLICK_PLUS: // Повторяющийся двойной щелчок
						DoubleClick(p);
						break;

					case BKB_MODE_DRAG: // Ну, дрег
						drag_in_progress=Drag(p);
						if(!drag_in_progress) BKBToolWnd::Reset(&BKB_Mode);
						break;
				}
			}
			// else = промахнулись мимо окна, окно скрылось, режим не изменился
		}
		else // Окно с увеличением не активно
		{
			if(!drag_in_progress) // Окно-то неактивно, а вдруг тянем?
			{
				// либо переключаем режим, либо показываем окно Magnify
				if(!BKBToolWnd::IsItYours(&p, &BKB_Mode))
				{
					// Мимо Toolbox, показываем окно Magnify
					BKBMagnifyWnd::FixPoint(&p);
				}
			}
			else // При drag_in_progress нужно закончить его
			{
				if(BKBMagnifyWnd::FixPoint(&p)) // попали в окно, координаты точки уточнены при увеличении
				{
					drag_in_progress=Drag(p); 
				}
			}
		}
		break;

	case BKB_MODE_KEYBOARD: 
		// клавиша нажата?
		if(!BKBKeybWnd::IsItYours(&p, &BKB_Mode))
		{
			// нет, возможно, это переключение режима
			// здесь продумать, как увеличить время фиксации...
			BKBToolWnd::IsItYours(&p, &BKB_Mode);
		}
		break;

	case BKB_MODE_TURTLE:
		// Попали в черепаху?
		if(!BKBTurtle::IsItYours(&p))
		{
			// нет, возможно, это переключение режима
			BKBToolWnd::IsItYours(&p, &BKB_Mode);
		}
		break;

	default: // Все, кого мы пока не умеем обрабатывать, должны хотя-бы переключать режим
		// В том числе BKB_MODE_NONE
		BKBToolWnd::IsItYours(&p, &BKB_Mode);
	}



	return true;
}

//==============================================================================================
// Имитирует нажатие и отпускание левой кнопки мыши
//==============================================================================================
void Fixation::LeftClick(POINT p, bool skip_modifier_press, bool skip_modifier_unpress)
{
	my_own_click=true; // Чтобы хук не перехватывал свои собственные клики

	// Содрано из интернета
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	int xs,ys,x,y;
	
	xs=XSCALEFACTOR;
	ys=YSCALEFACTOR;
	x=p.x;
	y=p.y;
	//screenx=GetSystemMetrics(SM_CXSCREEN);
	//sprintf(debug_buf,"xs:%d ys:%d x:%d y:%d",xs,ys,x,y);
	//MessageBox(NULL,debug_buf,"debug",MB_OK);

	if(!skip_modifier_press) ClickModifiers(true); // Вдруг нужно добавить Ctrl, Shift, Alt?

	INPUT input[3]={0};

	// 1. Сначала подвинем курсор
	input[0].type=INPUT_MOUSE;
	input[0].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[0].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[0].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
	input[0].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
	input[0].mi.time=0;
	input[0].mi.dwExtraInfo=0;

	// 2. нажатие левой кнопки
	input[1].type=INPUT_MOUSE;
	input[1].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[1].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[1].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
	input[1].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
	input[1].mi.time=0;
	input[1].mi.dwExtraInfo=0;

	// 3. отпускание левой кнопки
	input[2].type=INPUT_MOUSE;
	input[2].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[2].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[2].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
	input[2].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP;
	input[2].mi.time=0;
	input[2].mi.dwExtraInfo=0;
		
	// Имитирует нажатие и отпускание левой кнопки мыши
	skip_mouse_hook=true;
	SendInput(3,input,sizeof(INPUT));
	skip_mouse_hook=false;

	if(!skip_modifier_unpress) ClickModifiers(false); // Вдруг нужно отпустить Ctrl, Shift, Alt?

	my_own_click=false; 
}

//==============================================================================================
// Имитирует нажатие и отпускание правой кнопки мыши
//==============================================================================================
void Fixation::RightClick(POINT p)
{
	// Содрано из интернета
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	ClickModifiers(true); // Вдруг нужно добавить Ctrl, Shift, Alt?

	INPUT input[3]={0};

	// 1. Сначала подвинем курсор
	input[0].type=INPUT_MOUSE;
	input[0].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[0].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[0].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
	input[0].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
	input[0].mi.time=0;
	input[0].mi.dwExtraInfo=0;

	// 2. нажатие правой кнопки
	input[1].type=INPUT_MOUSE;
	input[1].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[1].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[1].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
	input[1].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN;
	input[1].mi.time=0;
	input[1].mi.dwExtraInfo=0;

	// 3. отпускание правой кнопки
	input[2].type=INPUT_MOUSE;
	input[2].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[2].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[2].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
	input[2].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP;
	input[2].mi.time=0;
	input[2].mi.dwExtraInfo=0;
		
	// Имитирует нажатие и отпускание правой кнопки мыши
	skip_mouse_hook=true;
	SendInput(3,input,sizeof(INPUT));
	skip_mouse_hook=false;

	ClickModifiers(false); // Вдруг нужно отпустить Ctrl, Shift, Alt?
}

//==============================================================================================
// Имитирует сами знаете что
//==============================================================================================
void Fixation::DoubleClick(POINT p)
{
	LeftClick(p,false,true); // Не отпускаем Ctrl и т.д.
	Sleep(80);
	LeftClick(p,true,false); // Не нажимаем повторно Ctrl и т.д.
}


//==============================================================================================
// Имитирует начало и конец дрега
//==============================================================================================
bool Fixation::Drag(POINT p)
{
	static POINT p_initial;

	// Содрано из интернета
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	// Из-за того, что не обнулял, были страшные глюки. Очень странно...
	// разобрался. SendInput вызывался даже на первом шаге, вот и все глюки...
	INPUT input[4]={0};

	if(!drag_in_progress) // Только нажимаем
	{
		drag_in_progress=true;
		// просто запоминаем исходную позицию
		p_initial=p;

		// Теперь нажимаем и удерживаем кнопку уже после первой фиксации
		// 1. Сначала подвинем курсор
		input[0].type=INPUT_MOUSE;
		input[0].mi.dx=(LONG)(p_initial.x*XSCALEFACTOR);
		input[0].mi.dy=(LONG)(p_initial.y*YSCALEFACTOR);
		input[0].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[0].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
		input[0].mi.time=0;
		input[0].mi.dwExtraInfo=0;

		// 2. нажатие левой кнопки
		input[1].type=INPUT_MOUSE;
		input[1].mi.dx=(LONG)(p_initial.x*XSCALEFACTOR);
		input[1].mi.dy=(LONG)(p_initial.y*YSCALEFACTOR);
		input[1].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[1].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
		input[1].mi.time=0;
		input[1].mi.dwExtraInfo=0;

		// Имитирует нажатие и отпускание правой кнопки мыши
		//SendInput(4,input,sizeof(INPUT));
		skip_mouse_hook=true;
		SendInput(2,&input[0],sizeof(INPUT));
		skip_mouse_hook=false;


	}
	else
	{
		drag_in_progress=false;

		// 3. Сначала подвинем курсор
		input[2].type=INPUT_MOUSE;
		input[2].mi.dx=(LONG)(p.x*XSCALEFACTOR);
		input[2].mi.dy=(LONG)(p.y*YSCALEFACTOR);
		input[2].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[2].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
		input[2].mi.time=0;
		input[2].mi.dwExtraInfo=0;

	
		// 4. отпускание левой кнопки
		input[3].type=INPUT_MOUSE;
		input[3].mi.dx=(LONG)(p.x*XSCALEFACTOR);
		input[3].mi.dy=(LONG)(p.y*YSCALEFACTOR);
		input[3].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[3].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP;
		input[3].mi.time=0;
		input[3].mi.dwExtraInfo=0;

		// Имитирует нажатие и отпускание правой кнопки мыши
		//SendInput(4,input,sizeof(INPUT));
		skip_mouse_hook=true;
		SendInput(2,&input[2],sizeof(INPUT));
		skip_mouse_hook=false;
	}

	return drag_in_progress;

}

//==============================================================================================
// Имитирует начало и конец дрега (старая версия)
//==============================================================================================
bool Fixation::Drag2(POINT p)
{
	static POINT p_initial;

	// Содрано из интернета
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	// Из-за того, что не обнулял, были страшные глюки. Очень странно...
	// разобрался. SendInput вызывался даже на первом шаге, вот и все глюки...
	INPUT input[4]={0};

	if(!drag_in_progress) // Только нажимаем
	{
		drag_in_progress=true;
		// просто запоминаем исходную позицию
		p_initial=p;
	}
	else
	{
		drag_in_progress=false;

		// 1. Сначала подвинем курсор
		input[0].type=INPUT_MOUSE;
		input[0].mi.dx=(LONG)(p_initial.x*XSCALEFACTOR);
		input[0].mi.dy=(LONG)(p_initial.y*YSCALEFACTOR);
		input[0].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[0].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
		input[0].mi.time=0;
		input[0].mi.dwExtraInfo=0;

		// 2. нажатие левой кнопки
		input[1].type=INPUT_MOUSE;
		input[1].mi.dx=(LONG)(p_initial.x*XSCALEFACTOR);
		input[1].mi.dy=(LONG)(p_initial.y*YSCALEFACTOR);
		input[1].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[1].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
		input[1].mi.time=0;
		input[1].mi.dwExtraInfo=0;
		
		// 3. Сначала подвинем курсор
		input[2].type=INPUT_MOUSE;
		input[2].mi.dx=(LONG)(p.x*XSCALEFACTOR);
		input[2].mi.dy=(LONG)(p.y*YSCALEFACTOR);
		input[2].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[2].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
		input[2].mi.time=0;
		input[2].mi.dwExtraInfo=0;

	
		// 4. отпускание левой кнопки
		input[3].type=INPUT_MOUSE;
		input[3].mi.dx=(LONG)(p.x*XSCALEFACTOR);
		input[3].mi.dy=(LONG)(p.y*YSCALEFACTOR);
		input[3].mi.mouseData=0; // Нужно для всяких колёс прокрутки 
		input[3].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP;
		input[3].mi.time=0;
		input[3].mi.dwExtraInfo=0;

		// Имитирует нажатие и отпускание правой кнопки мыши
		//SendInput(4,input,sizeof(INPUT));
		skip_mouse_hook=true;
		SendInput(2,&input[0],sizeof(INPUT));
		Sleep(80);
		SendInput(2,&input[2],sizeof(INPUT));
		skip_mouse_hook=false;
	}



	return drag_in_progress;

}
//==============================================================================================
// Скролл на величину, пропорциональную времени в сторону direction
//==============================================================================================
void Fixation::Scroll(uint64_t timelag, int direction)
{
	// Для отладки и понимания пределов timelag
			//char msgbuf[1024];
			//sprintf(msgbuf,"%llu\n",timelag);
			//OutputDebugString(msgbuf); 

	if(timelag>100000UL) timelag=100000UL;

	INPUT input={0};

	input.type=INPUT_MOUSE;
	input.mi.dx=0L;
	input.mi.dy=0L;
	input.mi.mouseData=direction*(timelag/2000UL); // Скролл на величину, пропорциональную времени
	//input.mi.mouseData=direction*3; // для пробы
	input.mi.dwFlags=MOUSEEVENTF_WHEEL;
	input.mi.time=0;
	input.mi.dwExtraInfo=0;


	SendInput(1,&input,sizeof(INPUT));
}

//===========================================================================
// спец-клавиши Ctrl, Shift, Alt  до (press=true) и после (press=false) клика
//===========================================================================
void Fixation::ClickModifiers(bool press)
{
	INPUT input={0};
	input.type=INPUT_KEYBOARD;
	if(!press) input.ki.dwFlags = KEYEVENTF_KEYUP ;

	// Спец-клавиши
	if(BKBToolWnd::tool_modifier[1]) // 1 - это + Shift
	{
		input.ki.wVk=VK_SHIFT;
		SendInput(1,&input,sizeof(INPUT));		
	}
	if(BKBToolWnd::tool_modifier[0]) // 0 - это + Ctrl
	{
		input.ki.wVk=VK_CONTROL;
		SendInput(1,&input,sizeof(INPUT));		
	}
	if(BKBToolWnd::tool_modifier[2]) // 2 - это + Alt
	{
		input.ki.wVk=VK_MENU;
		SendInput(1,&input,sizeof(INPUT));		
	}

}