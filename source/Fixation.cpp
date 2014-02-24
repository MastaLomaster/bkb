#include <Windows.h>
#include <stdint.h> // Это для uint64_t
#include <stdio.h> // Это для отладки 
#include "Fixation.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "KeybWnd.h"

static char debug_buf[4096];

BKB_MODE Fixation::BKB_Mode=BKB_MODE_NONE;

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
	case BKB_MODE_RCLICK: // Щелчок правой кнопкой мыши
	case BKB_MODE_DOUBLECLICK: // Двойной щелчок
	case BKB_MODE_DRAG: // Ну, дрег

		// Если окно уже показано, получить координаты на экране в этом окне
		// Переключение режима с открытым окном Magnify невозможно
		if(BKBMagnifyWnd::IsVisible())
		{
			if(BKBMagnifyWnd::FixPoint(&p)) // попали в окно, координаты точки уточнены при увеличении
			{
				switch (BKB_Mode) // О! Опять!
				{
					case BKB_MODE_LCLICK: // Щелчок левой кнопкой мыши
						LeftClick(p);
						BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то
						break;

					case BKB_MODE_RCLICK: // Щелчок правой кнопкой мыши
						RightClick(p);
						BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то
						break;

					case BKB_MODE_DOUBLECLICK: // Двойной щелчок
						DoubleClick(p);
						BKBToolWnd::Reset(&BKB_Mode); // один раз только ловим клик-то
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
		if(!BKBKeybWnd::IsItYours(&p))
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
void Fixation::LeftClick(POINT p)
{
	// Содрано из интернета
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	int xs,ys,x,y;
	
	xs=XSCALEFACTOR;
	ys=YSCALEFACTOR;
	x=p.x;
	y=p.y;
	//screenx=GetSystemMetrics(SM_CXSCREEN);
	sprintf(debug_buf,"xs:%d ys:%d x:%d y:%d",xs,ys,x,y);
	//MessageBox(NULL,debug_buf,"debug",MB_OK);

	INPUT input[3];

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
	SendInput(3,input,sizeof(INPUT));
}

//==============================================================================================
// Имитирует нажатие и отпускание правой кнопки мыши
//==============================================================================================
void Fixation::RightClick(POINT p)
{
	// Содрано из интернета
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	INPUT input[3];

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
	SendInput(3,input,sizeof(INPUT));
}

//==============================================================================================
// Имитирует сами знаете что
//==============================================================================================
void Fixation::DoubleClick(POINT p)
{
	LeftClick(p);
	Sleep(80);
	LeftClick(p);
}


//==============================================================================================
// Имитирует начало и конец дрега
//==============================================================================================
bool Fixation::Drag(POINT p)
{
	static bool drag_in_progress=false;
	static POINT p_initial;

	// Содрано из интернета
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	INPUT input[4];

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
	}

	// Имитирует нажатие и отпускание правой кнопки мыши
	SendInput(4,input,sizeof(INPUT));
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

	INPUT input;

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