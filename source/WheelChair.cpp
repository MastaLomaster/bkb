#include <Windows.h>
#include "WheelChair.h"
#include "BKBRepErr.h"
#include "ToolWnd.h"
#include "Internat.h"
#include "SerialComm.h"

extern int screenX, screenY;

int BKBWheelChair::selected_cell=-1;

// Определены в Grid.cpp
void DrawLeftArrow(HDC hdc,int x, int y);
void DrawRightArrow(HDC hdc,int x, int y);
// Определены в ToolWnd.cpp
void DrawUpArrow(HDC hdc,int x, int y);
void DrawDownArrow(HDC hdc,int x, int y);

//==============================================================================================================
// Фактически, попадание в ToolBar уже определено. Остаётся выяснить, какая именно ячейка затронута 
// и остаёмся ли в режиме WheelChair. Если остаёмся, то возвращаем 0. Если не остаёмся, то 1.
// При обработка IsItYours в WheelChair используется определенное ранее (В PinkFrame) значение selected_cell. 
// Заново определять его не будем.
//==============================================================================================================
//int BKBGrid::IsItYours(LONG x, LONG y, LONG width, LONG height)
int BKBWheelChair::IsItYours()
{
	// Раньше мы общались с Arduino здесь. Теперь будем общаться чаще в PinkFrame
	/* 
	STARTUPINFO cif;
	ZeroMemory(&cif,sizeof(STARTUPINFO));
	PROCESS_INFORMATION pi;
	wchar_t *cmd_filename=0;
	
	// Пока будем вызывать внешние скрипты. Потом посмотрим.
	switch(selected_cell)
	{
	case 0:
		cmd_filename=L"grid\\left.cmd";
		break;

	case 1:
		cmd_filename=L"grid\\gostop.cmd";
		break;

	case 2:
		cmd_filename=L"grid\\right.cmd";
		break;
	}
	
	if(cmd_filename) CreateProcess(cmd_filename,NULL,NULL,NULL,FALSE,NULL,NULL,NULL,&cif,&pi);
	*/

	return 0; // Остаёмся в режиме WheelChair
}

//==============================================================================================================
// Фактически, попадание в ToolBar уже определено. Остаётся выяснить, какая именно ячейка затронута 
//==============================================================================================================
LPRECT BKBWheelChair::PinkFrame(int _x, int _y, LONG width, LONG height)
{
	static RECT r;
	char c;
	
	r.top=screenY/3; r.bottom=screenY;

	if(_x<width/4)
	{
		selected_cell=0;
		c='L'; // поворачиваем влево
		r.left=0;
		r.right=width*1/4;
	}
	else if(_x<width*3/4)
	{ 
		// Здесь определяем, это кнопка вперёд или назад
		if(_y<height*2/3) // Это вперёд
		{
			selected_cell=1;
			r.left=width*1/4;
			r.right=width*3/4;
			r.bottom=screenY*7/9;
			c='F';
		}
		else
		{
			selected_cell=4;
			r.left=width*1/4;
			r.right=width*3/4;
			r.top=screenY*7/9;
			c='B';
		}
	}
	else
	{
		selected_cell=2;
		r.left=width*3/4;
		r.right=width;
		c='R';
	}

	// Здесь пошлём в COM-порт
	BKBSerial::SendByte(c);

	return &r;
}

//==============================================================================================================
// СТАРЫЙ ВАРИАНТ, БЕЗ ЗАДНЕГО ХОДА
// Фактически, попадание в ToolBar уже определено. Остаётся выяснить, какая именно ячейка затронута 
//==============================================================================================================
LPRECT BKBWheelChair::PinkFrame2(int _x, int _y, LONG width, LONG height)
{
	static RECT r;
	char c;
	
	r.top=screenY*3/4; r.bottom=screenY;

	if(_x<width/4)
	{
		selected_cell=0;
		c='L'; // поворачиваем влево
		r.left=0;
		r.right=width*1/4;
	}
	else if(_x<width*3/4)
	{ 
		selected_cell=1;
		r.left=width*1/4;;
		r.right=width*3/4;
		c='F';
	}
	else
	{
		selected_cell=2;
		r.left=width*3/4;;
		r.right=width;
		c='R';
	}

	// Здесь пошлём в COM-порт
	BKBSerial::SendByte(c);

	return &r;
}

//=============================================
// Стоп машина!
//=============================================
void BKBWheelChair::PinkFrameMissed()
{
	selected_cell=-1;
	// Здесь пошлём в COM-порт
	BKBSerial::SendByte('S');
}


//=============================================================================================
// Рисуем все ячейки и их содержимое
//=============================================================================================
void BKBWheelChair::OnPaint(HDC hdc, LONG width, LONG height)
{
	// Рисуем нижнюю часть
	MoveToEx(hdc,width/4,height*2/3,NULL);
	LineTo(hdc,width*3/4,height*2/3);
	
	// рисуем две вертикальные черты и две стрелки
	MoveToEx(hdc,width/4,0,NULL);
	LineTo(hdc,width/4,height);

	MoveToEx(hdc,width*3/4,0,NULL);
	LineTo(hdc,width*3/4,height);

	DrawLeftArrow(hdc,width/8,height/2);
	DrawRightArrow(hdc,width*7/8,height/2);

	DrawUpArrow(hdc,width/2,height/3);
	DrawDownArrow(hdc,width/2,height*5/6);


	TextOut(hdc, width*15/16,height/16,Internat::Message(87,L"Esc=выход"),wcslen(Internat::Message(87,L"Esc=выход")));
}

//=============================================================================================
// Старый вариант - без заднего хода. 
// Рисуем все ячейки и их содержимое
//=============================================================================================
void BKBWheelChair::OnPaint2(HDC hdc, LONG width, LONG height)
{
	// рисуем две вертикальные черты и две стрелки
	MoveToEx(hdc,width/4,0,NULL);
	LineTo(hdc,width/4,height);

	MoveToEx(hdc,width*3/4,0,NULL);
	LineTo(hdc,width*3/4,height);

	DrawLeftArrow(hdc,width/8,height/2);
	DrawRightArrow(hdc,width*7/8,height/2);

	TextOut(hdc, width*15/16,height/16,Internat::Message(87,L"Esc=выход"),wcslen(Internat::Message(87,L"Esc=выход")));
}


//=============================================================================================== 
// ЭТОТ ФУНКЦИОНАЛ БОЛЬШЕ НЕ ИСПОЛЬЗУЕТСЯ
// СИГНАЛ В COM-порт ИДЁТ НЕ ПОСЛЕ ФИКСАЦИИ ВЗГЛЯДА, А КАЖДЫЕ 3 ТАКТА.
// ПОЭТОМУ ВРЕМЯ ФИКСАЦИИ БЕЗРАЗЛИЧНО, НО ОСТАВИМ ДЛЯ ИСТОРИИ
//===============================================================================================
// Потом сделаем их настраиваемыми, а пока прикидочно сделаем так:
// для боковых кнопок 0.2+0.3сек, для центральной - 0.5+0.0, для "молока" - 0+0
// при 30 fps 0.1сек=3тика
//===============================================================================================
int BKBWheelChair::FixationLimit()
{
	switch(selected_cell)
	{
	case 0:
	case 2:
		return 6; // 0.2 сек
	case 1:
		return 15; // 0.5 сек
	}
	return 1; // ноль делать нельзя почему-то. Сейчас некогда разбираться, почему
}

int BKBWheelChair::PostFixationSkip()
{
	switch(selected_cell)
	{
	case 0:
	case 2:
		return 9; // 0.3 сек
	case 1:
		return 1; // ноль делать нельзя почему-то. Сейчас некогда разбираться, почему
	}
	return 1; // ноль делать нельзя почему-то. Сейчас некогда разбираться, почему
}