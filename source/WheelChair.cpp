#include <Windows.h>
#include "WheelChair.h"
#include "BKBRepErr.h"
#include "ToolWnd.h"
#include "Internat.h"
#include "SerialComm.h"

extern int screenX, screenY;

int BKBWheelChair::selected_cell=-1;

// ���������� � Grid.cpp
void DrawLeftArrow(HDC hdc,int x, int y);
void DrawRightArrow(HDC hdc,int x, int y);
// ���������� � ToolWnd.cpp
void DrawUpArrow(HDC hdc,int x, int y);
void DrawDownArrow(HDC hdc,int x, int y);

//==============================================================================================================
// ����������, ��������� � ToolBar ��� ����������. ������� ��������, ����� ������ ������ ��������� 
// � ������� �� � ������ WheelChair. ���� �������, �� ���������� 0. ���� �� �������, �� 1.
// ��� ��������� IsItYours � WheelChair ������������ ������������ ����� (� PinkFrame) �������� selected_cell. 
// ������ ���������� ��� �� �����.
//==============================================================================================================
//int BKBGrid::IsItYours(LONG x, LONG y, LONG width, LONG height)
int BKBWheelChair::IsItYours()
{
	// ������ �� �������� � Arduino �����. ������ ����� �������� ���� � PinkFrame
	/* 
	STARTUPINFO cif;
	ZeroMemory(&cif,sizeof(STARTUPINFO));
	PROCESS_INFORMATION pi;
	wchar_t *cmd_filename=0;
	
	// ���� ����� �������� ������� �������. ����� ���������.
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

	return 0; // ������� � ������ WheelChair
}

//==============================================================================================================
// ����������, ��������� � ToolBar ��� ����������. ������� ��������, ����� ������ ������ ��������� 
//==============================================================================================================
LPRECT BKBWheelChair::PinkFrame(int _x, int _y, LONG width, LONG height)
{
	static RECT r;
	char c;
	
	r.top=screenY/3; r.bottom=screenY;

	if(_x<width/4)
	{
		selected_cell=0;
		c='L'; // ������������ �����
		r.left=0;
		r.right=width*1/4;
	}
	else if(_x<width*3/4)
	{ 
		// ����� ����������, ��� ������ ����� ��� �����
		if(_y<height*2/3) // ��� �����
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

	// ����� ����� � COM-����
	BKBSerial::SendByte(c);

	return &r;
}

//==============================================================================================================
// ������ �������, ��� ������� ����
// ����������, ��������� � ToolBar ��� ����������. ������� ��������, ����� ������ ������ ��������� 
//==============================================================================================================
LPRECT BKBWheelChair::PinkFrame2(int _x, int _y, LONG width, LONG height)
{
	static RECT r;
	char c;
	
	r.top=screenY*3/4; r.bottom=screenY;

	if(_x<width/4)
	{
		selected_cell=0;
		c='L'; // ������������ �����
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

	// ����� ����� � COM-����
	BKBSerial::SendByte(c);

	return &r;
}

//=============================================
// ���� ������!
//=============================================
void BKBWheelChair::PinkFrameMissed()
{
	selected_cell=-1;
	// ����� ����� � COM-����
	BKBSerial::SendByte('S');
}


//=============================================================================================
// ������ ��� ������ � �� ����������
//=============================================================================================
void BKBWheelChair::OnPaint(HDC hdc, LONG width, LONG height)
{
	// ������ ������ �����
	MoveToEx(hdc,width/4,height*2/3,NULL);
	LineTo(hdc,width*3/4,height*2/3);
	
	// ������ ��� ������������ ����� � ��� �������
	MoveToEx(hdc,width/4,0,NULL);
	LineTo(hdc,width/4,height);

	MoveToEx(hdc,width*3/4,0,NULL);
	LineTo(hdc,width*3/4,height);

	DrawLeftArrow(hdc,width/8,height/2);
	DrawRightArrow(hdc,width*7/8,height/2);

	DrawUpArrow(hdc,width/2,height/3);
	DrawDownArrow(hdc,width/2,height*5/6);


	TextOut(hdc, width*15/16,height/16,Internat::Message(87,L"Esc=�����"),wcslen(Internat::Message(87,L"Esc=�����")));
}

//=============================================================================================
// ������ ������� - ��� ������� ����. 
// ������ ��� ������ � �� ����������
//=============================================================================================
void BKBWheelChair::OnPaint2(HDC hdc, LONG width, LONG height)
{
	// ������ ��� ������������ ����� � ��� �������
	MoveToEx(hdc,width/4,0,NULL);
	LineTo(hdc,width/4,height);

	MoveToEx(hdc,width*3/4,0,NULL);
	LineTo(hdc,width*3/4,height);

	DrawLeftArrow(hdc,width/8,height/2);
	DrawRightArrow(hdc,width*7/8,height/2);

	TextOut(hdc, width*15/16,height/16,Internat::Message(87,L"Esc=�����"),wcslen(Internat::Message(87,L"Esc=�����")));
}


//=============================================================================================== 
// ���� ���������� ������ �� ������������
// ������ � COM-���� �Ĩ� �� ����� �������� �������, � ������ 3 �����.
// ������� ����� �������� �����������, �� ������� ��� �������
//===============================================================================================
// ����� ������� �� ��������������, � ���� ���������� ������� ���:
// ��� ������� ������ 0.2+0.3���, ��� ����������� - 0.5+0.0, ��� "������" - 0+0
// ��� 30 fps 0.1���=3����
//===============================================================================================
int BKBWheelChair::FixationLimit()
{
	switch(selected_cell)
	{
	case 0:
	case 2:
		return 6; // 0.2 ���
	case 1:
		return 15; // 0.5 ���
	}
	return 1; // ���� ������ ������ ������-��. ������ ������� �����������, ������
}

int BKBWheelChair::PostFixationSkip()
{
	switch(selected_cell)
	{
	case 0:
	case 2:
		return 9; // 0.3 ���
	case 1:
		return 1; // ���� ������ ������ ������-��. ������ ������� �����������, ������
	}
	return 1; // ���� ������ ������ ������-��. ������ ������� �����������, ������
}