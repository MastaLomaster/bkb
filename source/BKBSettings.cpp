#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "BKBSettings.h"
#include "ToolWnd.h"
#include "BKBRepErr.h"
#include "TranspWnd.h"
#include "Internat.h"
#include "KeybWnd.h"
#include "BKBMetricsWnd.h"

extern HINSTANCE	BKBInst;

HWND BKBSettings::settings_hwnd=0;
HWND BKBSettings::parent_hwnd=0;

static char char_buf[4096];
static TCHAR tchar_buf[4096]; // Для сообщений об ошибках
static TCHAR *tfilename=L"config.bkb";

const int BKB_MOUSE_MULTIPLIERS=7;
BKBIntChar dlg_mouse_x_multiplier[BKB_MOUSE_MULTIPLIERS]={{L"не усиливать",10,IDC_RADIO_X_BUTTON1},{L"1.5",15,IDC_RADIO_X_BUTTON2},{L"2.0",20,IDC_RADIO_X_BUTTON3},{L"2.5",25,IDC_RADIO_X_BUTTON4},{L"3.0",30,IDC_RADIO_X_BUTTON5},{L"3.5",35,IDC_RADIO_X_BUTTON6},{L"4.0",40,IDC_RADIO_X_BUTTON7}}; 
BKBIntChar dlg_mouse_y_multiplier[BKB_MOUSE_MULTIPLIERS]={{L"не усиливать",10,IDC_RADIO_Y_BUTTON1},{L"1.5",15,IDC_RADIO_Y_BUTTON2},{L"2.0",20,IDC_RADIO_Y_BUTTON3},{L"2.5",25,IDC_RADIO_Y_BUTTON4},{L"3.0",30,IDC_RADIO_Y_BUTTON5},{L"3.5",35,IDC_RADIO_Y_BUTTON6},{L"4.0",40,IDC_RADIO_Y_BUTTON7}}; 
int dlg_current_mouse_x_multiplier=0, dlg_current_mouse_y_multiplier=0;
extern int g_BKB_MOUSE_X_MULTIPLIER, g_BKB_MOUSE_Y_MULTIPLIER; // Определены в AirMouse.cpp

const int BKB_FIXATIONS=9;
BKBIntChar dlg_btnfixation[BKB_FIXATIONS]={{L"10",10,IDC_RADIO_FIXATION1},{L"15",15,IDC_RADIO_FIXATION2},{L"20",20,IDC_RADIO_FIXATION3},{L"25",25,IDC_RADIO_FIXATION4},{L"30",30,IDC_RADIO_FIXATION5},{L"45",45,IDC_RADIO_FIXATION6},{L"60",60,IDC_RADIO_FIXATION7},{L"90",90,IDC_RADIO_FIXATION8},{L"120",120,IDC_RADIO_FIXATION9}};
BKBIntChar dlg_postfixation[BKB_FIXATIONS]={{L"10",10,IDC_RADIO_POSTFIXATION1},{L"15",15,IDC_RADIO_POSTFIXATION2},{L"20",20,IDC_RADIO_POSTFIXATION3},{L"25",25,IDC_RADIO_POSTFIXATION4},{L"30",30,IDC_RADIO_POSTFIXATION5},{L"45",45,IDC_RADIO_POSTFIXATION6},{L"60",60,IDC_RADIO_POSTFIXATION7},{L"90",90,IDC_RADIO_POSTFIXATION8},{L"120",120,IDC_RADIO_POSTFIXATION9}};
BKBIntChar dlg_nokbdfixation[BKB_FIXATIONS]={{L"10",10,IDC_RADIO_NKBD_FIXATION1},{L"15",15,IDC_RADIO_NKBD_FIXATION2},{L"20",20,IDC_RADIO_NKBD_FIXATION3},{L"25",25,IDC_RADIO_NKBD_FIXATION4},{L"30",30,IDC_RADIO_NKBD_FIXATION5},{L"45",45,IDC_RADIO_NKBD_FIXATION6},{L"60",60,IDC_RADIO_NKBD_FIXATION7},{L"90",90,IDC_RADIO_NKBD_FIXATION8},{L"120",120,IDC_RADIO_NKBD_FIXATION9}};
BKBIntChar dlg_nokbdpostfixation[BKB_FIXATIONS]={{L"10",10,IDC_RADIO_NKBD_POSTFIXATION1},{L"15",15,IDC_RADIO_NKBD_POSTFIXATION2},{L"20",20,IDC_RADIO_NKBD_POSTFIXATION3},{L"25",25,IDC_RADIO_NKBD_POSTFIXATION4},{L"30",30,IDC_RADIO_NKBD_POSTFIXATION5},{L"45",45,IDC_RADIO_NKBD_POSTFIXATION6},{L"60",60,IDC_RADIO_NKBD_POSTFIXATION7},{L"90",90,IDC_RADIO_NKBD_POSTFIXATION8},{L"120",120,IDC_RADIO_NKBD_POSTFIXATION9}};
int dlg_current_btnfixation=6, dlg_current_postfixation=4,dlg_current_nokbdfixation=6, dlg_current_nokbdpostfixation=4;

const int BKB_SET_TOOLBARCELLS=6;
BKBIntChar dlg_toolbarcells[BKB_SET_TOOLBARCELLS]={{L"4",4,IDC_RADIO_TOOLBAR4}, {L"5",5,IDC_RADIO_TOOLBAR5}, {L"6",6,IDC_RADIO_TOOLBAR6}, {L"7",7,IDC_RADIO_TOOLBAR7}, {L"8",8,IDC_RADIO_TOOLBAR8}, {L"9",9,IDC_RADIO_TOOLBAR9}};
int dlg_current_toolbarcells=3;

const int BKB_YESNO=2;
BKBIntChar dlg_kbd_fullscreen[BKB_YESNO]={{L"Нет",0,IDC_RADIO_KBDFS1},{L"Да",1,IDC_RADIO_KBDFS2}}; 
BKBIntChar dlg_kbd_2step[BKB_YESNO]={{L"Нет",0,IDC_RADIO_KBD_2STEPS_NO},{L"Да",1,IDC_RADIO_KBD_2STEPS_YES}}; 
BKBIntChar dlg_show_clickmods[BKB_YESNO]={{L"Нет",0,IDC_RADIO_CLICKMOD_NO},{L"Да",1,IDC_RADIO_CLICKMOD_YES}}; 
BKBIntChar dlg_show_metrics[BKB_YESNO]={{L"Нет",0,IDC_RADIO_METRICS_NO},{L"Да",1,IDC_RADIO_METRICS_YES}}; 


int dlg_current_kbd_fullscreen=1;
int dlg_current_kbd_2step=0;
int dlg_current_show_clickmods=0;
int dlg_current_show_metrics=0;

const int BKB_SET_MBUTTONFIX=3;
BKBIntChar dlg_mbuttonfix[BKB_SET_MBUTTONFIX]={{L"только фиксация",0,IDC_RADIO_MBUTTONFIX1}, {L"и мышь, и фиксация",1,IDC_RADIO_MBUTTONFIX2}, {L"только мышь",2,IDC_RADIO_MBUTTONFIX3}};
int dlg_current_mbuttonfix=1;

const int BKB_SET_DISPERSION=4;
BKBIntChar dlg_dispersion[BKB_SET_DISPERSION]={{L"10",10,IDC_RADIO_DISP1}, {L"15",15,IDC_RADIO_DISP2}, {L"20",20,IDC_RADIO_DISP3}, {L"25",25,IDC_RADIO_DISP4}};
int dlg_current_dispersion=0;

extern int FIXATION_LIMIT; // Сколько последовательных точек с низкой дисперсией считать фиксацией
extern int POSTFIXATION_SKIP; // сколько точек пропустить после фиксации, чтобы начать считать новую фиксацию
extern int NOTKBD_FIXATION_LIMIT; // Сколько последовательных точек с низкой дисперсией считать фиксацией (не для клавиатуры)
extern int NOTKBD_POSTFIXATION_SKIP;

extern int gBKB_FullSizeKBD; // Флаг того, что клавиатура занимает всю ширину экрана
extern bool gBKB_2STEP_KBD_MODE; // Флаг того, что клавиши нажимаются в два приёма
bool gBKB_SHOW_CLICK_MODS=false; // Флаг того, что нужно показывать модификаторы клика (+Ctrl, +Shift, ...)
extern int gBKB_TOOLBOX_BUTTONS; // Количество видимых кнопок на панели инструментов
int gBKB_MBUTTONFIX=1;
int gBKB_SHOW_METRICS=0; // Показывать ли окно метрик
int gBKB_DISP_PERCENT=10; // процент высоты экрана, который задаёт границы дисперсии

//===================================================================
// Диалог настроек
//===================================================================
static BOOL CALLBACK DlgSettingsWndProc(HWND hdwnd,
						   UINT uMsg,
						   WPARAM wparam,
						   LPARAM lparam )
{
	
	switch(uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wparam))
			{
			case IDCANCEL: // Не случилось, 
				if(BKBSettings::parent_hwnd) EndDialog(hdwnd,0); // диалог был вызван в начале программы
				ShowWindow(hdwnd,SW_HIDE); // диалог был вызван во время исполнения программы
				return 1;
	
			case IDOK: 	//Хорошо!
				//ShowWindow(hdwnd,SW_HIDE);
				BKBSettings::Screen2Load(hdwnd);
				BKBSettings::ActualizeLoad();
				BKBSettings::SaveBKBConfig();
				// Переделать клавиатуру при изменившихся параметрах
				BKBKeybWnd::Place();
				// А также ToolBar
				BKBToolWnd::offset=0; // пока-что насильно перематываем в начало
				BKBToolWnd::Place();
				BKBMetricsWnd::Show(gBKB_SHOW_METRICS);
				if(BKBSettings::parent_hwnd) EndDialog(hdwnd,0); // диалог был вызван в начале программы
				ShowWindow(hdwnd,SW_HIDE); // диалог был вызван во время исполнения программы
				return 1; 

			} // switch WM_COMMAND
		
		BKBTranspWnd::ToTop(); // После нажатия прозрачное окно уходит вниз
		break; // if WM_COMMAND 

	case WM_INITDIALOG:
		SetWindowPos(hdwnd,HWND_TOP,150,150,0,0,SWP_NOSIZE | SWP_NOREDRAW);
			
		BKBSettings::PrepareDialogue(hdwnd); // Заполняет списки
		BKBSettings::ShowLoad(hdwnd); // Показываем текущие значения

		return 1; // Обработали
		break;

	case WM_CLOSE:
		ShowWindow(hdwnd,SW_HIDE);
        return 1; // Обработали, т.е. забили
		break;

	case WM_DESTROY:
		BKBSettings::settings_hwnd=0;
		return 1;
		break;

	} // switch uMsg
 
return 0;
}

//===========================================================================
// Вызов диалога настроек
//===========================================================================
void BKBSettings::SettingsDialogue(HWND _parent_hwnd)
{
	parent_hwnd=_parent_hwnd;

	// Вызов диалога в начале работы программы, ждём его завершения
	if(parent_hwnd)
	{
		DialogBox(BKBInst,MAKEINTRESOURCE(IDD_SETTINGS), parent_hwnd,(DLGPROC)DlgSettingsWndProc);
		return;
	}

	// Вызов диалога при работе программы
	if(!settings_hwnd)
	{
		settings_hwnd=CreateDialog(BKBInst, MAKEINTRESOURCE(IDD_SETTINGS), parent_hwnd , (DLGPROC)DlgSettingsWndProc);
		ShowWindow(settings_hwnd, SW_SHOW);
	}
	else
	{
		ShowWindow(settings_hwnd, SW_SHOW);
		// При нажатии на "отмена" могли остаться несохранённые значения, поэтому переустанавливаем текущие значения
		ShowLoad(settings_hwnd);
	}
}

//====================================================================================
// Заполнить выпадающие списки диалога возможными значениями
//====================================================================================
void BKBSettings::PrepareDialogue(HWND hdwnd)
{
	// Перевод заголовка окна
	if(Internat::Message(38,0)) SetWindowText(hdwnd,Internat::Message(38,0));

	// Перевод текста про усилители мыши (идентификаторы в двух диалогах совпадают)
	if(Internat::Message(39,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_MOVEHARD, WM_SETTEXT, 0L, (LPARAM)Internat::Message(39,0)); // Усилитель мыши для тех, кому трудно сильно поворачивать голову с аэромышью
	if(Internat::Message(40,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_X_MULTIPLIER, WM_SETTEXT, 0L, (LPARAM)Internat::Message(40,0)); // Усиление по горизонтали:
	if(Internat::Message(41,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_Y_MULTIPLIER, WM_SETTEXT, 0L, (LPARAM)Internat::Message(41,0)); // Усиление по вертикали:

	if(Internat::Message(44,0)) 
	{
		SendDlgItemMessage(hdwnd,IDC_RADIO_X_BUTTON1, WM_SETTEXT, 0L, (LPARAM)Internat::Message(44,0)); // не усиливать
		SendDlgItemMessage(hdwnd,IDC_RADIO_Y_BUTTON1, WM_SETTEXT, 0L, (LPARAM)Internat::Message(44,0)); // не усиливать
	}

	if(Internat::Message(46,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_TIMINGS, WM_SETTEXT, 0L, (LPARAM)Internat::Message(46,0)); // Время задано в количестве отсчетов устройства (обычно 30 отсчетов в секунду)
	if(Internat::Message(47,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_FIXATION_LIMIT, WM_SETTEXT, 0L, (LPARAM)Internat::Message(47,0));// Время нажатия на кнопки клавиатуры
	if(Internat::Message(48,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_POSTFIXATION_SKIP, WM_SETTEXT, 0L, (LPARAM)Internat::Message(48,0));// Пауза между нажатиями на кнопки клавиатуры

	if(Internat::Message(55,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_NKBD_FIXATION_LIMIT, WM_SETTEXT, 0L, (LPARAM)Internat::Message(55,0));// Время фиксации на других элементах
	if(Internat::Message(56,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_NKBD_POSTFIXATION_SKIP, WM_SETTEXT, 0L, (LPARAM)Internat::Message(56,0));// Пауза между нажатиями на кнопки клавиатуры

	if(Internat::Message(57,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_KBDFS, WM_SETTEXT, 0L, (LPARAM)Internat::Message(57,0));// Клавиатура во всю ширину экрана
	if(Internat::Message(58,0)) // Нет
	{
		SendDlgItemMessage(hdwnd,IDC_RADIO_KBDFS1, WM_SETTEXT, 0L, (LPARAM)Internat::Message(58,0));// Нет
		SendDlgItemMessage(hdwnd,IDC_RADIO_KBD_2STEPS_NO, WM_SETTEXT, 0L, (LPARAM)Internat::Message(58,0));// Нет
		SendDlgItemMessage(hdwnd,IDC_RADIO_CLICKMOD_NO, WM_SETTEXT, 0L, (LPARAM)Internat::Message(58,0));// Нет
		SendDlgItemMessage(hdwnd,IDC_RADIO_METRICS_NO, WM_SETTEXT, 0L, (LPARAM)Internat::Message(58,0));// Нет
	}
	if(Internat::Message(59,0))
	{
		SendDlgItemMessage(hdwnd,IDC_RADIO_KBDFS2, WM_SETTEXT, 0L, (LPARAM)Internat::Message(59,0));// Да
		SendDlgItemMessage(hdwnd,IDC_RADIO_KBD_2STEPS_YES, WM_SETTEXT, 0L, (LPARAM)Internat::Message(59,0));// Да
		SendDlgItemMessage(hdwnd,IDC_RADIO_CLICKMOD_YES, WM_SETTEXT, 0L, (LPARAM)Internat::Message(59,0));// Да
		SendDlgItemMessage(hdwnd,IDC_RADIO_METRICS_YES, WM_SETTEXT, 0L, (LPARAM)Internat::Message(59,0));// Да
	}

	if(Internat::Message(49,0)) SendDlgItemMessage(hdwnd,IDOK, WM_SETTEXT, 0L, (LPARAM)Internat::Message(49,0));// Сохранить
	if(Internat::Message(50,0)) SendDlgItemMessage(hdwnd,IDCANCEL, WM_SETTEXT, 0L, (LPARAM)Internat::Message(50,0));// Отмена

	if(Internat::Message(63,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_TOOLBAR_NUM, WM_SETTEXT, 0L, (LPARAM)Internat::Message(63,0));// Кнопок в панели инструментов
	if(Internat::Message(64,0)) // сек.
	{
		SendDlgItemMessage(hdwnd,IDC_STATIC_SEC1, WM_SETTEXT, 0L, (LPARAM)Internat::Message(64,0));// сек.
		SendDlgItemMessage(hdwnd,IDC_STATIC_SEC2, WM_SETTEXT, 0L, (LPARAM)Internat::Message(64,0));// сек.
		SendDlgItemMessage(hdwnd,IDC_STATIC_SEC3, WM_SETTEXT, 0L, (LPARAM)Internat::Message(64,0));// сек.
		SendDlgItemMessage(hdwnd,IDC_STATIC_SEC4, WM_SETTEXT, 0L, (LPARAM)Internat::Message(64,0));// сек.
		SendDlgItemMessage(hdwnd,IDC_STATIC_SEC5, WM_SETTEXT, 0L, (LPARAM)Internat::Message(64,0));// сек.
		SendDlgItemMessage(hdwnd,IDC_STATIC_SEC6, WM_SETTEXT, 0L, (LPARAM)Internat::Message(64,0));// сек.
	}
	if(Internat::Message(65,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_30FPS_TRACKER, WM_SETTEXT, 0L, (LPARAM)Internat::Message(65,0));// Трекер с 30 отсчетами в секунду
	if(Internat::Message(66,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_60FPS_TRACKER, WM_SETTEXT, 0L, (LPARAM)Internat::Message(66,0));// Трекер с 60 отсчетами в секунду

	if(Internat::Message(67,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_KBD_2STEPS, WM_SETTEXT, 0L, (LPARAM)Internat::Message(67,0));// Клавиатура двумя шагами
	if(Internat::Message(68,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_CLICKMOD, WM_SETTEXT, 0L, (LPARAM)Internat::Message(68,0));// Модификаторы кликов (+Ctrl,..)

	if(Internat::Message(69,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_MBUTTON, WM_SETTEXT, 0L, (LPARAM)Internat::Message(69,0));// Модификаторы кликов (+Ctrl,..)

	if(Internat::Message(70,0)) SendDlgItemMessage(hdwnd,IDC_RADIO_MBUTTONFIX1, WM_SETTEXT, 0L, (LPARAM)Internat::Message(70,0));// Только фиксация
	if(Internat::Message(71,0)) SendDlgItemMessage(hdwnd,IDC_RADIO_MBUTTONFIX2, WM_SETTEXT, 0L, (LPARAM)Internat::Message(71,0));// Только фиксация
	if(Internat::Message(72,0)) SendDlgItemMessage(hdwnd,IDC_RADIO_MBUTTONFIX3, WM_SETTEXT, 0L, (LPARAM)Internat::Message(72,0));// Только фиксация
	
	if(Internat::Message(74,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_SHOW_METRICS, WM_SETTEXT, 0L, (LPARAM)Internat::Message(74,0));// Показывать окно с метриками
	if(Internat::Message(75,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_DISPERSION, WM_SETTEXT, 0L, (LPARAM)Internat::Message(75,0));// Фиксация при разбросе координат не более (% от высоты экрана)
	
}

//=======================================================================================
// Показать в полях диалога загруженнные значения переменных
//=======================================================================================
void BKBSettings::ShowLoad(HWND hdwnd)
{
	int i;

	// Заполнить выпадающие списки с текущими значениями и нажать все радиокнопки!
	// 1. Усилители мыши. 
	// 1.1. Сначала отпускаем все радиокнопки
	for(i=0;i<BKB_MOUSE_MULTIPLIERS;i++)
	{
		SendDlgItemMessage(hdwnd, dlg_mouse_x_multiplier[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, dlg_mouse_y_multiplier[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	// 1.2 Нажать те, что надо
	SendDlgItemMessage(hdwnd, dlg_mouse_x_multiplier[dlg_current_mouse_x_multiplier].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_mouse_y_multiplier[dlg_current_mouse_y_multiplier].button, BM_SETCHECK, BST_CHECKED, 0);
		

	// 2. Времена нажатия
	// 2.1. Сначала отпускаем все радиокнопки
	for(i=0;i<BKB_FIXATIONS;i++)
	{
		SendDlgItemMessage(hdwnd, dlg_btnfixation[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, dlg_postfixation[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, dlg_nokbdfixation[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, dlg_nokbdpostfixation[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	// 1.2 Нажать те, что надо
	SendDlgItemMessage(hdwnd, dlg_btnfixation[dlg_current_btnfixation].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_postfixation[dlg_current_postfixation].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_nokbdfixation[dlg_current_nokbdfixation].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_nokbdpostfixation[dlg_current_nokbdpostfixation].button, BM_SETCHECK, BST_CHECKED, 0);


	// 3. Клавиатура на полный экран / в два нажатия / показывать "+Ctrl" / показывать метрики
	for(i=0;i<BKB_YESNO;i++)
	{
		SendDlgItemMessage(hdwnd, dlg_kbd_fullscreen[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, dlg_kbd_2step[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, dlg_show_clickmods[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
		SendDlgItemMessage(hdwnd, dlg_show_metrics[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	SendDlgItemMessage(hdwnd, dlg_kbd_fullscreen[dlg_current_kbd_fullscreen].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_kbd_2step[dlg_current_kbd_2step].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_show_clickmods[dlg_current_show_clickmods].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_show_metrics[dlg_current_show_metrics].button, BM_SETCHECK, BST_CHECKED, 0);

	// 4. Количество кнопок на тулбаре
	for(i=0;i<BKB_SET_TOOLBARCELLS;i++)
	{
		SendDlgItemMessage(hdwnd, dlg_toolbarcells[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	SendDlgItemMessage(hdwnd, dlg_toolbarcells[dlg_current_toolbarcells].button, BM_SETCHECK, BST_CHECKED, 0);

	// 5. Фиксация средней кнопкой мыши
	for(i=0;i<BKB_SET_MBUTTONFIX;i++)
	{
		SendDlgItemMessage(hdwnd, dlg_mbuttonfix[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	SendDlgItemMessage(hdwnd, dlg_mbuttonfix[dlg_current_mbuttonfix].button, BM_SETCHECK, BST_CHECKED, 0);

	// 6. Граница дисперсии в процентах от высоты экрана
	for(i=0;i<BKB_SET_DISPERSION;i++)
	{
		SendDlgItemMessage(hdwnd, dlg_dispersion[i].button, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	SendDlgItemMessage(hdwnd, dlg_dispersion[dlg_current_dispersion].button, BM_SETCHECK, BST_CHECKED, 0);
	
}



//=======================================================================================
// Актуализировать загруженнные значения в реальные переменные программы
//=======================================================================================
void BKBSettings::ActualizeLoad()
{
	g_BKB_MOUSE_X_MULTIPLIER=dlg_mouse_x_multiplier[dlg_current_mouse_x_multiplier].value;
	g_BKB_MOUSE_Y_MULTIPLIER=dlg_mouse_y_multiplier[dlg_current_mouse_y_multiplier].value;

	FIXATION_LIMIT=dlg_btnfixation[dlg_current_btnfixation].value;
	POSTFIXATION_SKIP=dlg_postfixation[dlg_current_postfixation].value;

	NOTKBD_FIXATION_LIMIT=dlg_nokbdfixation[dlg_current_nokbdfixation].value;
	NOTKBD_POSTFIXATION_SKIP=dlg_nokbdpostfixation[dlg_current_nokbdpostfixation].value;

	gBKB_FullSizeKBD=dlg_kbd_fullscreen[dlg_current_kbd_fullscreen].value;
	gBKB_2STEP_KBD_MODE=(bool)(dlg_kbd_2step[dlg_current_kbd_2step].value);
	gBKB_SHOW_CLICK_MODS=(bool)(dlg_show_clickmods[dlg_current_show_clickmods].value);
	gBKB_TOOLBOX_BUTTONS=dlg_toolbarcells[dlg_current_toolbarcells].value;
	gBKB_MBUTTONFIX=dlg_mbuttonfix[dlg_current_mbuttonfix].value;
	gBKB_SHOW_METRICS=dlg_show_metrics[dlg_current_show_metrics].value;
	gBKB_DISP_PERCENT=dlg_dispersion[dlg_current_dispersion].value;
}


//===============================================================================================
// Копирует из полей диалога в реальные переменные
//===============================================================================================
void BKBSettings::Screen2Load(HWND hdwnd)
{
	int i;

	// 1. Усилители мыши
	for(i=0;i<BKB_MOUSE_MULTIPLIERS;i++)
	{

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_mouse_x_multiplier[i].button, BM_GETCHECK, 0, 0))
			dlg_current_mouse_x_multiplier=i;

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_mouse_y_multiplier[i].button, BM_GETCHECK, 0, 0))
			dlg_current_mouse_y_multiplier=i;
	}

	

	// 2. Временные задержки
	for(i=0;i<BKB_FIXATIONS;i++)
	{

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_btnfixation[i].button, BM_GETCHECK, 0, 0))
			dlg_current_btnfixation=i;

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_postfixation[i].button, BM_GETCHECK, 0, 0))
			dlg_current_postfixation=i;

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_nokbdfixation[i].button, BM_GETCHECK, 0, 0))
			dlg_current_nokbdfixation=i;

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_nokbdpostfixation[i].button, BM_GETCHECK, 0, 0))
			dlg_current_nokbdpostfixation=i;
	}

	// 3. Клавиатура на полный экран / 2 шага клавиатуры / +Ctrl / показывать метрики
	for(i=0;i<BKB_YESNO;i++)
	{

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_kbd_fullscreen[i].button, BM_GETCHECK, 0, 0))
			dlg_current_kbd_fullscreen=i;

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_kbd_2step[i].button, BM_GETCHECK, 0, 0))
			dlg_current_kbd_2step=i;
		
		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_show_clickmods[i].button, BM_GETCHECK, 0, 0))
			dlg_current_show_clickmods=i;

		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_show_metrics[i].button, BM_GETCHECK, 0, 0))
			dlg_current_show_metrics=i;
	}

	// 4. Количество кнопок на тулбаре
	for(i=0;i<BKB_SET_TOOLBARCELLS;i++)
	{
		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_toolbarcells[i].button, BM_GETCHECK, 0, 0))
			dlg_current_toolbarcells=i;
	}

	// 5. Фиксация средней кнопкой мыши
	for(i=0;i<BKB_SET_MBUTTONFIX;i++)
	{
		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_mbuttonfix[i].button, BM_GETCHECK, 0, 0))
			dlg_current_mbuttonfix=i;
	}

	// 6. Граница дисперсии в процентах от высоты экрана
	for(i=0;i<BKB_SET_DISPERSION;i++)
	{
		if(BST_CHECKED==SendDlgItemMessage(hdwnd, dlg_dispersion[i].button, BM_GETCHECK, 0, 0))
			dlg_current_dispersion=i;
	}


}


//===============================================================================================
// Читать-писать конфигурацию
//===============================================================================================
typedef struct
{
	char *name; // Название в файле конфигурации
	int *pointer;
	BKBIntChar *check_pointer;
	int max_index;
} T_save_struct;

#define NUM_SAVE_LINES 13

static T_save_struct save_struct[NUM_SAVE_LINES]=
{
	{"MouseXMultiplier",&dlg_current_mouse_x_multiplier,dlg_mouse_x_multiplier, BKB_MOUSE_MULTIPLIERS},
	{"MouseYMultiplier",&dlg_current_mouse_y_multiplier,dlg_mouse_y_multiplier, BKB_MOUSE_MULTIPLIERS},
	{"FixationLimit",&dlg_current_btnfixation,dlg_btnfixation, BKB_FIXATIONS},
	{"PostFixationSkip",&dlg_current_postfixation,dlg_postfixation, BKB_FIXATIONS},
	{"NotKbdFixationLimit",&dlg_current_nokbdfixation,dlg_nokbdfixation, BKB_FIXATIONS},
	{"NotKbdPostFixationSkip",&dlg_current_nokbdpostfixation,dlg_nokbdpostfixation, BKB_FIXATIONS},
	{"KBDFullScreen",&dlg_current_kbd_fullscreen,dlg_kbd_fullscreen, BKB_YESNO},
	{"KBD2STEP",&dlg_current_kbd_2step,dlg_kbd_2step, BKB_YESNO},
	{"ClickMods",&dlg_current_show_clickmods,dlg_show_clickmods, BKB_YESNO},
	{"ToolBarCells",&dlg_current_toolbarcells,dlg_toolbarcells, BKB_SET_TOOLBARCELLS },
	{"MButtonFix",&dlg_current_mbuttonfix,dlg_mbuttonfix, BKB_SET_MBUTTONFIX },
	{"ShowMetrics",&dlg_current_show_metrics,dlg_show_metrics, BKB_YESNO },
	{"Dispersion",&dlg_current_dispersion,dlg_dispersion, BKB_SET_DISPERSION }
};


//===============================================================================================
// Сохраняет конфигурацию в файл
//===============================================================================================
void BKBSettings::SaveBKBConfig()
{
	FILE *fout=NULL;
	_wfopen_s(&fout,tfilename,L"w+");
	if(NULL==fout)
	{
		wcscpy_s(tchar_buf,Internat::Message(51,L"Не могу записать в файл настроек: '"));
		wcsncat_s(tchar_buf,tfilename,1000);
		wcsncat_s(tchar_buf,L"'",2);
		BKBReportError(tchar_buf);
		return;
	}

	T_save_struct ss;
	for(int i=0;i<NUM_SAVE_LINES;i++)
	{
		ss=save_struct[i];
		// Сохраняем Имя, индекс, значение
		fprintf(fout,"%s ",ss.name);
		fprintf(fout,"%d ",*ss.pointer);
		fprintf(fout,"%d ",(ss.check_pointer + *ss.pointer)->value);

		// Перевод строки 
		fprintf(fout,"\n");
	}

	fclose(fout);
}

//===============================================================================================
// Читает конфигурацию из файла
//===============================================================================================
int BKBSettings::OpenBKBConfig()
{
	FILE *fin=NULL;
	_wfopen_s(&fin,tfilename,L"r");
	if(NULL==fin)
	{
#ifdef _DEBUG
		wcscpy_s(tchar_buf,Internat::Message(52,L"Не могу прочитать файл настроек: '"));
		wcsncat_s(tchar_buf,tfilename,1000);
		wcsncat_s(tchar_buf,L"'",2);
		BKBReportError(tchar_buf);
#endif
		return (-1);
	}

	int num_succeeded=0;
	int i;
	T_save_struct ss;
	bool found;

	// Сюда считываются числа
	int int_arg1, int_arg2;

	// Читаем все строки одну за другой
	while(1==fscanf_s(fin,"%s",char_buf, _countof(char_buf)))
	{
		found=false;
		for(i=0;i<NUM_SAVE_LINES;i++) // Перебираем все возможные параметры
		{
			if(0==strncmp(char_buf,save_struct[i].name,sizeof(char_buf)-1))
			{
				ss=save_struct[i];
				
				fgets(char_buf,sizeof(char_buf)-1,fin); // Остаток строки загоняем в буфер

				if(2!=sscanf_s(char_buf,"%d %d",&int_arg1,&int_arg2)) goto load_error;
				if((int_arg1<0)||(int_arg1>=ss.max_index)) goto load_error;
				// Проверка, что по указанному индексу лежит правлильное значение
				if( (ss.check_pointer + int_arg1)->value != int_arg2 ) goto load_error; // индекс не соответствует значению
				*ss.pointer=int_arg1; // Всё правильно, прописываем
				
				num_succeeded++; // Количество успешно считанных параметров
				found=true;
				break; // Не нужно больше сравнивать, выходим из цикла
			} // если найдена строка
			
		} // for
		
		if(!found) 
			goto load_error; // наткнулись на неизвестную строку
	}

#ifdef _DEBUG
	swprintf_s(tchar_buf,Internat::Message(53,L"Файл конфигурации прочитан без ошибок.\r\nЧисло считанных параметров: %d"), num_succeeded);
	BKBReportError(tchar_buf);
#endif
	fclose(fin);
	return 0;


load_error:
	swprintf_s(tchar_buf,Internat::Message(54,L"Файл конфигурации прочитан с ошибками.\r\nВозможно, он от другой версии программы.\r\nОднако, число успешно считанных параметров: %d\r\n(Рекомендую сохранить конфигурацию заново)"), num_succeeded);
	BKBReportError(tchar_buf);
	fclose(fin);
	return -1;
}