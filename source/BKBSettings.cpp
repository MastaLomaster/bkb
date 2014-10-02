#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "BKBSettings.h"
#include "ToolWnd.h"
#include "BKBRepErr.h"
#include "TranspWnd.h"
#include "Internat.h"

extern HINSTANCE	BKBInst;

HWND BKBSettings::settings_hwnd=0;

static char char_buf[4096];
static TCHAR tchar_buf[4096]; // Для сообщений об ошибках
static TCHAR *tfilename=L"config.bkb";

const int BKB_MOUSE_MULTIPLIERS=7;
BKBIntChar dlg_mouse_x_multiplier[BKB_MOUSE_MULTIPLIERS]={{L"не усиливать",10,IDC_RADIO_X_BUTTON1},{L"1.5",15,IDC_RADIO_X_BUTTON2},{L"2.0",20,IDC_RADIO_X_BUTTON3},{L"2.5",25,IDC_RADIO_X_BUTTON4},{L"3.0",30,IDC_RADIO_X_BUTTON5},{L"3.5",35,IDC_RADIO_X_BUTTON6},{L"4.0",40,IDC_RADIO_X_BUTTON7}}; 
BKBIntChar dlg_mouse_y_multiplier[BKB_MOUSE_MULTIPLIERS]={{L"не усиливать",10,IDC_RADIO_Y_BUTTON1},{L"1.5",15,IDC_RADIO_Y_BUTTON2},{L"2.0",20,IDC_RADIO_Y_BUTTON3},{L"2.5",25,IDC_RADIO_Y_BUTTON4},{L"3.0",30,IDC_RADIO_Y_BUTTON5},{L"3.5",35,IDC_RADIO_Y_BUTTON6},{L"4.0",40,IDC_RADIO_Y_BUTTON7}}; 
int dlg_current_mouse_x_multiplier=0, dlg_current_mouse_y_multiplier=0;
extern int g_BKB_MOUSE_X_MULTIPLIER, g_BKB_MOUSE_Y_MULTIPLIER; // Определены в AirMouse.cpp

const int BKB_FIXATIONS=5;
BKBIntChar dlg_btnfixation[BKB_FIXATIONS]={{L"10",10,IDC_RADIO_FIXATION1},{L"15",15,IDC_RADIO_FIXATION2},{L"20",20,IDC_RADIO_FIXATION3},{L"25",25,IDC_RADIO_FIXATION4},{L"30",30,IDC_RADIO_FIXATION5}};
BKBIntChar dlg_postfixation[BKB_FIXATIONS]={{L"10",10,IDC_RADIO_POSTFIXATION1},{L"15",15,IDC_RADIO_POSTFIXATION2},{L"20",20,IDC_RADIO_POSTFIXATION3},{L"25",25,IDC_RADIO_POSTFIXATION4},{L"30",30,IDC_RADIO_POSTFIXATION5}};
int dlg_current_btnfixation=4, dlg_current_postfixation=4;
extern int FIXATION_LIMIT; // Сколько последовательных точек с низкой дисперсией считать фиксацией
extern int POSTFIXATION_SKIP; // сколько точек пропустить после фиксации, чтобы начать считать новую фиксацию

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
				ShowWindow(hdwnd,SW_HIDE);
				return 1;
	
			case IDOK: 	//Хорошо!
				ShowWindow(hdwnd,SW_HIDE);
				BKBSettings::Screen2Load(hdwnd);
				BKBSettings::ActualizeLoad();
				BKBSettings::SaveBKBConfig();
				return 1; 

			} // switch WM_COMMAND
		
		BKBTranspWnd::ToTop(); // После нажатия прозрачное окно уходит вниз
		break; // if WM_COMMAND 

	case WM_INITDIALOG:
		SetWindowPos(hdwnd,HWND_TOP,50,50,0,0,SWP_NOSIZE | SWP_NOREDRAW);
			
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
void BKBSettings::SettingsDialogue()
{
	if(!settings_hwnd)
	{
		settings_hwnd=CreateDialog(BKBInst, MAKEINTRESOURCE(IDD_SETTINGS), BKBToolWnd::GetHwnd(), DlgSettingsWndProc);
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
	if(Internat::Message(47,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_FIXATION_LIMIT, WM_SETTEXT, 0L, (LPARAM)Internat::Message(47,0));// Время нажатия на кнопки 
	if(Internat::Message(48,0)) SendDlgItemMessage(hdwnd,IDC_STATIC_POSTFIXATION_SKIP, WM_SETTEXT, 0L, (LPARAM)Internat::Message(48,0));// Пауза между нажатиями

	if(Internat::Message(49,0)) SendDlgItemMessage(hdwnd,IDOK, WM_SETTEXT, 0L, (LPARAM)Internat::Message(49,0));// Сохранить
	if(Internat::Message(50,0)) SendDlgItemMessage(hdwnd,IDCANCEL, WM_SETTEXT, 0L, (LPARAM)Internat::Message(50,0));// Отмена
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
	}
	// 1.2 Нажать те, что надо
	SendDlgItemMessage(hdwnd, dlg_btnfixation[dlg_current_btnfixation].button, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hdwnd, dlg_postfixation[dlg_current_postfixation].button, BM_SETCHECK, BST_CHECKED, 0);
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

#define NUM_SAVE_LINES 4

static T_save_struct save_struct[NUM_SAVE_LINES]=
{
	{"MouseXMultiplier",&dlg_current_mouse_x_multiplier,dlg_mouse_x_multiplier, BKB_MOUSE_MULTIPLIERS},
	{"MouseYMultiplier",&dlg_current_mouse_y_multiplier,dlg_mouse_y_multiplier, BKB_MOUSE_MULTIPLIERS},
	{"FixationLimit",&dlg_current_btnfixation,dlg_btnfixation, BKB_FIXATIONS},
	{"PostFixationSkip",&dlg_current_postfixation,dlg_postfixation, BKB_FIXATIONS}
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