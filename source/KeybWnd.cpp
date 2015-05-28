#include <Windows.h>
#include "KeybWnd.h"
#include "BKBRepErr.h"
#include "Click.h"
#include "WM_USER_messages.h"
#include "BKBSettings.h"
#include "ToolWnd.h"
#include "TranspWnd.h"
#include "BKBProgressWnd.h"

static bool flag_hide_anim=false; // Прятать окно анимации только поcле перерисовки клавиатуры (с запозданием, чтобы не было flickers)

int gBKB_FullSizeKBD = 0; // Флаг того, что клавиатура занимает всю ширину экрана

extern int gBKB_TOOLBOX_WIDTH; 
extern int tracking_device;
extern int screenX, screenY; // Определены в BKBgdi

// Раскладка клавиатур задана в файле KeybLayouts.cpp

extern HINSTANCE BKBInst;
static const TCHAR *wnd_class_name=L"BKBKeyb";
static const TCHAR *anim_wnd_class_name=L"BKBAnimKeyb";

HWND  BKBKeybWnd::Kbhwnd=0, BKBKeybWnd::AnimHwnd;
int BKBKeybWnd::current_pane=0;
POINT BKBKeybWnd::start_point, BKBKeybWnd::place_point={0,0};
bool BKBKeybWnd::fixation_approved=false;
int BKBKeybWnd::row, BKBKeybWnd::column, BKBKeybWnd::percentage;
bool BKBKeybWnd::shift_pressed=false, BKBKeybWnd::ctrl_pressed=false,
	BKBKeybWnd::alt_pressed=false,BKBKeybWnd::caps_lock_pressed=false,
	BKBKeybWnd::Fn_pressed=false;
float BKBKeybWnd::cell_width, BKBKeybWnd::cell_height, BKBKeybWnd::cropped_cell_width;
bool BKBKeybWnd::bottom_side=true;
int BKBKeybWnd::ctrl_row,BKBKeybWnd::ctrl_column,BKBKeybWnd::alt_row,BKBKeybWnd::alt_column,BKBKeybWnd::shift_row,BKBKeybWnd::shift_column,
	BKBKeybWnd::fn_row,BKBKeybWnd::fn_column;

HDC BKBKeybWnd::memdc1=0, BKBKeybWnd::memdc2=0, BKBKeybWnd::whitespot_dc=0;
HBITMAP BKBKeybWnd::hbm1=0, BKBKeybWnd::hbm2=0, BKBKeybWnd::whitespot_bitmap=0;

volatile LONG BKBKeybWnd::redraw_state=0;
int BKBKeybWnd::width, BKBKeybWnd::height;
POINT BKBKeybWnd::whitespot_point={-100,-100};

int BKBKeybWnd::row_pressed=-1, BKBKeybWnd::column_pressed=-1;

RECT BKBKeybWnd::pink_rect;
int BKBKeybWnd::step=0;
LONG  BKBKeybWnd::anim_width=50,  BKBKeybWnd::anim_height=50;

extern HBRUSH dkblue_brush, blue_brush;
extern HFONT hfont;

extern bool gBKB_2STEP_KBD_MODE;

#ifdef BELYAKOV
// раскладки клавиатуры
LPCTSTR kbd_usenglish=L"00000409", kbd_russian=L"00000419"; 
HKL hkl_usenglish=0, hkl_russian=0;
#endif


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
#ifdef BELYAKOV
		if(2!=tracking_device)
#endif
		SetCursor(NULL);
		break;

	case WM_CREATE:
		// Создаём белое пятно
		BKBKeybWnd::CreateWhiteSpot(hwnd);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		hdc=BeginPaint(hwnd,&ps);
		if(0==BKBKeybWnd::step)
			BKBKeybWnd::OnPaintStep0(hdc);
		else 
			BKBKeybWnd::OnPaintStep1(hdc);

		EndPaint(hwnd,&ps);
		break;

	case WM_USER_KBD_TOPDOWN:
		BKBKeybWnd::OnTopDown();
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

//=====================================================
// Оконная процедура для окна анимации
//=====================================================
LRESULT CALLBACK BKBAnimKeybWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	HDC hdc;
	RECT tmpRECT={10,10,50,50};

	switch (message)
	{

	//case WM_SIZE:
	case WM_MOVE:
		hdc=GetDC(hwnd);
	//FillRect(hdc,&tmpRECT,blue_brush);
	BKBKeybWnd::OnAnimPaint(hdc);
		ReleaseDC(hwnd,hdc);
		break;
/*
		
		

	case WM_WINDOWPOSCHANGED:
		// так мы убиваем WM_SIZE и WM_MOVE
		break;
*/
	case WM_PAINT:
		PAINTSTRUCT ps;
		hdc=BeginPaint(hwnd,&ps);
		
		
		

		//MoveToEx(hdc,10,10,NULL);
		//LineTo(hdc,50,50);

		//BKBKeybWnd::OnAnimPaint(hdc);

		EndPaint(hwnd,&ps);
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
		// (Так было раньше, теперь всё из своего потока)
}

//================================================================
// Инициализация 
//================================================================
void BKBKeybWnd::Init(HWND master_hwnd)
{
	ATOM aresult; // Для всяких кодов возврата
	
	
	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBKeybWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
#ifdef BELYAKOV
        LoadCursor(NULL, IDC_ARROW), 
#else
		NULL,
#endif
		// Фон красится теперь в memdc1
        // dkblue_brush,
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

#ifdef BELYAKOV
	// приготовим заранее идентификаторы клавиатур
	hkl_usenglish=LoadKeyboardLayout(kbd_usenglish, 0);
	hkl_russian=LoadKeyboardLayout(kbd_russian, 0);
#endif

	// Теперь клавиатура может занимать не всю ширину экрана
	if(gBKB_FullSizeKBD) width=screenX;
	else width=screenX-gBKB_TOOLBOX_WIDTH;

	cell_width=width/(float)columns;


	//start_y=screen_y-(int)(cell_size*3);
	if(cell_width*rows<screenY*0.45f) cell_height=cell_width; // Удалось уложиться в 0.45 высоты экрана при квадратных кнопках
	else cell_height=0.45f*screenY/rows; // Приплюснем кнопки, чтобы не перекрыть более 0.45 экрана

	Kbhwnd=CreateWindowEx(
	WS_EX_TOPMOST,
	//WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	//0,screen_y-1-(INT)(cell_height*rows),screen_x,(INT)(cell_height*rows), 
	0,screenY-1-(INT)(cell_height*rows),width,(INT)(cell_height*rows), 
    //0, 
	master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна
	0, BKBInst, 0L );

	// Это для BKBProgressWnd
	place_point.x=0;
	place_point.y=screenY-1-(INT)(cell_height*rows);

	if(NULL==Kbhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	// ShowWindow(Kbhwnd,SW_SHOWNORMAL);
	ShowWindow(Kbhwnd,SW_HIDE);

	//=============================================================
	// Теперь ещё и окно анимации добавилось
	//=============================================================

	// 1. Регистрация класса окна
	//WNDCLASS anim_wcl={CS_HREDRAW | CS_VREDRAW, BKBAnimKeybWndProc, 0,
	WNDCLASS anim_wcl={0, BKBAnimKeybWndProc, 0,
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
#ifdef BELYAKOV
        LoadCursor(NULL, IDC_ARROW), 
#else
		NULL,
#endif
		// От фона - только flickering
        // dkblue_brush,
		 0,
		NULL,
		anim_wnd_class_name
	};

	aresult=::RegisterClass(&anim_wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return;
	}


	AnimHwnd=CreateWindowEx(
	WS_EX_TOPMOST,
	anim_wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	master_hwnd, // Чтобы в таскбаре и при альт-табе не появлялись лишние окна
	0, BKBInst, 0L );

	if(NULL==AnimHwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}
	
	ShowWindow(AnimHwnd,SW_HIDE);
}

//================================================================
// Рисуем окно (Из WM_PAINT или сами)
//================================================================
void BKBKeybWnd::OnPaintStep0(HDC hdc)
{
	bool release_dc=false;
	LONG local_redraw_state;
	BKB_key key;

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
	RECT rect_pressed={int(column_pressed*cell_width),int(row_pressed*cell_height),int((column_pressed+1)*cell_width),int((row_pressed+1)*cell_height)};

	// Для отладки
	//local_redraw_state=0;
	
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
		if(shift_pressed&&shift_row!=-1)
		{
			RECT rect={int(shift_column*cell_width),int(shift_row*cell_height),int((shift_column+1)*cell_width),int((shift_row+1)*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
			if(caps_lock_pressed)	TextOut(memdc1,int(shift_column*cell_width+cell_width/3), int(shift_row*cell_height+cell_height*0.8),L"CAPS",4);
		}

		if(ctrl_pressed&&ctrl_row!=-1)
		{
			RECT rect={int(ctrl_column*cell_width),int(ctrl_row*cell_height),int((ctrl_column+1)*cell_width),int((ctrl_row+1)*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if(alt_pressed&&alt_row!=-1)
		{
			//RECT rect={int(14*cell_width),int(cell_height),int(15*cell_width),int(2*cell_height)};
			RECT rect={int(alt_column*cell_width),int(alt_row*cell_height),int((alt_column+1)*cell_width),int((alt_row+1)*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if(Fn_pressed&&fn_row!=-1)
		{
			RECT rect={int(fn_column*cell_width),int(fn_row*cell_height),int((fn_column+1)*cell_width),int((fn_row+1)*cell_height)};
			//RECT rect={0,int(cell_height),int(cell_width),int(2*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
		}

		// 2. Расчерчиваем линии
		SelectObject(memdc1,GetStockObject(WHITE_PEN));
		int i,j;

		// 2.1. Горизонтальные
		for(j=1;j<rows;j++)
		{
			MoveToEx(memdc1,0,int(j*cell_height),NULL);
			LineTo(memdc1,width-1,int(j*cell_height));
		}

		// 2.2. Вертикальные
		for(i=1;i<columns;i++)
		{
			MoveToEx(memdc1,int(cell_width*i),0,NULL);
			LineTo(memdc1,int(cell_width*i),height);
			
			//TextOut(hdc,35,60+i*screen_y/BKB_NUM_TOOLS,tool_names[i],(int)strlen(tool_names[i]));
		}

		// 3. Пишем буквы
		// 3.1. Своим фонтом - из одного символа
		old_font=(HFONT)SelectObject(memdc1, hfont);

		for(j=0;j<rows;j++)
			for(i=0;i<columns;i++)
			{
				key=layout[current_pane*rows*columns+j*columns+i]; //BKBKeybLayouts[current_pane][j][i];
				if(undefined==key.bkb_keytype) continue;

				if(NULL!=key.label) // проверка, что там не NULL
				if(1==wcslen(key.label))
				TextOut(memdc1,int(cell_width*0.4+i*cell_width) , int(cell_height/3.3+j*cell_height),
					key.label,wcslen(key.label));
			}
	
		// Возвращаем старый фонт
		SelectObject(memdc1, old_font);

		// 3.2. Системным фонтом - то, что длиннее одного символа
		for(j=0;j<rows;j++)
			for(i=0;i<columns;i++)
			{
				key=layout[current_pane*rows*columns+j*columns+i]; //BKBKeybLayouts[current_pane][j][i];
				if(undefined==key.bkb_keytype) continue;

				if(NULL!=key.label) // проверка, что там не NULL
				if(wcslen(key.label)>1)
				{
					if(fn==key.bkb_keytype) SetTextColor(memdc1,RGB(255,155,155)); // Кнопку Fn подкрашиваем
					TextOut(memdc1,int(cell_width*0.2+i*cell_width) , int(cell_height/3.3+j*cell_height),
						key.label,wcslen(key.label));
					if(fn==key.bkb_keytype) SetTextColor(memdc1,RGB(255,255,255)); 
				}
				
				// Если есть Fn-клавиша, написать её в [правом] левом нижнем углу красным цветом
				if(NULL!=key.fn_label)
				{
					SetTextColor(memdc1,RGB(255,155,155));
					//TextOut(memdc1,int(cell_width*0.4+i*cell_width) , int(cell_height/3+j*cell_height), key.label,wcslen(key.label));
#ifdef BELYAKOV
					TextOut(memdc1,int(cell_width*0.03+i*cell_width) , int(cell_height*0.75+j*cell_height),key.fn_label,wcslen(key.fn_label));
#else
					TextOut(memdc1,int(cell_width*0.1+i*cell_width) , int(cell_height*0.79+j*cell_height),key.fn_label,wcslen(key.fn_label));
#endif
					SetTextColor(memdc1,RGB(255,255,255)); // Верни цвет на белый
				}
			}
				
		
		// ЗДЕСЬ НЕ НУЖЕН break !!! После нулевого шага всегда идёт первый!!!

	case 1:
		BitBlt(memdc2,0,0,width,height,memdc1,0,0,SRCCOPY);

		// Возможно, рисовать Progress Bar
		if(fixation_approved)
		{
			RECT rect;
		
			rect.left=(LONG)(cell_width/20+column*cell_width);
			rect.right=(LONG)(rect.left+percentage*cell_width*90/100/100);
			rect.top=(LONG)(cell_height/20+cell_height*row);
			rect.bottom=(LONG)(rect.top+cell_height/20); 

			FillRect(memdc2,&rect,blue_brush);
		}

		// В режиме аэромыши для Белякова это отключить
#ifdef BELYAKOV
		if(2!=tracking_device)
#endif
		{
			// Теперь рисуем белое пятно
			BLENDFUNCTION bfn;
			bfn.BlendOp = AC_SRC_OVER;
			bfn.BlendFlags = 0;
			bfn.SourceConstantAlpha = 255;
			bfn.AlphaFormat = AC_SRC_ALPHA;

			//AlphaBlend(memdc2, 0, 0, 100, 64, BufferDC, 0, 0, 100, 64, bfn);
			AlphaBlend(memdc2,whitespot_point.x-50,whitespot_point.y-50,100,100,whitespot_dc,0,0,100,100,bfn);
		}

		// ЗДЕСЬ НЕ НУЖЕН break !!! После первого шага всегда идёт второй!!!
	case 2:
		BitBlt(hdc,0,0,width,height,memdc2,0,0,SRCCOPY);
		break;
	}

	// Если брал DC - верни его
	if(release_dc) ReleaseDC(Kbhwnd,hdc);

}

//=====================================================================================================================================
// Когда рисуем клавиатуру в два прохода
//=====================================================================================================================================
void BKBKeybWnd::OnPaintStep1(HDC hdc)
{
	bool release_dc=false;
	LONG local_redraw_state;
	BKB_key key;

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
	
	//RECT rect_pressed={int(column_pressed*cell_width),int(row_pressed*cell_height),int((column_pressed+1)*cell_width),int((row_pressed+1)*cell_height)};

	// Для отладки
	//local_redraw_state=0;

	switch(local_redraw_state)
	{
	case 0: // Перерисовываем всё с самой глубины
		FillRect(memdc1,&fill_r,dkblue_brush);
		// Нажатой клавиши нет в step==1
/*		// Подсвечиваем нажатую клавишу
		if(column_pressed!=-1)
		{
			//RECT rect_pressed={int(column_pressed*cell_size),int(row_pressed*cell_size),int((column_pressed+1)*cell_size),int((row_pressed+1)*cell_size)};
			FillRect(memdc1,&rect_pressed,blue_brush);
		}
*/
		// Собственно, рисование
/*		// 1. Сначала подсветим нажатые  спец. клавиши
		if(shift_pressed&&shift_row!=-1)
		{
			RECT rect={int(shift_column*cell_width),int(shift_row*cell_height),int((shift_column+1)*cell_width),int((shift_row+1)*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
			if(caps_lock_pressed)	TextOut(memdc1,int(shift_column*cell_width+cell_width/3), int(shift_row*cell_height+cell_height*0.8),L"CAPS",4);
		}

		if(ctrl_pressed&&ctrl_row!=-1)
		{
			RECT rect={int(ctrl_column*cell_width),int(ctrl_row*cell_height),int((ctrl_column+1)*cell_width),int((ctrl_row+1)*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if(alt_pressed&&alt_row!=-1)
		{
			//RECT rect={int(14*cell_width),int(cell_height),int(15*cell_width),int(2*cell_height)};
			RECT rect={int(alt_column*cell_width),int(alt_row*cell_height),int((alt_column+1)*cell_width),int((alt_row+1)*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if(Fn_pressed&&fn_row!=-1)
		{
			RECT rect={int(fn_column*cell_width),int(fn_row*cell_height),int((fn_column+1)*cell_width),int((fn_row+1)*cell_height)};
			//RECT rect={0,int(cell_height),int(cell_width),int(2*cell_height)};
			FillRect(memdc1,&rect,blue_brush);
		}
*/
		// 2. Расчерчиваем линии
		SelectObject(memdc1,GetStockObject(WHITE_PEN));
		int i,j;

		// 2.1. Горизонтальные
		for(j=1;j<cropped_rows;j++)
		{
			MoveToEx(memdc1,0,int(j*cropped_cell_width),NULL);
			LineTo(memdc1,width-1,int(j*cropped_cell_width));
		}

		// 2.2. Вертикальные
		for(i=1;i<cropped_columns;i++)
		{
			MoveToEx(memdc1,int(cropped_cell_width*i),0,NULL);
			LineTo(memdc1,int(cropped_cell_width*i),height);
			
			//TextOut(hdc,35,60+i*screen_y/BKB_NUM_TOOLS,tool_names[i],(int)strlen(tool_names[i]));
		}

		// 3. Пишем буквы
		// 3.1. Своим фонтом - из одного символа
		old_font=(HFONT)SelectObject(memdc1, hfont);

		for(j=0;j<cropped_rows;j++)
			for(i=0;i<cropped_columns;i++)
			{
				key=layout[current_pane*rows*columns+(j+cropped_row)*columns+i+cropped_column]; //BKBKeybLayouts[current_pane][j][i];
				if(undefined==key.bkb_keytype) continue;

				if(NULL!=key.label) // проверка, что там не NULL
				if(1==wcslen(key.label))
				TextOut(memdc1,int(cropped_cell_width*0.4+i*cropped_cell_width) , int(cropped_cell_width/3.3+j*cropped_cell_width),
					key.label,wcslen(key.label));
			}
	
		// Возвращаем старый фонт
		SelectObject(memdc1, old_font);

		// 3.2. Системным фонтом - то, что длиннее одного символа
		for(j=0;j<cropped_rows;j++)
			for(i=0;i<cropped_columns;i++)
			{
				key=layout[current_pane*rows*columns+(j+cropped_row)*columns+i+cropped_column]; //BKBKeybLayouts[current_pane][j][i];
				if(undefined==key.bkb_keytype) continue;

				if(NULL!=key.label) // проверка, что там не NULL
				if(wcslen(key.label)>1)
				{
					if(fn==key.bkb_keytype) SetTextColor(memdc1,RGB(255,155,155)); // Кнопку Fn подкрашиваем
					TextOut(memdc1,int(cropped_cell_width*0.2+i*cropped_cell_width) , int(cropped_cell_width/3.3+j*cropped_cell_width),
						key.label,wcslen(key.label));
					if(fn==key.bkb_keytype) SetTextColor(memdc1,RGB(255,255,255)); 
				}
				
				// Если есть Fn-клавиша, написать её в [правом] левом нижнем углу красным цветом
				if(NULL!=key.fn_label)
				{
					SetTextColor(memdc1,RGB(255,155,155));
					//TextOut(memdc1,int(cell_width*0.4+i*cell_width) , int(cell_height/3+j*cell_height), key.label,wcslen(key.label));
#ifdef BELYAKOV
					TextOut(memdc1,int(cropped_cell_width*0.03+i*cropped_cell_width) , int(cropped_cell_width*0.75+j*cropped_cell_width),key.fn_label,wcslen(key.fn_label));
#else
					TextOut(memdc1,int(cropped_cell_width*0.1+i*cropped_cell_width) , int(cropped_cell_width*0.79+j*cropped_cell_width),key.fn_label,wcslen(key.fn_label));
#endif
					SetTextColor(memdc1,RGB(255,255,255)); // Верни цвет на белый
				}
			}
				
		
		// ЗДЕСЬ НЕ НУЖЕН break !!! После нулевого шага всегда идёт первый!!!

	case 1:
		BitBlt(memdc2,0,0,width,height,memdc1,0,0,SRCCOPY);

		// При step 1 никаких progress-bar нет
		/*
		// Возможно, рисовать Progress Bar
		if(fixation_approved)
		{
			RECT rect;
		
			rect.left=(LONG)(cell_width/20+column*cell_width);
			rect.right=(LONG)(rect.left+percentage*cell_width*90/100/100);
			rect.top=(LONG)(cell_height/20+cell_height*row);
			rect.bottom=(LONG)(rect.top+cell_height/20); 

			FillRect(memdc2,&rect,blue_brush);
		}
		*/
		// В режиме аэромыши для Белякова это отключить
#ifdef BELYAKOV
		if(2!=tracking_device)
#endif
		{
			// Теперь рисуем белое пятно
			BLENDFUNCTION bfn;
			bfn.BlendOp = AC_SRC_OVER;
			bfn.BlendFlags = 0;
			bfn.SourceConstantAlpha = 255;
			bfn.AlphaFormat = AC_SRC_ALPHA;

			//AlphaBlend(memdc2, 0, 0, 100, 64, BufferDC, 0, 0, 100, 64, bfn);
			AlphaBlend(memdc2,whitespot_point.x-50,whitespot_point.y-50,100,100,whitespot_dc,0,0,100,100,bfn);
		}

		// ЗДЕСЬ НЕ НУЖЕН break !!! После первого шага всегда идёт второй!!!
	case 2:
		BitBlt(hdc,0,0,width,height,memdc2,0,0,SRCCOPY);
		break;
	}

	if(flag_hide_anim)
	{
		ShowWindow(AnimHwnd,SW_HIDE);
		flag_hide_anim=false;
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
		ScreenToClient(Kbhwnd,&start_point);
		if((start_point.y>=0)&&(start_point.y<height)&&(start_point.x>=0)&&(start_point.x<width)) // Попали внутрь окна
		{
			fixation_approved=true;

			row=(int)(start_point.y/cell_height);
			if(row>rows-1) row=rows-1;
			column=(int)(start_point.x/cell_width);
			if(column>columns-1) column=columns-1;
			// Здесь была охренительная ошибка! Как это вообще работало...
			// if(column>columns-1) columns=columns-1;
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
			ScreenToClient(Kbhwnd,&p_tmp);
			int row_tmp=(int)(p_tmp.y/cell_height);
			int column_tmp=(int)(p_tmp.x/cell_width);

			if((row_tmp!=row)||(column_tmp!=column)) return false;
		}
	}


	// Рисуем прогресс-бар на кнопке
	if(fixation_approved)
	{
		// Теперь это всё в WM_PAINT
		// Атомарно меняем 2 на 1
		LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);
		
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

	// Из другого потока нельзя вызывать InvalidateRect
	// Делаем перерисовку, только если старое состояние было двойкой
	if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
	//InvalidateRect(Kbhwnd,NULL,TRUE);
}

//===========================================================================
// Произошла фиксация, возможно на клавиатуре
//===========================================================================
extern bool flag_Pink_approved;
DWORD BKBKeybWnd::animation_start_time;
bool BKBKeybWnd::animation_started;

bool BKBKeybWnd::IsItYours(POINT *p, BKB_MODE *bm)
{
	INPUT input[2]={0},shift_input={0},ctrl_input={0},alt_input={0};
	
	bool result;
	POINT p_tmp=*p;
	ScreenToClient(Kbhwnd,&p_tmp); // Пришлось переместить в начало, потому что в середине меняется геометрия окна
	if((p_tmp.y>=0)&&(p_tmp.y<height)&&(p_tmp.x>=0)&&(p_tmp.x<width)) result=true;// Попали внутрь окна
	else result=false; // но не возвращаемся немендленно. Ибо при розовом прямоугольнике валидная область БОЛЬШЕ, чем размер окна

	// Нововведение - нажатие может означать переход на следующий шаг
	if(gBKB_2STEP_KBD_MODE)
	{
		// Попадание нужно контролировать:
		// 1. на шаге 0 = попадание в последний розовый прямоугольник
		// 2. на шаге 1 = попадание в клавиатуру

		if(0==step)
		{
			// А попали ли мы вообще в клавиатуру (творчески расширенную)?
			if((p->x<extended_rect.left)||(p->x>extended_rect.right)||(p->y<extended_rect.top)||(p->y>extended_rect.bottom))
			return false;

			//step=1;
			//redraw_state=0; // перерисовать клавиатуру с самого начала
			//Place();
			//InvalidateRect(Kbhwnd,NULL,FALSE); 

			// Теперь вместо этого убираем клавиатуру и начинаем анимацию
			animation_start_time=timeGetTime();
			animation_started=true;
			Animate();
	
			ShowWindow(AnimHwnd,SW_SHOWNORMAL);
			//ShowWindow(Kbhwnd, SW_HIDE);
			BKBProgressWnd::Hide();

			return true;
		}
		else
		{
			if(!result) return false; // В окно не попали

			column=cropped_column+step1_element_column;
			row=cropped_row+step1_element_row;
			fixation_approved=true;
		}
	}

	// Далее - старый код, когда ещё не было Pink (с маленькой вставкой)
	if(fixation_approved)
	{
		// Сбросить Progress Bar
		ProgressBarReset();
		fixation_approved=false;

		// Реагируем на нажатую кнопку
		// Определим нажатую кнопку
		BKB_key key_pressed=layout[current_pane*rows*columns+row*columns+column]; //BKBKeybLayouts[current_pane][row][column];
		// Функциональные (Fn) клавиши
		if(Fn_pressed)
		{
			// во внешних файлах клавиатур загрузка строки Fn происходит, только когда стоит bkb_fn_scancode
			if((key_pressed.bkb_fn_vscancode)&&(0x09!=key_pressed.bkb_vscancode)) // Есть, что нажать
			{
				ScanCodeButton(key_pressed.bkb_fn_vscancode);
			}
			else if(0x09==key_pressed.bkb_vscancode) // Fn+TAB==Settings Dialogue
			{
				BKBKeybWnd::DeActivate();	// деактивировать клавиатуру, сбросить режим, перерисовать Toolbar
				BKBToolWnd::current_tool=-1;
				*bm=BKB_MODE_NONE;
				BKBTranspWnd::Show(); // Показать стрелку
				//PostMessage(BKBToolWnd::GetHwnd(), WM_USER_INVALRECT, 0, 0);
				BKBToolWnd::Place();
				BKBSettings::SettingsDialogue();
			}
			Fn_pressed=false; // Сбрасываем нажатую клавишу Fn, даже если не было, что нажать
		}
		else // Это НЕ функциональная клавиша
		switch (key_pressed.bkb_keytype)
		{
		case unicode: // Самое простое
			// Поправка. Если нажаты Alt или Ctrl и задан сканкод, то нужно не юникод, а сканкод использовать
			if((alt_pressed||ctrl_pressed)&&key_pressed.bkb_vscancode)
			{
				ScanCodeButton(key_pressed.bkb_vscancode);
			}
			else
			{
				// Нажатие unicode-кнопки
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
			}

		case scancode: // Здесь нужно обрабатывать Alt,Shift,Ctrl; вынесено в отдельную функцию
			ScanCodeButton(key_pressed.bkb_vscancode);
			break;

		case leftkbd: // другой экран клавиатуры (слева от текущего)
			current_pane--;
			if(current_pane<0) current_pane=panes-1;
			PopulateCtrlAltShiftFn();
			break;

		case rightkbd: // другой экран клавиатуры (справа от текущего)
			current_pane++;
			if(current_pane>=panes) current_pane=0;
			PopulateCtrlAltShiftFn();
			break;

		case top_down: // другой экран клавиатуры (справа от текущего)
			PostMessage(Kbhwnd, WM_USER_KBD_TOPDOWN, 0, 0);
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

		// Добавим системный щелчок
		//PlaySound( TEXT("DeviceDisconnect"), NULL, SND_ALIAS|SND_ASYNC );
		// Если внешний файл со звуком недоступен, играем синтезированный звук
		if(S_OK!=PlaySound( TEXT("click.wav"), NULL, SND_FILENAME|SND_ASYNC|SND_NODEFAULT ))
		BKBClick::Play();

		// запомним, что стереть после окончания таймера
		row_pressed=row; column_pressed=column;
		SetTimer(Kbhwnd,4,500,0); // полсекунды
		// Теперь ВСЕГДА перерисовываем клавиатуру после ЛЮБОГО нажатия
		redraw_state=0;

		// Для двухфазного нажатия - придаём клавиатуре обычный вид
		if(gBKB_2STEP_KBD_MODE)
		{
			step=0;
			Place();
			BKBToolWnd::Place(); // ToolBar также меняет форму
		}

		// Из другого потока нельзя вызывать InvalidateRect
		PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
		//InvalidateRect(Kbhwnd,NULL,FALSE); // перерисовать клавиатуру с самого начала
		//OutputDebugString("row_pressed\n");

		return true;
	} //fixation approved

	// Возвращаем всегда, относится ли это нажатие к нам, даже если кнопку не нажали
	return result;
}

//===========================================================================
// Подсветить пятном место взгляда
//===========================================================================
bool BKBKeybWnd::WhiteSpot(POINT *p)
{
	// для отладки
	if(animation_started) 
		return true;

	// Не, всё не так. Надо проверить, может надо стереть пятно, которое было раньше
	//if(p->y<start_y-50) return; // Белое пятно не заехало на клавиатуру

	whitespot_point=*p;
	ScreenToClient(Kbhwnd,&whitespot_point);
	if(!((whitespot_point.y>=0)&&(whitespot_point.y<height)&&(whitespot_point.x>=0)&&(whitespot_point.x<width))) return false;// НЕ попали внутрь окна

	//whitespot_point.y-=start_y; // Такой простой перевод экранных координат в оконные
	//if(whitespot_point.y<0)
	//	return false; // Стрелочка где-то вне клавиатуры

	LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);

	// Из другого потока нельзя вызывать InvalidateRect
	// Делаем перерисовку, только если старое состояние было двойкой
	if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);

	return true; // Вместо стрелочки рисуем пятно
}




//===========================================================================
// Активация клавиатуры
//===========================================================================
void BKBKeybWnd::Activate()
{
	step=0; // перестрахуемся
	PopulateCtrlAltShiftFn(); // Вообще-то это нужно только при первой активации... Ну да ладно.
	ShowWindow(Kbhwnd,SW_SHOWNORMAL);
	fixation_approved=false;
}

//===========================================================================
// Деактивация клавиатуры
//===========================================================================
void BKBKeybWnd::DeActivate()
{
	step=0; // перестрахуемся
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

	// 15.09.2014 При нажатии на PrntScrn прятать клавиатуру
	if(VK_SNAPSHOT==scancode)
	{
		//ShowWindow(Kbhwnd,SW_HIDE);
		ShowWindow(Kbhwnd,SW_MINIMIZE);
		Sleep(300);
	}

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
	
	// Попробуем внести задержку в 0.08 секунды
	Sleep(80);

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
	
	// 15.09.2014 При нажатии на PrntScrn прятать клавиатуру
	if(VK_SNAPSHOT==scancode) 
	{
		Sleep(300);
		ShowWindow(Kbhwnd,SW_RESTORE);
		//ShowWindow(Kbhwnd,SW_SHOWNORMAL);
	}

	//if(redraw_reqired) InvalidateRect(Kbhwnd,NULL,TRUE); // перерисовать клавиатуру
}

//===========================================================================================================
// Меняем размеры окна (фактически, один раз при создании окна)
// 03.10.2014 А вот и враки! теперь сможем ужимать его при выборе опции "не перекрывать панель инструментов"
// Пересоздает все memdc и hbm
//===========================================================================================================
void BKBKeybWnd::OnSize(HWND hwnd, int _width, int _height)
{
	HDC hdc=GetDC(hwnd);

	width=_width;
	height=_height;
	cell_width=width/(float)columns;
	cell_height=height/(float)rows;

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
	InvalidateRect(Kbhwnd,NULL,TRUE); 
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

//============================================================================================
// Перемещает окно клавиатуры сверху вниз и наоборот
//============================================================================================
void BKBKeybWnd::OnTopDown()
{
	if(bottom_side) bottom_side=false; else bottom_side=true;

	Place();
}

//============================================================================================
//  В переключенной раскладке находим клавиши Ctrl, Alt, Shift, Fn
//============================================================================================
void BKBKeybWnd::PopulateCtrlAltShiftFn()
{
	if(!layout) return;

#ifdef BELYAKOV
	// переключаем раскладку для клавиатуры у Белякова
	// код свистнут из форума vingrad
	if(0==current_pane)
		PostMessage(GetForegroundWindow(),WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)hkl_russian);
		//ActivateKeyboardLayout(hkl_russian, KLF_SETFORPROCESS);
	else
		PostMessage(GetForegroundWindow(),WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)hkl_usenglish);
		//ActivateKeyboardLayout(hkl_usenglish, KLF_SETFORPROCESS);
#endif

	int i,j;

	ctrl_row=-1;
	ctrl_column=-1;
	alt_row=-1;
	alt_column=-1;
	shift_row=-1;
	shift_column=-1;
	fn_row=-1;
	fn_column=-1;

	for(j=0;j<rows;j++)
		for(i=0;i<columns;i++)
		{
			switch(layout[current_pane*rows*columns+j*columns+i].bkb_keytype)
			{
			case shift:
				shift_row=j;
				shift_column=i;
				break;
				
			case control:
				ctrl_row=j;
				ctrl_column=i;
				break;
				
			case alt:
				alt_row=j;
				alt_column=i;
				break;
				
			case fn:
				fn_row=j;
				fn_column=i;
				break;
			}
		}

		// Если в выбранной раскладке нет спец. клавиш, которые нажаты, то их отжать, чтобы не сработало что-то неожиданное
		if(-1==shift_row) {shift_pressed=false;	caps_lock_pressed=false;}
		if(-1==ctrl_row) ctrl_pressed=false;
		if(-1==alt_row)  alt_pressed=false; 
		if(-1==fn_row) Fn_pressed=false; 
}

//===============================================================================================================
//  Показывать клавиатуру в полном или урезанном размере, если в урезанном - сдвинуть в зависимость от тулбара
//===============================================================================================================
void BKBKeybWnd::Place()
{
	// Теперь запоминаем place_point, он потребуется для BKBProgressWnd
	
	if(0==step)
	{
		// 05.10.2014 - борьба с деградацией размера клавиатуры
		if(gBKB_FullSizeKBD) cell_width=screenX/(float)columns;
		else  cell_width=(screenX-gBKB_TOOLBOX_WIDTH)/(float)columns;

		if(cell_width*rows<screenY*0.45f) cell_height=cell_width; // Удалось уложиться в 0.45 высоты экрана при квадратных кнопках
		else cell_height=0.45f*screenY/rows; // Приплюснем кнопки, чтобы не перекрыть более 0.45 экрана

		height=cell_height*rows;
		// \05.10.2014

		if(bottom_side) place_point.y=screenY-1-height; else place_point.y=0;

		if(gBKB_FullSizeKBD)
		{
			place_point.x=0;
			width=screenX;
		}
		else
		{
			if(BKBToolWnd::LeftSide()) place_point.x=gBKB_TOOLBOX_WIDTH; else place_point.x=0;
			width=screenX-gBKB_TOOLBOX_WIDTH;
		}
	}
	else // 1==step
	{
		// Терерь при зуме клавиатуры она никогда не наезжает на тулбар (23/5/2015)
		//if(gBKB_FullSizeKBD) cropped_cell_width=screenX/4.0f;
		//else  cropped_cell_width=(screenX-gBKB_TOOLBOX_WIDTH)/4.0f;
		cropped_cell_width=(screenX-gBKB_TOOLBOX_WIDTH)/4.0f;

		height=cropped_cell_width*cropped_rows;

		if(bottom_side) place_point.y=screenY-1-height; else place_point.y=0;

		// Терерь при зуме клавиатуры она никогда не наезжает на тулбар (23/5/2015)
		//if(gBKB_FullSizeKBD)
		//{
		//	place_point.x=0;
		//	width=screenX;
		//}
		//else
		{
			if(BKBToolWnd::LeftSide()) place_point.x=gBKB_TOOLBOX_WIDTH; else place_point.x=0;
			width=screenX-gBKB_TOOLBOX_WIDTH;
		}

		if (cropped_columns!=4) // менее 4 клавиш в строке обрубка клавиатуры
		{
			place_point.x+=(4-cropped_columns)*cropped_cell_width;
			width-=(4-cropped_columns)*cropped_cell_width;
		}
	}

	redraw_state=0;
	if(Kbhwnd)
		SetWindowPos(Kbhwnd,HWND_TOPMOST,place_point.x,place_point.y,width,height,0);
	
}

//================================================================================================================
// Розовая обводка нескольких клавиш, когла клавиша нажимается в два захода
//================================================================================================================
RECT BKBKeybWnd::extended_rect; // Прямоугольник, возможно с большей высотой, чем клавиатура, взгляд на который считается попаданием в клавиатуру
RECT BKBKeybWnd::element_rect; // Прямоугольник, который будет увеличен до целой клавиатуры на следующем шаге

int BKBKeybWnd::element_rows,BKBKeybWnd::element_columns, BKBKeybWnd::element_row, BKBKeybWnd::element_column; // element - это кусок клавиатуры 4x2 клавиши
int BKBKeybWnd::cropped_rows,BKBKeybWnd::cropped_columns, BKBKeybWnd::cropped_row, BKBKeybWnd::cropped_column; // кусок клавиатуры внутри элемента
int BKBKeybWnd::step1_element_row, BKBKeybWnd::step1_element_column;

// и прямоугольник элемента
LPRECT BKBKeybWnd::PinkFrame(int _x, int _y)
{
	if(0==step)
	{
		// 1. Определить количество элементов
		element_rows=(rows+1)/2;
		element_columns=(columns+3)/4;

		// 2. Находим прямоугольник, куда можно смотреть
		extended_rect.left=place_point.x;
		extended_rect.right=place_point.x+width-1;
		if(bottom_side)
		{
			extended_rect.bottom=screenY-1;
			extended_rect.top=screenY-cell_height*2.0f*element_rows-1;
		}
		else
		{
			extended_rect.bottom=cell_height*2.0f*element_rows-1;
			extended_rect.top=0;
		}

		// А попали ли мы вообще в клавиатуру (творчески расширенную)?
		if((_x<extended_rect.left)||(_x>extended_rect.right)||(_y<extended_rect.top)||(_y>extended_rect.bottom))
			return NULL;

		// 3. Находим прямоугольник для подсветки
		element_column=(_x-extended_rect.left)/(4.0f*cell_width);
		element_row=(_y-extended_rect.top)/(2.0f*cell_height);

		element_rect.left=extended_rect.left+element_column*4.0f*cell_width;
		element_rect.right=extended_rect.left+(element_column+1)*4.0f*cell_width-1;

		element_rect.top=extended_rect.top+element_row*2.0f*cell_height;
		element_rect.bottom=extended_rect.top+(element_row+1)*2.0f*cell_height-1;

		// Последний жлемент по горизонтали может быть обрезком из 3 и менее клавиш...
		if(element_rect.right>extended_rect.right) element_rect.right=extended_rect.right;

		// В принципе, это надо подсчитывать только перед увеличением элемента, но здесь удобнее (читабельнее)
		// В раскладке клавиатуры этот прямоугольник будет соответствовать вот таким позициям:
		cropped_row=element_row*2;
		cropped_rows=2;
		
		// Но если количество строк нечётное:
		if(element_rows!=rows*2)
		{
			if(bottom_side) // Нулевой элемент начинается в пустоте
			{
				cropped_row=element_row*2-1;
				if(-1==cropped_row) {cropped_row=0; cropped_rows=1;}
			}
			else // последний элемент состоит из одной строки
			{
				if(element_row==element_rows-1) cropped_rows=1;
			}
		}
		
		// Если количество клавиш в строке обрезанного элемента меньше 4:
		cropped_column=element_column*4;
		cropped_columns=4;
		if(columns<(element_column+1)*4) cropped_columns=columns-4*element_column;

		return &element_rect;
	} // 0==step
	else // Рисование прямоугольника на шаге 1
	{
		// 1. Находим прямоугольник, куда можно смотреть
		extended_rect.left=place_point.x;
		extended_rect.right=place_point.x+width-1;
		extended_rect.top=place_point.y;
		extended_rect.bottom=place_point.y+height-1;
		

		// А попали ли мы вообще в клавиатуру (творчески расширенную)?
		if((_x<extended_rect.left)||(_x>extended_rect.right)||(_y<extended_rect.top)||(_y>extended_rect.bottom))
			return NULL;

		// 3. Находим прямоугольник для подсветки
		step1_element_column=(_x-extended_rect.left)/cropped_cell_width;
		step1_element_row=(_y-extended_rect.top)/cropped_cell_width;

		element_rect.left=extended_rect.left+step1_element_column*cropped_cell_width;
		element_rect.right=extended_rect.left+(step1_element_column+1)*cropped_cell_width-1;

		element_rect.top=extended_rect.top+step1_element_row*cropped_cell_width;
		element_rect.bottom=extended_rect.top+(step1_element_row+1)*cropped_cell_width;
				
		return &element_rect;
	}
}

//=================================================================
// Здесь мы рисуем разрастание куска клавиатуры на весь экран
//=================================================================
// Пока будет расти целую секунду
#define BKB_ANIMATION_TIME 500
bool BKBKeybWnd::Animate()
{
	if(!animation_started) return false;

	
	// Временно отключаем...
	step=1;
	redraw_state=0; // перерисовать клавиатуру с самого начала
	Place();
	BKBToolWnd::Place(); // ToolBar также меняет форму

	flag_hide_anim=true;
	animation_started=false;
		return false;


	// Для медленной отладки
	/* static int slowdown=0;
	if(slowdown<5) {slowdown+=1; return true;}
	slowdown=0; */
	

	float percentage=(timeGetTime()-animation_start_time)*100.0f/BKB_ANIMATION_TIME;
	if((percentage>=100.0f)||(percentage<0.0f)) // percentage < 0 - это подстраховка от переполнения
	{
		//Попробуем почистить за собой перед изменением размера
		// Только это и спасло от бликующих символов раскладки при переключении!
		RECT fill_r={0,0,width,height};
		HDC hdc=GetDC(Kbhwnd);
		FillRect(hdc,&fill_r,dkblue_brush);
		ReleaseDC(Kbhwnd,hdc); 
		
		step=1;
		redraw_state=0; // перерисовать клавиатуру с самого начала
		Place();
		
		// !!! Спрятать только после прорисовки клавиатуры на новом месте!
		// ShowWindow(AnimHwnd,SW_HIDE);
		flag_hide_anim=true;



		animation_started=false;
		return false; // Можете продолжать, как обычно. Анимация закончилась.
	}

	// Собственно рисование
	LONG anim_x_initial=place_point.x+cropped_column*cell_width;
	LONG anim_y_initial=place_point.y+cropped_row*cell_height;
	LONG anim_width_initial=cropped_columns*cell_width;
	LONG anim_height_initial=cropped_rows*cell_height;

	LONG anim_x_final,anim_y_final,anim_width_final,anim_height_final;

	// Содрано из Place(). К сожалению, пока вызвать его не можем, ибо старое окно убирается только после показа над ним анимированного.
	{
		if(gBKB_FullSizeKBD) cropped_cell_width=screenX/4.0f;
		else  cropped_cell_width=(screenX-gBKB_TOOLBOX_WIDTH)/4.0f;

		anim_height_final=cropped_cell_width*cropped_rows;

		if(bottom_side) anim_y_final=screenY-1-anim_height_final; else anim_y_final=0;

		if(gBKB_FullSizeKBD)
		{
			anim_x_final=0;
			anim_width_final=screenX;
		}
		else
		{
			if(BKBToolWnd::LeftSide()) anim_x_final=gBKB_TOOLBOX_WIDTH; else anim_x_final=0;
			anim_width_final=screenX-gBKB_TOOLBOX_WIDTH;
		}

		if (cropped_columns!=4) // менее 4 клавиш в строке обрубка клавиатуры
		{
			anim_x_final+=(4-cropped_columns)*cropped_cell_width;
			anim_width_final-=(4-cropped_columns)*cropped_cell_width;
		}
	}

	LONG anim_x=anim_x_initial+percentage*(anim_x_final-anim_x_initial)/100.0f;
	LONG anim_y=anim_y_initial+percentage*(anim_y_final-anim_y_initial)/100.0f;
	anim_width=anim_width_initial+percentage*(anim_width_final-anim_width_initial)/100.0f;
	anim_height=anim_height_initial+percentage*(anim_height_final-anim_height_initial)/100.0f;
	
	
	// HDC hdc2=GetDC(AnimHwnd);
	//RECT fill_r2;
	//GetClientRect(AnimHwnd,&fill_r2);
	//FillRect(hdc2,&fill_r2,dkblue_brush);

	//MoveWindow(AnimHwnd,anim_x,anim_y,anim_width,anim_height,TRUE);
	//SetWindowPos(AnimHwnd,HWND_TOPMOST,anim_x,anim_y,anim_width,anim_height,SWP_NOCOPYBITS | SWP_NOREDRAW);
	SetWindowPos(AnimHwnd,HWND_TOPMOST,anim_x,anim_y,anim_width_final,anim_height_final,SWP_NOCOPYBITS | SWP_NOREDRAW);
	//MoveWindow(AnimHwnd,anim_x,anim_y,anim_width,anim_height,FALSE);
	
	/*
	HDC hdc2=GetDC(AnimHwnd);
	OnAnimPaint(hdc2);
	ReleaseDC(AnimHwnd,hdc2);
	*/

	return true;
}

//======================================================================
// Увеличение элемента клавиатуры
//======================================================================
void BKBKeybWnd::OnAnimPaint(HDC hdc)
{
	StretchBlt(hdc,0,0, anim_width,anim_height,
		memdc1, cell_width*cropped_column, cell_height*cropped_row,
		cell_width*cropped_columns, cell_height*cropped_rows,
		SRCCOPY);
}

//======================================================================
// Увеличение элемента клавиатуры
// Урезанная ScanCodeButton - не жмёт никакие шифты и контролы
//======================================================================
void BKBKeybWnd::BackSpace()
{
	INPUT input={0};
	input.type=INPUT_KEYBOARD;

	// Нажатие кнопки
	input.ki.dwFlags =  0;
	input.ki.wVk=VK_BACK;
	SendInput(1,&input,sizeof(INPUT));		
	
	// Попробуем внести задержку в 0.08 секунды
	Sleep(80);

	// Отпускание кнопки
	input.ki.dwFlags = KEYEVENTF_KEYUP ;
	SendInput(1,&input,sizeof(INPUT));	

	// Тут тоже будем издавать звук
	if(S_OK!=PlaySound( TEXT("click.wav"), NULL, SND_FILENAME|SND_ASYNC|SND_NODEFAULT ))
		BKBClick::Play();
}