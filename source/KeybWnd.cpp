#include <Windows.h>
#include "KeybWnd.h"
#include "BKBRepErr.h"

#define WM_USER_INVALRECT (WM_USER + 100)

extern int flag_using_airmouse;

// Раскладка клавиатур задана в файле KeybLayouts.cpp
extern BKB_key BKBKeybLayouts [3][3][BKB_KBD_NUM_CELLS];

extern HINSTANCE BKBInst;
static const char *wnd_class_name="BKBKeyb";

HWND  BKBKeybWnd::Kbhwnd;
int BKBKeybWnd::current_pane=0;
float BKBKeybWnd::cell_size=0;
int BKBKeybWnd::screen_x, BKBKeybWnd::screen_y, BKBKeybWnd::start_y;
POINT BKBKeybWnd::start_point;
bool BKBKeybWnd::fixation_approved=false;
int BKBKeybWnd::row, BKBKeybWnd::column, BKBKeybWnd::screen_num=0, BKBKeybWnd::percentage;
bool BKBKeybWnd::shift_pressed=false, BKBKeybWnd::ctrl_pressed=false,
	BKBKeybWnd::alt_pressed=false,BKBKeybWnd::caps_lock_pressed=false,
	BKBKeybWnd::Fn_pressed=false;

HDC BKBKeybWnd::memdc1=0, BKBKeybWnd::memdc2=0, BKBKeybWnd::whitespot_dc=0;
HBITMAP BKBKeybWnd::hbm1=0, BKBKeybWnd::hbm2=0, BKBKeybWnd::whitespot_bitmap=0;

volatile LONG BKBKeybWnd::redraw_state=0;
int BKBKeybWnd::width, BKBKeybWnd::height;
POINT BKBKeybWnd::whitespot_point={-100,-100};

int BKBKeybWnd::row_pressed=-1, BKBKeybWnd::column_pressed=-1;

extern HBRUSH dkblue_brush, blue_brush;
extern HFONT hfont;


// Оконная процедура 
LRESULT CALLBACK BKBKeybWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	HDC hdc;

	switch (message)
	{

	case WM_USER_INVALRECT: // Это приходит из другого потока
		InvalidateRect(hwnd,NULL,TRUE);
		break;

	case WM_TIMER: // Кнопку можно отпустить
		BKBKeybWnd::OnTimer();
		break;

	case WM_SETCURSOR:
		// Теперь прячем курсор над окном, его роль выполняет белое пятно
		// И одновременно показываем 
		SetCursor(NULL);
		break;

	case WM_CREATE:
		// Создаём белое пятно
		BKBKeybWnd::CreateWhiteSpot(hwnd);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		hdc=BeginPaint(hwnd,&ps);
		BKBKeybWnd::OnPaint(hdc);
		EndPaint(hwnd,&ps);
		break;

	case WM_SIZE:
		BKBKeybWnd::OnSize(hwnd, LOWORD(lparam), HIWORD(lparam));
		break;

		// !! Добавить сюда чистку всех memdc при выходе (WM_CLOSE) !!
	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Прекращаем рисовать нажатую кнопку
//================================================================
void BKBKeybWnd::OnTimer()
{
	//OutputDebugString("timer\n");
		KillTimer(Kbhwnd,4);
		row_pressed=-1; column_pressed=-1;
		redraw_state=0;
		InvalidateRect(Kbhwnd,NULL,TRUE); // Это единственное место, где InvalidateRect может быть вызван, так как поток - свой.
}

//================================================================
// Инициализация 
//================================================================
void BKBKeybWnd::Init()
{
	ATOM aresult; // Для всяких кодов возврата
	
	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBKeybWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
		NULL,
        //LoadCursor(NULL, IDC_ARROW), 
		// Фон красится теперь в memdc1
        //dkblue_brush,
		0,
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
	cell_size=screen_x/(float)BKB_KBD_NUM_CELLS;

	start_y=screen_y-(int)(cell_size*3);

	Kbhwnd=CreateWindowEx(
	WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	TEXT(wnd_class_name),
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	0,start_y,screen_x,screen_x-start_y, 
    0, 0, BKBInst, 0L );

	if(NULL==Kbhwnd)
	{
		BKBReportError(__FILE__,"CreateWindow",__LINE__);
	}

	// ShowWindow(Kbhwnd,SW_SHOWNORMAL);
}

//================================================================
// Рисуем окно (Из WM_PAINT или сами)
//================================================================
void BKBKeybWnd::OnPaint(HDC hdc)
{
	bool release_dc=false;
	LONG local_redraw_state;

	// Сейчас будем АТОМАРНО считывать и сбрасывать redraw_state;
	local_redraw_state=InterlockedCompareExchange(&redraw_state,2,0);
	if(0!=local_redraw_state) // Замены не произошло, пробуем проверить 1
	{
		local_redraw_state=InterlockedCompareExchange(&redraw_state,2,1);
		// Если и тут ничего не вышло, ничего страшного
		// Либо там была двойка, либо значение успело поменяться между двумя InterlockedCompareExchange
		// Если успело поменяться, то придёт ещё один WM_PAINT, и мы его всё равно отловим
		// Просто лишний раз выполним BitBlt в шаге 2. Никто не пострадает.
		// Вообще-то 0 на 1 поменяться не может в принципе. Потому что тоже через Interlocked... единица выставляется
		// Короче, всё работает.
		if(1!=local_redraw_state) local_redraw_state=2;
	}
	

	if(0==hdc)
	{
		release_dc=true;
		hdc=GetDC(Kbhwnd);
	}

	HFONT old_font;

	SetTextColor(memdc1,RGB(255,255,255));
	SetBkColor(memdc1,RGB(45,62,90));
	//SetBkMode(memdc1,TRANSPARENT);

	RECT fill_r={0,0,width,height};
	RECT rect_pressed={int(column_pressed*cell_size),int(row_pressed*cell_size),int((column_pressed+1)*cell_size),int((row_pressed+1)*cell_size)};

	/*
	switch(local_redraw_state)
	{
	case 0: // Перерисовываем всё с самой глубины
		OutputDebugString("redraw 0\n");
		break;
	case 1:
		OutputDebugString("redraw 1\n");
		break;
	case 2:
		OutputDebugString("redraw 2\n");
		break;
	default:
		OutputDebugString("redraw XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		break;
	}

	*/
	

	switch(local_redraw_state)
	{
	case 0: // Перерисовываем всё с самой глубины
		FillRect(memdc1,&fill_r,dkblue_brush);
		// Подсвечиваем нажатую клавишу
		if(column_pressed!=-1)
		{
			//RECT rect_pressed={int(column_pressed*cell_size),int(row_pressed*cell_size),int((column_pressed+1)*cell_size),int((row_pressed+1)*cell_size)};
			FillRect(memdc1,&rect_pressed,blue_brush);
		}

		// Собственно, рисование
		// 1. Сначала подсветим нажатые  спец. клавиши
		if(shift_pressed)
		{
			RECT rect={0,int(2*cell_size),int(cell_size),int(3*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
			if(caps_lock_pressed)	TextOut(memdc1,int(cell_size/3), int(cell_size*2.8),"CAPS",4);
		}
		if((ctrl_pressed)&&(screen_num>0))
		{
			RECT rect={int(14*cell_size),int(2*cell_size),int(15*cell_size),int(3*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if((alt_pressed)&&(screen_num>0))
		{
			RECT rect={int(14*cell_size),int(cell_size),int(15*cell_size),int(2*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if((Fn_pressed)&&(2==screen_num))
		{
			RECT rect={0,int(cell_size),int(cell_size),int(2*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
			// Подчеркнём клавиши, которые будут функциональными (F1-F12)
			SelectObject(memdc1,GetStockObject(WHITE_PEN));
			MoveToEx(memdc1,int(cell_size)+2,int(2*cell_size-4),NULL);
			LineTo(memdc1,int(12*cell_size-3),int(2*cell_size-4));
		}

		// 2. Расчерчиваем линии
		SelectObject(memdc1,GetStockObject(WHITE_PEN));
		// 2.1. Горизонтальные
		MoveToEx(memdc1,0,int(cell_size),NULL);
		LineTo(memdc1,screen_x-1,int(cell_size));
		MoveToEx(memdc1,0,int(2*cell_size),NULL);
		LineTo(memdc1,screen_x-1,int(2*cell_size));

		// 2.2. Вертикальные
		int i,j;
		for(i=1;i<BKB_KBD_NUM_CELLS;i++)
		{
			MoveToEx(memdc1,int(cell_size*i),0,NULL);
			LineTo(memdc1,int(cell_size*i),int(cell_size*3));
			
			//TextOut(hdc,35,60+i*screen_y/BKB_NUM_TOOLS,tool_names[i],(int)strlen(tool_names[i]));
		}

		// 3. Пишем буквы
		// 3.1. Системным фонтом - то, что длиннее одного символа
		for(j=0;j<3;j++)
			for(i=0;i<BKB_KBD_NUM_CELLS;i++)
			{
				if(NULL!=BKBKeybLayouts[screen_num][j][i].label) // проверка, что там не NULL
				if(strlen(BKBKeybLayouts[screen_num][j][i].label)>1)
				TextOut(memdc1,int(cell_size*0.4+i*cell_size) , int(cell_size/3+j*cell_size),
					BKBKeybLayouts[screen_num][j][i].label,strlen(BKBKeybLayouts[screen_num][j][i].label));
			}
				
		// 3.2. Своим фонтом - из нескольких символов
		old_font=(HFONT)SelectObject(memdc1, hfont);

		for(j=0;j<3;j++)
			for(i=0;i<BKB_KBD_NUM_CELLS;i++)
			{
				if(NULL!=BKBKeybLayouts[screen_num][j][i].label) // проверка, что там не NULL
				if(1==strlen(BKBKeybLayouts[screen_num][j][i].label))
				TextOut(memdc1,int(cell_size*0.4+i*cell_size) , int(cell_size/3+j*cell_size),
					BKBKeybLayouts[screen_num][j][i].label,1);
			}
	
		// Возвращаем старый фонт
		SelectObject(memdc1, old_font);

		// ЗДЕСЬ НЕ НУЖЕН break !!! После нулевого шага всегда идёт первый!!!

	case 1:
		BitBlt(memdc2,0,0,width,height,memdc1,0,0,SRCCOPY);

		// Возможно, рисовать Progress Bar
		if(fixation_approved)
		{
			RECT rect;
		
			rect.left=(LONG)(cell_size/20+column*cell_size);
			rect.right=(LONG)(rect.left+percentage*cell_size*90/100/100);
			rect.top=(LONG)(cell_size/20+cell_size*row);
			rect.bottom=(LONG)(rect.top+cell_size/20); 

			FillRect(memdc2,&rect,blue_brush);
		}

		// Теперь рисуем белое пятно
		BLENDFUNCTION bfn;
		bfn.BlendOp = AC_SRC_OVER;
		bfn.BlendFlags = 0;
		bfn.SourceConstantAlpha = 255;
		bfn.AlphaFormat = AC_SRC_ALPHA;

		//AlphaBlend(memdc2, 0, 0, 100, 64, BufferDC, 0, 0, 100, 64, bfn);
		AlphaBlend(memdc2,whitespot_point.x-50,whitespot_point.y-50,100,100,whitespot_dc,0,0,100,100,bfn);

		// ЗДЕСЬ НЕ НУЖЕН break !!! После первого шага всегда идёт второй!!!
	case 2:
		BitBlt(hdc,0,0,width,height,memdc2,0,0,SRCCOPY);
		break;
	}

	// Если брал DC - верни его
	if(release_dc) ReleaseDC(Kbhwnd,hdc);

}


//===========================================================================
// Похоже, что хотят нажать на кнопку, рисовать прогресс нажатия
//===========================================================================
bool BKBKeybWnd::ProgressBar(POINT *p, int fixation_count, int _percentage)
{
	percentage=_percentage;

	if(1==fixation_count) // начало фиксации, проверим и запомним эту точку
	{
		start_point=*p;
		if(start_point.y>=start_y) 
		{
			fixation_approved=true;

			// загоняем в границы экрана
			if(start_point.y>=screen_y) start_point.y=screen_y-2; // минус два на всякий случай, чтобы не дала слишком большие row/column 
			if(start_point.x<0) start_point.x=0;
			if(start_point.x>=screen_x) start_point.x=screen_x-2;

			row=(int)((start_point.y-start_y)/cell_size);
			column=(int)(start_point.x/cell_size);
		}
		else fixation_approved=false;
	}
	else // Это продолжение фиксации. 
	{
		if(fixation_approved) 
		//if((2==flag_using_airmouse)&&fixation_approved) // Фиксация была наша. В случае с аэромышью (особенно) недопустимо, 
		// чтобы курсор вышел за пределы клавиши, с которой началась фиксация
		// Для остальных режимов попробуем так же
		{
			POINT p_tmp=*p;
			int row_tmp=(int)((p_tmp.y-start_y)/cell_size);
			int column_tmp=(int)(p_tmp.x/cell_size);

			if((row_tmp!=row)||(column_tmp!=column)) return false;
		}
	}


	// Рисуем прогресс-бар на кнопке
	if(fixation_approved)
	{
		// Теперь это всё в WM_PAINT
		// Атомарно меняем 2 на 1
		LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);
		
	/*	if(redraw_state>1) 
		{
			redraw_state=1; //Здесь был баг. Могло перебить перерисовку с нижнего слоя
			//OutputDebugString("rstate->1 PB\n");
		}	 */

		// Из другого потока вроде нельзя вызывать InvalidateRect
		// Делаем перерисовку, только если старое состояние было двойкой
		if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
		//InvalidateRect(Kbhwnd,NULL,TRUE);
	}

	return true;
}

//===========================================================================
// Не удержали взгляд, или переключаемся на другой режим
//===========================================================================
void BKBKeybWnd::ProgressBarReset()
{
	fixation_approved=false;
	// Атомарно меняем 2 на 1
	LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);

	/* if(redraw_state>1) 
	{
		redraw_state=1; //Здесь был баг. Могло перебить перерисовку с нижнего слоя
		OutputDebugString("rstate->1 PBreset\n");
	} */
	// Из другого потока нельзя вызывать InvalidateRect
	// Делаем перерисовку, только если старое состояние было двойкой
	if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
	//InvalidateRect(Kbhwnd,NULL,TRUE);
}

//===========================================================================
// Произошла фиксация, возможно на клавиатуре
//===========================================================================
bool BKBKeybWnd::IsItYours(POINT *p)
{
	INPUT input[2]={0},shift_input={0},ctrl_input={0},alt_input={0};

	if(fixation_approved)
	{
		// Сбросить Progress Bar
		ProgressBarReset();
		fixation_approved=false;

		// Реагируем на нажатую кнопку
		// Определим нажатую кнопку
		BKB_key key_pressed=BKBKeybLayouts[screen_num][row][column];
		if((Fn_pressed)&&(2==screen_num)&&(1==row)&&(0<column)&&(12>=column)) // Жмём одну из 12 функциональных клавиш
		{
			ScanCodeButton(VK_F1+column-1);
			Fn_pressed=false; // Сбрасываем нажатую клавишу Fn
		}
		else // Это НЕ функциональная клавиша
		switch (key_pressed.bkb_keytype)
		{
		case unicode: // Самое простое
					
			// Нажатие кнопки
			input[0].type=INPUT_KEYBOARD;
			input[0].ki.dwFlags =  KEYEVENTF_UNICODE;
		
			if(shift_pressed) input[0].ki.wScan=key_pressed.bkb_unicode_uppercase;
			else input[0].ki.wScan=key_pressed.bkb_unicode;

			// Отпускание кнопки
			input[1].type=INPUT_KEYBOARD;
			input[1].ki.dwFlags = KEYEVENTF_KEYUP |  KEYEVENTF_UNICODE;
			if(shift_pressed) input[1].ki.wScan=key_pressed.bkb_unicode_uppercase;
			else input[1].ki.wScan=key_pressed.bkb_unicode;
			
			SendInput(2,input,sizeof(INPUT));

			if((shift_pressed)&&(!caps_lock_pressed))  // Сбрасываем shift, если он не ужерживается caps_lock'ом
			{
				shift_pressed=false;
			}
			break;

		case scancode: // Потом здесь нужно обрабатывать Alt,Shift,Ctrl; а пока - так
			ScanCodeButton(key_pressed.bkb_vscancode);
			break;

		case leftkbd: // другой экран клавиатуры (слева от текущего)
			screen_num--;
			if(screen_num<0) screen_num=2;
			break;

		case rightkbd: // другой экран клавиатуры (справа от текущего)
			screen_num++;
			if(screen_num>2) screen_num=0;
			break;

		case shift: // нажали кнопку Shift
			if(shift_pressed) 
			{
				if(caps_lock_pressed) // Это сброс caps_lock
				{
					shift_pressed=false;
					caps_lock_pressed=false;
				}
				else // Повторное нажатие на Shift взводит CapsLock
				{
					caps_lock_pressed=true;
				}
			}
			else shift_pressed=true; // просто взводим shift
			break;

		case control: // нажали кнопку Ctrl
			if(ctrl_pressed) ctrl_pressed=false; else ctrl_pressed=true;
			break;

		case alt: // нажали кнопку Alt
			if(alt_pressed) alt_pressed=false; else alt_pressed=true;
			break;

		case fn: // нажали кнопку Fn
			if(Fn_pressed) Fn_pressed=false; else Fn_pressed=true;
			break;
		}
		// Пока здесь, потом, возможно, вынесем в отдельную функцию

		// запомним, что стереть после окончания таймера
		row_pressed=row; column_pressed=column;
		SetTimer(Kbhwnd,4,500,0); // полсекунды
		// Теперь ВСЕГДА перерисовываем клавиатуру после ЛЮБОГО нажатия
		redraw_state=0;
		// Из другого потока нельзя вызывать InvalidateRect
		PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
		//InvalidateRect(Kbhwnd,NULL,FALSE); // перерисовать клавиатуру с самого начала
		//OutputDebugString("row_pressed\n");
	} //fixation approved

	// Возвращаем всегда, относится ли это нажатие к нам, даже если кнопку не нажали
	if(p->y>=start_y) 	return true;
	else return false;
}

//===========================================================================
// Подсветить пятном место взгляда
//===========================================================================
void BKBKeybWnd::WhiteSpot(POINT *p)
{
	// Не, всё не так. Надо проверить, может надо стереть пятно, которое было раньше
	//if(p->y<start_y-50) return; // Белое пятно не заехало на клавиатуру

	whitespot_point=*p;
	whitespot_point.y-=start_y; // Такой простой перевод экранных координат в оконные

	LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);

	// Из другого потока нельзя вызывать InvalidateRect
	// Делаем перерисовку, только если старое состояние было двойкой
	if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
/*	if(redraw_state>1) 
	{
		redraw_state=1; //Здесь был баг. Могло перебить перерисовку с нижнего слоя
		OutputDebugString("rstate->1 WS\n");
	} 
		// Из другого потока нельзя вызывать InvalidateRect
	PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
	//InvalidateRect(Kbhwnd,NULL,TRUE); // перерисовать клавиатуру */
}




//===========================================================================
// Активация клавиатуры
//===========================================================================
void BKBKeybWnd::Activate()
{
	ShowWindow(Kbhwnd,SW_SHOWNORMAL);
	fixation_approved=false;
}

//===========================================================================
// Деактивация клавиатуры
//===========================================================================
void BKBKeybWnd::DeActivate()
{
	ShowWindow(Kbhwnd,SW_HIDE);
	ProgressBarReset();
}

//===========================================================================
// Обрабатывает нажатие кнопки, заданной сканкодом, а не юникодом
// Здесь требуется дополнительный контроль спец-клавиш Shift,Alt,Ctrl
//===========================================================================
void BKBKeybWnd::ScanCodeButton(WORD scancode)
{
	bool redraw_reqired=false;

	INPUT input={0};
	input.type=INPUT_KEYBOARD;

	// Спец-клавиши
	if(shift_pressed)
	{
		input.ki.dwFlags =  0;
		input.ki.wVk=VK_SHIFT;
		SendInput(1,&input,sizeof(INPUT));		
	}
	if(ctrl_pressed)
	{
		input.ki.dwFlags =  0;
		input.ki.wVk=VK_CONTROL;
		SendInput(1,&input,sizeof(INPUT));		
	}
	if(alt_pressed)
	{
		input.ki.dwFlags =  0;
		input.ki.wVk=VK_MENU;
		SendInput(1,&input,sizeof(INPUT));		
	}

	// Сама нажатая кнопка
	// Нажатие кнопки
	input.ki.dwFlags =  0;
	input.ki.wVk=scancode;
	SendInput(1,&input,sizeof(INPUT));		
	// Отпускание кнопки
	input.ki.dwFlags = KEYEVENTF_KEYUP ;
	SendInput(1,&input,sizeof(INPUT));		
	
	// Спец-клавиши
	if(shift_pressed)
	{
		// Отпускаем кнопку Shift
		input.ki.wVk=VK_SHIFT;
		input.ki.dwFlags = KEYEVENTF_KEYUP ;
		SendInput(1,&input,sizeof(INPUT));

		// Сохраняем ли shift в работе?
		if((shift_pressed)&&(!caps_lock_pressed))  // Сбрасываем shift, если он не ужерживается caps_lock'ом
		{
			shift_pressed=false;
			redraw_reqired=true;
		}
	}
	if(ctrl_pressed)
	{
		// Отпускаем кнопку Ctrl
		input.ki.wVk=VK_CONTROL;
		input.ki.dwFlags = KEYEVENTF_KEYUP ;
		SendInput(1,&input,sizeof(INPUT));

		redraw_reqired=true;
		ctrl_pressed=false; // отпускаем Ctrl
	}
	if(alt_pressed)
	{
		// Отпускаем кнопку Alt
		input.ki.wVk=VK_MENU;
		input.ki.dwFlags = KEYEVENTF_KEYUP ;
		SendInput(1,&input,sizeof(INPUT));

		redraw_reqired=true;
		alt_pressed=false; // отпускаем Alt
	}
	
	//if(redraw_reqired) InvalidateRect(Kbhwnd,NULL,TRUE); // перерисовать клавиатуру
}

//===================================================================
// Меняем размеры окна (фактически, один раз при создании окна)
// Пересоздает все memdc и hbm
//===================================================================
void BKBKeybWnd::OnSize(HWND hwnd, int _width, int _height)
{
	HDC hdc=GetDC(hwnd);

	width=_width;
	height=_height;

	// Убираем DC
	if(memdc1!=0) DeleteDC(memdc1);
	if(memdc2!=0) DeleteDC(memdc2);
	
	// Убираем битмапы
	if(hbm1!=0) DeleteObject(hbm1);
	if(hbm2!=0) DeleteObject(hbm2);
	
	// Воссоздаём DC и битмапы
	memdc1=CreateCompatibleDC(hdc);
	memdc2=CreateCompatibleDC(hdc);

	hbm1=CreateCompatibleBitmap(hdc,width,height);
	hbm2=CreateCompatibleBitmap(hdc,width,height);

	SelectObject(memdc1,hbm1);
	SelectObject(memdc2,hbm2);

	ReleaseDC(hwnd,hdc);

	redraw_state=0; // Требуется пересовка всех слоёв
}

//===================================================================
// Создаём битмап с пятном, как у фонарика
//===================================================================
void BKBKeybWnd::CreateWhiteSpot(HWND hwnd)
{
	HDC hdc=GetDC(hwnd);
	whitespot_dc=CreateCompatibleDC(hdc);
	whitespot_bitmap=CreateCompatibleBitmap(hdc,100,100);

	// Создаём битмап с альфа-каналом

	BITMAPINFO BufferInfo;
	ZeroMemory(&BufferInfo,sizeof(BITMAPINFO));
	BufferInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BufferInfo.bmiHeader.biWidth = 100;
	BufferInfo.bmiHeader.biHeight = 100;
	BufferInfo.bmiHeader.biPlanes = 1;
	BufferInfo.bmiHeader.biBitCount = 32;
	BufferInfo.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* pArgb;

	whitespot_bitmap = CreateDIBSection(whitespot_dc, &BufferInfo, DIB_RGB_COLORS,
                                       (void**)&pArgb, NULL, 0);

	// Заполяем альфа-канал
	int i,j,alpha,distance_squared;
	for(i=0;i<100;i++)
	{
		for(j=0;j<100;j++)
		{
			// Чем дальше от центра, тем прозрачнее (в квадрате)
			distance_squared=(i-50)*(i-50)+(j-50)*(j-50);
			//alpha=255-distance_squared/10; if(alpha<0) alpha=0; 
			alpha=100-distance_squared/24; if(alpha<0) alpha=0; 
			pArgb[i*100+j].rgbBlue=alpha;
			pArgb[i*100+j].rgbGreen=alpha;
			pArgb[i*100+j].rgbRed=alpha;
			pArgb[i*100+j].rgbReserved = alpha;
		}
	}
		
	//============================

	SelectObject(whitespot_dc,whitespot_bitmap);

	SetBkMode(whitespot_dc,TRANSPARENT);
	//TextOut(whitespot_dc,20,20,"KKK",3);
	
}


