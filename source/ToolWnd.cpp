#include <Windows.h>
#include <stdint.h> // Это для uint64_t
#include "ToolWnd.h"
#include "BKBRepErr.h"
#include "KeybWnd.h"
#include "TobiiREX.h"
#include "AirMouse.h"
#include "TranspWnd.h"
#include "Internat.h"
#include "WM_USER_messages.h"
#include "Fixation.h"
#include "BKBTurtle.h"


//int gBKB_TOOLBOX_WIDTH=128;

int gBKB_TOOLBOX_WIDTH=256;
int gBKB_TOOLBOX_BUTTONS=7;
extern bool gBKB_SHOW_CLICK_MODS;
bool gBKB_SLEEP_IN_BLACK=true;
static int BKB_TURTLE_BUTTONS_VISIBLE=4;

#define BKB_SLEEP_COUNT 3


typedef struct
{
	TCHAR *tool_name;
	int internat_num; // номер строки для загрузки другого языка
	int flag_modifiers; // bool в натуре
	BKB_MODE bkb_mode;
} ToolWndConfig;

#ifdef BELYAKOV
// кнопка Двойной..
#define BKB_NUM_TOOLS 10
ToolWndConfig tool_config[BKB_NUM_TOOLS]=
{
	{L"КЛАВИШИ",17,0,BKB_MODE_KEYBOARD},
	{L"ЛЕВЫЙ",12,1,BKB_MODE_LCLICK},
	{L"ЛЕВЫЙ,..",34,1,BKB_MODE_LCLICK_PLUS},
	{L"ПРАВЫЙ",13,1,BKB_MODE_RCLICK},
	{L"ДВОЙНОЙ",14,1,BKB_MODE_DOUBLECLICK},
	{L"ДВОЙНОЙ,..",37,1,BKB_MODE_DOUBLECLICK_PLUS},
	{L"ДРЕГ",15,0,BKB_MODE_DRAG},
	{L"СКРОЛЛ",16,0,BKB_MODE_SCROLL},
	//{L"КЛАВИШИ",17,0,BKB_MODE_KEYBOARD},
	{L"Туда-Сюда",18,0,BKB_MODE_SWAP},
	{L"Спать",76,0,BKB_MODE_SLEEP}
};
#else

#define BKB_NUM_TOOLS 7
ToolWndConfig tool_config[BKB_NUM_TOOLS]=
{
	{L"КЛАВИШИ",17,0,BKB_MODE_KEYBOARD},
	//{L"ЧЕРЕПАШКА",77,1,BKB_MODE_TURTLE},
	{L"ЛЕВЫЙ",12,1,BKB_MODE_LCLICK},
	{L"ЛЕВЫЙ,..",34,1,BKB_MODE_LCLICK_PLUS},
	{L"ДВОЙНОЙ",14,1,BKB_MODE_DOUBLECLICK},
	{L"ПРАВЫЙ",13,1,BKB_MODE_RCLICK},

	{L"ДРЕГ",15,0,BKB_MODE_DRAG},
	//{L"СКРОЛЛ",16,0,BKB_MODE_SCROLL},
	// {L"КЛАВИШИ",17,0,BKB_MODE_KEYBOARD},
	{L"Туда-Сюда",18,0,BKB_MODE_SWAP},
	//{L"Спать",76,0,BKB_MODE_SLEEP}
	//{L"РЕЗЕРВ",19,0,BKB_MODE_NONE} 
};

/*
#define BKB_NUM_TOOLS 4
ToolWndConfig tool_config[BKB_NUM_TOOLS]=
{
	{L"ЛЕВЫЙ",12,1,BKB_MODE_LCLICK},
	{L"ЛЕВЫЙ,..",34,1,BKB_MODE_LCLICK_PLUS},
	{L"КЛАВИШИ",17,0,BKB_MODE_KEYBOARD},
	{L"Туда-Сюда",18,0,BKB_MODE_SWAP}
}; */
#endif // BELYAKOV

// Кнопки для черепашки
ToolWndConfig tool_config_turtle[5]=
{
	{L"ЛЕВЫЙ",12,1,BKB_MODE_LCLICK},
	{L"ДВОЙНОЙ",14,1,BKB_MODE_DOUBLECLICK},
	{L"ДРЕГ",15,0,BKB_MODE_DRAG},
	{L"ПРАВЫЙ",13,1,BKB_MODE_RCLICK},
	{L"ВЫХОД",60,0,BKB_MODE_NONE} 
};

extern HINSTANCE BKBInst;
extern HBRUSH dkblue_brush, dkblue_brush2, blue_brush;
extern HPEN pink_pen;
extern int tracking_device;
extern int screenX, screenY;
extern bool flag_continuous_turtle;
void on_gaze_data_main_thread(); // определена в OnGazeData.cpp


static const TCHAR *wnd_class_name=L"BKBTool";

//static const TCHAR *tool_names[BKB_NUM_TOOLS];
//static BKB_MODE tool_bm[BKB_NUM_TOOLS]={BKB_MODE_LCLICK, BKB_MODE_LCLICK_PLUS, BKB_MODE_RCLICK, BKB_MODE_DOUBLECLICK, BKB_MODE_DRAG, 
//	BKB_MODE_SCROLL, BKB_MODE_KEYBOARD, BKB_MODE_NONE, BKB_MODE_NONE};

bool BKBToolWnd::tool_modifier[4]={false,false,false,false};
TCHAR *BKBToolWnd::tool_modifier_name[4]={L"+ Ctrl",L"+ Shift",L"+ Alt",L"Без Зума"};

HWND  BKBToolWnd::Tlhwnd=0;
int BKBToolWnd::current_tool=-1;
int BKBToolWnd::offset=0; //какой инструмент виден первым в прокрутке
int BKBToolWnd::height=0;
bool BKBToolWnd::left_side=false;

static int bkb_sleep_count=BKB_SLEEP_COUNT; // Количество фиксаций для вывода из состояния сна
static int transparency=60, last_transparency=60;

static TCHAR *button_exit=L"ВЫХОД";
static TCHAR *button_cancel=L"ОТМЕНА";
static TCHAR *button_backspace=L"Backspace";
static TCHAR *label_repeating=L"(повторяющийся)"; // message #35

POINT BKBToolWnd::place_point={0,0};
//================================================================================================================
// Оконная процедура 
//================================================================================================================
LRESULT CALLBACK BKBToolWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_USER_INVALRECT: // Это приходит из другого потока
		InvalidateRect(hwnd,NULL,TRUE);
		break;

	case WM_CREATE:
		SetLayeredWindowAttributes(hwnd,NULL,255*60/100,LWA_ALPHA);
		last_transparency=60;
		if(2==tracking_device) BKBAirMouse::Init(hwnd);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc=BeginPaint(hwnd,&ps);
		BKBToolWnd::OnPaint(hdc);
		EndPaint(hwnd,&ps);
		break;

	case WM_DESTROY:	// Завершение программы
		if(2==tracking_device) BKBAirMouse::Halt(hwnd);
		else	BKBTobiiREX::Halt();
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		BKBAirMouse::OnTimer();
		break;

	case WM_USER_DATA_READY:
		on_gaze_data_main_thread();
		break;

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // Обработали, свалились сюда по break
}

//================================================================
// Инициализация 
//================================================================
HWND BKBToolWnd::Init()
{
	ATOM aresult; // Для всяких кодов возврата
	int i;
	
	gBKB_TOOLBOX_WIDTH=screenY/gBKB_TOOLBOX_BUTTONS;

	// 0. Заполняем названия инструментов иностранным языком
	for(i=0;i<BKB_NUM_TOOLS;i++)
	{
		if(Internat::Message(tool_config[i].internat_num,0)) tool_config[i].tool_name=Internat::Message(tool_config[i].internat_num,0);
	}
	for(i=0;i<5;i++)
	{
		if(Internat::Message(tool_config_turtle[i].internat_num,0)) tool_config_turtle[i].tool_name=Internat::Message(tool_config[i].internat_num,0);
	}
	
	// tool_names[3]=Internat::Message(35,L"ПРАВЫЙ,..");
	//tool_modifier_name[0]=Internat::Message(35,L"ПОВТОР");
	tool_modifier_name[3]=Internat::Message(36,L"БЕЗ ЗУМА");
	button_exit=Internat::Message(60,L"ВЫХОД");
	button_cancel=Internat::Message(61,L"ОТМЕНА");
	
	// 1. Регистрация класса окна
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBToolWndProc, 0,
		//sizeof(LONG_PTR), // Сюда пропишем ссылку на объект
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		//Надо бы красить фон
        dkblue_brush,
		//0,
		NULL,
		wnd_class_name
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return 0;
	}


	Tlhwnd=CreateWindowEx(
	//WS_EX_LAYERED | WS_EX_TOPMOST| WS_EX_CLIENTEDGE,
	WS_EX_LAYERED | WS_EX_TOPMOST,
	wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	screenX-gBKB_TOOLBOX_WIDTH,0,gBKB_TOOLBOX_WIDTH,screenY, 
    0, 0, BKBInst, 0L );

	if(NULL==Tlhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	Place();
	ShowWindow(Tlhwnd,SW_SHOWNORMAL);
	

	return Tlhwnd;
}

//==========================================================================================
// Вспомогательные функции, преобразуют позицию на экране в номер тула и наоборот
//==========================================================================================
int BKBToolWnd::PositionFromTool(int tool_num)
{
	bool show_arrows;
	int position;

	if((tool_num<0)||(offset<0)||(tool_num<offset)) return -1;

	if(BKB_NUM_TOOLS<=gBKB_TOOLBOX_BUTTONS) show_arrows=false;
	else show_arrows=true;

	position=tool_num-offset;
	if(show_arrows) position+=1;

	if((position>=gBKB_TOOLBOX_BUTTONS)||(show_arrows&&(position>=gBKB_TOOLBOX_BUTTONS-1)))
		return -1;

	else return position;

}

int BKBToolWnd::ToolFromPosition(int position)
{
	bool show_arrows;
	int tool_num;

	if(BKB_NUM_TOOLS<=gBKB_TOOLBOX_BUTTONS) show_arrows=false;
	else show_arrows=true;

	// Валидны только позиции между стрелками перемотки
	if((true==show_arrows)&&((0>=position)||(gBKB_TOOLBOX_BUTTONS-1<=position))) return -1;
	if((false==show_arrows)&&((0>position)||(gBKB_TOOLBOX_BUTTONS<=position))) return -1; 

	tool_num=position+offset;
	if(show_arrows) tool_num-=1;

	if((tool_num<0)||(tool_num>=BKB_NUM_TOOLS))
		return -1;

	else return tool_num;

}

// Функции, рисующие стрелки (потом сделать другие примитивы)
static void DrawUpArrow(HDC hdc,int x, int y)
{
	MoveToEx(hdc,x-35,y,NULL);
	LineTo(hdc,x,y-50);
	LineTo(hdc,x+35,y);
	LineTo(hdc,x+20,y);
	LineTo(hdc,x+20,y+50);
	LineTo(hdc,x-20,y+50);
	LineTo(hdc,x-20,y);
	LineTo(hdc,x-35,y);
}

static void DrawDownArrow(HDC hdc,int x, int y)
{
	MoveToEx(hdc,x-35,y,NULL);
	LineTo(hdc,x,y+50);
	LineTo(hdc,x+35,y);
	LineTo(hdc,x+20,y);
	LineTo(hdc,x+20,y-50);
	LineTo(hdc,x-20,y-50);
	LineTo(hdc,x-20,y);
	LineTo(hdc,x-35,y);
}
//================================================================
// Рисуем окно (Из WM_PAINT или сами)
//================================================================
void BKBToolWnd::OnPaint(HDC hdc)
{
	int i,position;
	bool release_dc=false;
	//LONG tool_height=screenY/BKB_NUM_TOOLS;
	LONG tool_height=gBKB_TOOLBOX_WIDTH;
	bool show_arrows;

	if(BKB_NUM_TOOLS<=gBKB_TOOLBOX_BUTTONS) show_arrows=false;
	else show_arrows=true;

	// Возможно, кто-то захотел изменить прозрачность, например, BKB_MODE_SLEEP
	if(transparency!=last_transparency)
	{
		SetLayeredWindowAttributes(Tlhwnd,NULL,255*transparency/100,LWA_ALPHA);
		last_transparency=transparency;
	}

	if(0==hdc)
	{
		release_dc=true;
		hdc=GetDC(Tlhwnd);
	}

	//====================================================================================
	// Спец. случай - sleep mode Малевича на весь экран
	if((BKB_MODE_SLEEP==Fixation::CurrentMode())&&gBKB_SLEEP_IN_BLACK)
	{
		RECT rect;

		GetClientRect(Tlhwnd,&rect);
		FillRect(hdc,&rect,(HBRUSH)GetStockObject(BLACK_BRUSH));
		
		// И точечку белую (розовую)
		// SelectObject(hdc,GetStockObject(WHITE_PEN));
		SelectObject(hdc,pink_pen);
		
		for(i=0;i<bkb_sleep_count;i++)
		{
			MoveToEx(hdc,rect.right-10-i*10,rect.bottom-10,NULL);
			LineTo(hdc,rect.right-10-i*10,rect.bottom-10);
		}

		// Если брал DC - верни его
		if(release_dc) ReleaseDC(Tlhwnd,hdc);

		return;
	} // Сон в темноте

	
   // цвета подправим
	SetTextColor(hdc,RGB(255,255,255));
	SetBkColor(hdc,RGB(45,62,90));
	SelectObject(hdc,GetStockObject(WHITE_PEN));

	// Собственно, рисование
	// Новый раздел - в режиме клавиатуры рисуем только две кнопки
	// Сначала проверяем, уж не клавиатура ли выбрана? В этом случае вырожденное окно получаем
	if(BKB_MODE_KEYBOARD==Fixation::CurrentMode())
	{
		// Первая кнопка меняет название в зависимости от шага
		if(1==BKBKeybWnd::step) TextOut(hdc,25,60,button_cancel,wcslen(button_cancel));
		else
		{
			TextOut(hdc,25,60,button_backspace,wcslen(button_backspace));
			TextOut(hdc,25,60+tool_height,button_exit,wcslen(button_exit));
			// Ещё чертим линию
			MoveToEx(hdc,0,tool_height,NULL);
			LineTo(hdc,gBKB_TOOLBOX_WIDTH-1,tool_height);
		}
	}
	else if(BKB_MODE_TURTLE==Fixation::CurrentMode()) // Также спец.случай - черепашка
	{
		// Подсветка бывает только у DRAG на первом шаге
		if(Fixation::drag_in_progress)
		{
			RECT rect={10,10+2*tool_height,gBKB_TOOLBOX_WIDTH/2-10,20+2*tool_height};
			FillRect(hdc,&rect,blue_brush);
		}

		for(i=0;i<BKB_TURTLE_BUTTONS_VISIBLE;i++)
		{
			TextOut(hdc,25,60+i*tool_height,tool_config_turtle[i].tool_name,wcslen(tool_config_turtle[i].tool_name));
			// Ещё чертим линию
			MoveToEx(hdc,0,tool_height*i,NULL);
			LineTo(hdc,gBKB_TOOLBOX_WIDTH-1,tool_height*i);
		}
	}
	else // Нет, не клавиатура и не черепашка
	{
		// 1. Сначала подсветим рабочий инструмент
		if(current_tool>=0)
		{ 
			position=PositionFromTool(current_tool);
			if(position>=0)
			{
				RECT rect={0,position*tool_height,gBKB_TOOLBOX_WIDTH,(position+1)*tool_height};
				FillRect(hdc,&rect,blue_brush);
			}
			// подсветим модификаторы
			if(gBKB_SHOW_CLICK_MODS)
			if(tool_config[current_tool].flag_modifiers)
			{
				int pos_modif;
				for(i=0;i<4;i++)
				{
					pos_modif=PositionFromTool(current_tool+1+i);
					if(pos_modif>=0)
					{
						RECT rect={0,pos_modif*tool_height,gBKB_TOOLBOX_WIDTH,(pos_modif+1)*tool_height};
						if(tool_modifier[i]) FillRect(hdc,&rect,blue_brush);
						else FillRect(hdc,&rect,dkblue_brush2);
					}
				}
			}
			
		}


		// Чертим линии
		for(i=0;i<gBKB_TOOLBOX_BUTTONS;i++)
		{
			MoveToEx(hdc,0,i*tool_height,NULL);
			LineTo(hdc,gBKB_TOOLBOX_WIDTH-1,i*tool_height);
		}
	
		int pos_i;
		for(i=0;i<BKB_NUM_TOOLS;i++)
		{
			pos_i=PositionFromTool(i);
			if(pos_i<0) continue;

			// Если у текущего инструмента есть модификаторы - пишем модификаторы вместо следующих четырёх инструментов
			if(current_tool>=0) 
			{
				if(tool_config[current_tool].flag_modifiers&&(i>current_tool)&&(i<=current_tool+4)&&(gBKB_SHOW_CLICK_MODS)) 
					TextOut(hdc,25,60+pos_i*tool_height,BKBToolWnd::tool_modifier_name[i-current_tool-1],wcslen(BKBToolWnd::tool_modifier_name[i-current_tool-1]));
				else TextOut(hdc,25,60+pos_i*tool_height,tool_config[i].tool_name,wcslen(tool_config[i].tool_name));
			}
			else TextOut(hdc,25,60+pos_i*tool_height,tool_config[i].tool_name,wcslen(tool_config[i].tool_name));
		
			// Progress-bar засыпания-просыпания
			// Проверим, попали ли на кнопку засыпания
			if((bkb_sleep_count<BKB_SLEEP_COUNT)&&(bkb_sleep_count>0))
			{
				if(BKB_MODE_SLEEP==tool_config[i].bkb_mode)
				{
					RECT rect;
		
					rect.left=(LONG)(gBKB_TOOLBOX_WIDTH/10);
					rect.right=(LONG)(rect.left+(BKB_SLEEP_COUNT-bkb_sleep_count)*100/BKB_SLEEP_COUNT*gBKB_TOOLBOX_WIDTH*8/1000);
					rect.top=(LONG)(tool_height/20+pos_i*tool_height);
					rect.bottom=(LONG)(rect.top+tool_height/20); 

					FillRect(hdc,&rect,blue_brush);
				}
			}
		}

		// Рисуем стрелки
		if(show_arrows)
		{
			DrawUpArrow(hdc,gBKB_TOOLBOX_WIDTH/2, gBKB_TOOLBOX_WIDTH/2);
			DrawDownArrow(hdc,gBKB_TOOLBOX_WIDTH/2, height-gBKB_TOOLBOX_WIDTH/2);
		}



	}

	// Если брал DC - верни его
	if(release_dc) ReleaseDC(Tlhwnd,hdc);

}

//================================================================
// Возможно переключение режима
//================================================================
bool BKBToolWnd::IsItYours(POINT *_pnt, BKB_MODE *bm)
{
	// Попала ли точка фиксации в границы окна?
	POINT pnt=*_pnt;
	RECT crect;
	GetClientRect(Tlhwnd,&crect);
	ScreenToClient(Tlhwnd,&pnt);
	if((pnt.x>=0)&&(pnt.x<crect.right)&&(pnt.y>0)&&(pnt.y<height))
	// Ещё не включать режим резерв (потом)
	{
		// попала, определяем номер инструмента
		int position=pnt.y/gBKB_TOOLBOX_WIDTH; // Высота и ширина совпадают
		int tool_candidate=ToolFromPosition(position);
		
		// При аэромыши - берём координаты прозрачного окна (с учётом HighDPI)
		// !!! Потом проверить на windows 8.1
		// При других трекерах - координаты курсора
		POINT p;
		if(2==tracking_device) BKBTranspWnd::GetPos(&p);
		else GetCursorPos(&p);

		// Спец. режим - черепашка
		if(BKB_MODE_TURTLE==*bm)
		{
			switch(position)
			{
			case 0: // левый
				BKBTurtle::Show(SW_HIDE);
				Fixation::LeftClick(p);
				break;

			case 1: // двойной
				BKBTurtle::Show(SW_HIDE);
				Fixation::DoubleClick(p);
				break;
		
			case 2: // дрег
				// Если возвращает true, то это был только первый шаг в дреге
				// поэтому режим черепашки на завершаем, просто выходим
				BKBTurtle::Show(SW_HIDE);
				if(Fixation::Drag(p))
				{
					BKBTurtle::Show(SW_SHOW);
					InvalidateRect(Tlhwnd,NULL,TRUE);
					return true;
				}
				break;
			

			case 3: // правый
				BKBTurtle::Show(SW_HIDE);
				Fixation::RightClick(p);
				break;

			case 4: // Выход 
				BKBTurtle::Show(SW_HIDE);
				break;

			default: // фигня какая-то, не может быть, ничего не делать.
				return true;
			
			}

			// Кроме Drag вернуться из режима черепашки
			// В режиме аэромыши спрятать курсор
			if(2==tracking_device) BKBTranspWnd::Hide();

			Fixation::drag_in_progress=false; // Если дрег прошёл только наполовину
			current_tool=-1;
			*bm=BKB_MODE_NONE;
			Place(); // при возврате из режима черепашки нужен Place при уже новом значении *bm
			PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);

			return true;
		}

		// Здесь будут спецрежимы - sleep in black и клавиатура 
		if(BKB_MODE_KEYBOARD==*bm)
		{
			if(1==BKBKeybWnd::step) // Это возможно только в gBKB_PINK_RECT_MODE
			{
				// Любое попадание в тулбар на шаге 1 означает возврат на шаг 0
				BKBKeybWnd::step=0;
				BKBKeybWnd::Place();
				Place();
			}
			else
			{
				if(pnt.y<gBKB_TOOLBOX_WIDTH) // Попали в кнопку BackSpace
				{
					BKBKeybWnd::BackSpace();
				}
				else
				{
					// Выходим из режима клавиатуры (слизано из конца этой функции )
					BKBKeybWnd::DeActivate(); 
					current_tool=-1;
					*bm=BKB_MODE_NONE;
					Place(); // при возврате из режима клавиатуры нужен Place при уже новом значении *bm
				}
			}

			return true;
		}


		// Спец. режим - сон в темноте
		if(gBKB_SLEEP_IN_BLACK&&(BKB_MODE_SLEEP==*bm))
		{
			// Попали ли мы в нижний правый угол экрана?
			if((pnt.y>=screenY-gBKB_TOOLBOX_WIDTH)&&(pnt.x>=crect.right-gBKB_TOOLBOX_WIDTH))
			{
				// Нам нужно выставить tool_candidate, но функция выставляет current_tool
				// Поэтому вот такой геморрой:
				SetCurrentTool(BKB_MODE_SLEEP);
				tool_candidate=current_tool;
				current_tool=-1;
			}
		}

		if(tool_candidate<0)
		{
			// не очень помню, зачем это нужно - потом разберусь
			if((bkb_sleep_count<BKB_SLEEP_COUNT)&&(bkb_sleep_count>0))
			{
				bkb_sleep_count=BKB_SLEEP_COUNT; // если были в состоянии сна, но недодержали 5 секунд, они опять перевзводятся
				PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
			}

			// Попали в окно, а инструмента нет... Возможно - это стрелки!
			// Скроллинг тулбара
			// Стрелки работают, когда не выбран режим сна
			if((BKB_NUM_TOOLS>gBKB_TOOLBOX_BUTTONS)&&(BKB_MODE_SLEEP!=*bm))
			{
				if(0==position) // стрелка вверх, уменьшаем offset
				{
					offset-=1;
					if(offset<0) offset=0;
					InvalidateRect(Tlhwnd,NULL,TRUE);
					return true;
				}
				else if(position==gBKB_TOOLBOX_BUTTONS-1) 
				{
					offset+=1;
					if(BKB_NUM_TOOLS-offset<gBKB_TOOLBOX_BUTTONS-2) offset=BKB_NUM_TOOLS-gBKB_TOOLBOX_BUTTONS+2;
					InvalidateRect(Tlhwnd,NULL,TRUE);
					return true;
				}
				
				return false; // невозможная ситуация, тут бы ошибку напечатать...
			}
			else return false;  // невозможная ситуация, тут бы ошибку напечатать...
		}

		//===================================================================================
		// Очень много про кнопку сна
		//===================================================================================
		// Выключение режима сна
		if(BKB_MODE_SLEEP==*bm)
		{
			// Попали ли в выключалку?
			if(BKB_MODE_SLEEP==tool_config[tool_candidate].bkb_mode)
			{
				bkb_sleep_count--;
				if(bkb_sleep_count<=0)
				{
					// Дождались-таки!
					*bm=BKB_MODE_NONE;
					Place();
					transparency=60;
					bkb_sleep_count=BKB_SLEEP_COUNT;
				}
			}
			else bkb_sleep_count=BKB_SLEEP_COUNT;
			PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);

			return true; // В частности, не выводит увеличительное стекло; режим не меняется
		}
		
		// Включение режима сна (сюда также добавлена деактивация клавиатуры)
		if(BKB_MODE_SLEEP==tool_config[tool_candidate].bkb_mode)
		{
			bkb_sleep_count--;
			if(bkb_sleep_count<=0)
			{
				// Специальные действия с клавиатурой
				if(BKB_MODE_KEYBOARD==*bm)	BKBKeybWnd::DeActivate();	// деактивировать клавиатуру

				*bm=BKB_MODE_SLEEP;
				bkb_sleep_count=BKB_SLEEP_COUNT;
				current_tool=-1; // ничего не подсвечивать
				if(gBKB_SLEEP_IN_BLACK) {transparency=100; Place();}
				else transparency=20;
			}
			PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
			return true; // В частности, не выводит увеличительное стекло;
		}
		
		// Промах мимо кнопки сна вызывает
		bkb_sleep_count=BKB_SLEEP_COUNT;
		PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
		
		//===================================================================================
		// Конец (Очень многого про кнопку сна)
		//===================================================================================




		// Добавление контролов и шифтов с альтами к кликам мыши
		if((current_tool>=0)&&(gBKB_SHOW_CLICK_MODS))
			if(tool_config[current_tool].flag_modifiers&&(tool_candidate>current_tool)&&(tool_candidate<=current_tool+4))
		{
			// Попали в модификаторы кликов
			int modif_number=tool_candidate-current_tool-1; // Какой из модификаторов поменять
			if(tool_modifier[modif_number]) tool_modifier[modif_number]=false;
			else tool_modifier[modif_number]=true;

			// Пусть окно перерисует стандартная оконная процедура
			PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);

			return true; // В частности, не выводит увеличительное стекло
		}

		
		// Перенести панель инструментов в другую половину экрана - предпоследняя кнопка
		if(BKB_MODE_SWAP==tool_config[tool_candidate].bkb_mode)
		{
			if(left_side)
			{
				left_side=false;
				//MoveWindow(Tlhwnd, screen_x-gBKB_TOOLBOX_WIDTH,0,gBKB_TOOLBOX_WIDTH,screen_y,TRUE);
				Place();
			}
			else
			{
				left_side=true;
				//MoveWindow(Tlhwnd, 0,0,gBKB_TOOLBOX_WIDTH,screen_y,TRUE);
				Place();

			}
			BKBKeybWnd::Place();
			return true; // В частности, не выводит увеличительное стекло; режим не меняется
		}

		//

		// а ещё нельзя включать скролл, когда работает клавиатура (легко промахнуться и нажать его вместо клавиши)
		// ОТМЕНЕНО
		//if((BKB_MODE_KEYBOARD==*bm)&&(tool_candidate>=4)) return false; 

		// Специальные действия с клавиатурой
		if(BKB_MODE_KEYBOARD==*bm)	{BKBKeybWnd::DeActivate();  }	// деактивировать клавиатуру
		
		// Действие по кмолчанию. Режим соответствует кнопке
		// если этот инструмент уже был выбран, деактивируем его
		if(tool_candidate==current_tool)
		{
			current_tool=-1;
			*bm=BKB_MODE_NONE;
			Place(); // при возврате из режима клавиатуры нужен Place при уже новом значении *bm
		}
		else // замена одного инструмента на другой
		{
			Fixation::drag_in_progress=false; // перестраховка на предмет незавершенного дрега
			current_tool=tool_candidate;
			*bm=tool_config[tool_candidate].bkb_mode;
			// Специальные действия с клавиатурой и черепашкой
			if(BKB_MODE_KEYBOARD==*bm) {BKBKeybWnd::Activate(); Place(); }	// активировать клавиатуру
			if(BKB_MODE_TURTLE==*bm)
			{
				// Если режим аэромыши - включить курсор !!
				if(2==tracking_device) BKBTranspWnd::Show();
				Place(); 
				BKBTurtle::Place();
				BKBTurtle::Show(SW_SHOW);
				flag_continuous_turtle=false; // Непрерывный взгляд на стрелку черепашки соскочил
				BKBTurtle::swap_step=0; // Каким бы ни был шаг фиксации - он слетел
			}
		}



		// сбросим контрол-шифт-альт модификаторы
		tool_modifier[0]=false;
		tool_modifier[1]=false;
		tool_modifier[2]=false;
		tool_modifier[3]=false;
		// tool_modifier[4]=false; - нет более такого

		// Пусть окно перерисует стандартная оконная процедура
		 PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
		
		//RECT rect={0,0,BKB_TOOLBOX_WIDTH,screen_y};
		//InvalidateRect(Tlhwnd,&rect,TRUE);
		return true;
	}
	else // не попали - снова контроль включения/выключения сна.
	{
		if((bkb_sleep_count<BKB_SLEEP_COUNT)&&(bkb_sleep_count>0))
		{
			bkb_sleep_count=BKB_SLEEP_COUNT; // если были в состоянии сна, но недодержали 5 секунд, они опять перевзводятся
			PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
		}
		return false;
	}
}

//================================================================
// Текущий режим отработал, сбрасывай всё
//================================================================
void BKBToolWnd::Reset(BKB_MODE *bm)
{
	current_tool=-1;
	*bm=BKB_MODE_NONE;

	// На всякий случай
	tool_modifier[0]=false;
	tool_modifier[1]=false;
	tool_modifier[2]=false;
	tool_modifier[3]=false;
	// tool_modifier[4]=false; - нет его более

	// Пусть окно перерисует стандартная оконная процедура
	 PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);

		
}

//=======================================================================
// В режиме скролла показывает курсор только когда он попадает на тулбар
//=======================================================================
void BKBToolWnd::ScrollCursor(POINT *p)
{
	static bool mouse_inside_toolbar=true, last_mouse_inside_toolbar=true; // Для скрытия второго курсора при перемещении за область тулбара
	
	if((left_side&&(p->x<gBKB_TOOLBOX_WIDTH)) || !left_side&&(p->x>screenX-gBKB_TOOLBOX_WIDTH))
	{
		// Попали в тулбокс, покажите курсор
		mouse_inside_toolbar=true;
		if(false==last_mouse_inside_toolbar) BKBTranspWnd::Show(); // Показать стрелку
		BKBTranspWnd::Move(p->x,p->y);
	}
	else
	{
		// мимо тулбара
		mouse_inside_toolbar=false;
		if(true==last_mouse_inside_toolbar) BKBTranspWnd::Hide(); // Убрать стрелку
	}
	last_mouse_inside_toolbar=mouse_inside_toolbar;
}

//================================================================
// Уведомляем о движениях мыши на случай сна
//================================================================
void BKBToolWnd::SleepCheck(POINT *_pnt)
{
	POINT pnt=*_pnt;
	if((bkb_sleep_count<BKB_SLEEP_COUNT)&&(bkb_sleep_count>0)) // Засыпаем или просыпаемся, мышь уводить с кнопки нельзя
	{
		ScreenToClient(Tlhwnd,&pnt);

		// Спец. режим - сон в темноте
		if(gBKB_SLEEP_IN_BLACK&&BKB_MODE_SLEEP==Fixation::CurrentMode()) 
		{
			RECT crect;
			GetClientRect(Tlhwnd,&crect);

			if((pnt.y>=screenY-gBKB_TOOLBOX_WIDTH)&&(pnt.x>=crect.right-gBKB_TOOLBOX_WIDTH)) return; // Всё в порядке, мышь не елозит, продолжайте..
			else
			{
				bkb_sleep_count=BKB_SLEEP_COUNT; // были в состоянии сна, но недодержали 5 секунд, они опять перевзводятся
				PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
			}
		}


		if((pnt.x>=0)&&(pnt.x<gBKB_TOOLBOX_WIDTH)&&(pnt.y>0)&&(pnt.y<height))
		{
			// попала, определяем номер инструмента
			int position=pnt.y/gBKB_TOOLBOX_WIDTH; // Высота и ширина совпадают
			int tool_candidate=ToolFromPosition(position);
			if(tool_candidate>=0) // это не стрелка?
			{
				if(BKB_MODE_SLEEP==tool_config[tool_candidate].bkb_mode) // Попали ли в засыпалку?
				{
					// Всё в порядке, мышь не елозит, продолжайте...
					return;
				}
			
			}
		}

		// Сбросить bkb_sleep_count
		bkb_sleep_count=BKB_SLEEP_COUNT; // были в состоянии сна, но недодержали 5 секунд, они опять перевзводятся
		PostMessage(Tlhwnd, WM_USER_INVALRECT, 0, 0);
	}
}

//=======================================================================================================================
// Изменяем форму и положение тулбара в зависимости от режима и правого/левого расположения
// (а также верхнего/нижнего расположения клавиатуры)
//=======================================================================================================================
void BKBToolWnd::Place()
{
	// Всякий раз сызнова вычисляем..
	gBKB_TOOLBOX_WIDTH=screenY/gBKB_TOOLBOX_BUTTONS;
	
	if(BKB_MODE_TURTLE==Fixation::CurrentMode()) // Спец. случай - черепашка
	{
		// Количество кнопок в режиме черепашки 4 или 5 в зависимости от числа кнопок в основном тулбаре
		if(gBKB_TOOLBOX_BUTTONS>4) BKB_TURTLE_BUTTONS_VISIBLE=5;
		else BKB_TURTLE_BUTTONS_VISIBLE=4;

		gBKB_TOOLBOX_WIDTH=screenY/BKB_TURTLE_BUTTONS_VISIBLE;
	}

	// Спец. случай - сон в темноте?
	if(gBKB_SLEEP_IN_BLACK&&BKB_MODE_SLEEP==Fixation::CurrentMode()) 
	{
		MoveWindow(Tlhwnd, 0,0, screenX,screenY,TRUE);
		return;
	}

	// Сначала проверяем, уж не клавиши ли выбраны? В этом слкчае вырожденное окно получаем
	if(BKB_MODE_KEYBOARD==Fixation::CurrentMode())
	{
		// 2 или одна кнопка в зависимости от step (шага зумирования клавиатуры)
		if(0==BKBKeybWnd::step) height=2*gBKB_TOOLBOX_WIDTH; // Две кнопки в шаге 0 клавиатуры
		else height=gBKB_TOOLBOX_WIDTH; // Одна только кнопка при зумировании 

		if(!BKBKeybWnd::bottom_side) // Клавиатура вверху, переносим тулбар вниз
			place_point.y=screenY-height;
	}
	else // Нет, не клавиатура 
	{
		height=screenY; // Да, вот так просто.
	}
	
	if(!left_side) place_point.x=screenX-gBKB_TOOLBOX_WIDTH;
	else place_point.x=0;

	MoveWindow(Tlhwnd, place_point.x,place_point.y,gBKB_TOOLBOX_WIDTH,height,TRUE);

}

static RECT pink_rect;
//=======================================================================================================================
// Будем рисовать розовый прямоугольничек на тулбаре
//=======================================================================================================================
LPRECT BKBToolWnd::PinkFrame(int _x, int _y)
{
	// Попала ли точка в границы окна?
	POINT pnt={_x,_y};
	RECT crect;
	GetClientRect(Tlhwnd,&crect);
	ScreenToClient(Tlhwnd,&pnt);
	if((pnt.x>=0)&&(pnt.x<crect.right)&&(pnt.y>0)&&(pnt.y<height))
	{
		// Попала, осталось только найти, в какую ячейку
		int cell_num=pnt.y/gBKB_TOOLBOX_WIDTH;

		// Во сне подсвечиваем только кнопку сна
		if(BKB_MODE_SLEEP==Fixation::CurrentMode()) // Да, мы в режиме сна
		{
			// Спец.случай: Здесь возможно большое чёрное окно
			if(gBKB_SLEEP_IN_BLACK)
			{
				// Попали ли мы в нижний правый угол экрана?
				if((pnt.y>=screenY-gBKB_TOOLBOX_WIDTH)&&(pnt.x>=crect.right-gBKB_TOOLBOX_WIDTH))
				{
					pink_rect.left=crect.right-gBKB_TOOLBOX_WIDTH;
					pink_rect.right=crect.right;
					pink_rect.top=screenY-gBKB_TOOLBOX_WIDTH;;
					pink_rect.bottom=screenY;
		
					return &pink_rect;
				}
				else return NULL;
			}

			// Подсвечивать только Sleep
			int tool_candidate=ToolFromPosition(cell_num);
			if(0<=tool_candidate) // Стрелки не являются кандидатами
			{
				if(BKB_MODE_SLEEP!=tool_config[tool_candidate].bkb_mode) return NULL;
			}
			else return NULL;
		}

		pink_rect.left=place_point.x;
		pink_rect.right=place_point.x+gBKB_TOOLBOX_WIDTH-1;
		pink_rect.top=place_point.y+gBKB_TOOLBOX_WIDTH*cell_num;
		pink_rect.bottom=place_point.y+gBKB_TOOLBOX_WIDTH*(cell_num+1);
		
		return &pink_rect;
	}
	else return NULL;
}

//===========================================================================================
// Устанавливает подсветку нужной клавиши в зависимости от режима
//===========================================================================================
void BKBToolWnd::SetCurrentTool(BKB_MODE bm)
{
	int i;
	
	current_tool=-1;

	for(i=0;i<BKB_NUM_TOOLS;i++)
	{
		if(bm==tool_config[i].bkb_mode)
		{
			current_tool=i;
			return;
		}
	}
}

