// ������ � com-���� (�����������) ��� ���������� Arduino
// ��������� �������� ���������� � ��������� ������� (������ ��������������)
// ������ � ������ COM9. ������ ������, ���������. ��������.

#include <Windows.h>
#include <stdio.h>
#include "SerialComm.h"

int BKBSerial::wait_counter=0;
HANDLE BKBSerial::Port=INVALID_HANDLE_VALUE;
DCB BKBSerial::dcb;


int BKBSerial::TryToOpen()
{
	// 0. ������ �� ���������� ��������
	if (INVALID_HANDLE_VALUE!=Port) return 0;

	// 0.5 ����� ������ �� ��������� �������� 30 �������
	if(wait_counter>0)
	{
		wait_counter--;
		return 1;
	}

	// 1. ��������� ����
	Port = CreateFile(L"\\\\.\\COM9", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
	if (INVALID_HANDLE_VALUE==Port)
	{
		wait_counter=60;
		return 1;
	}

	// 2. ������������� ��������
	FillMemory(&dcb, sizeof(dcb), 0);
	dcb.DCBlength = sizeof(dcb);
	if (!BuildCommDCB(L"9600,n,8,1", &dcb))
	{
		//puts("Couldn't build the DCB. Usually a problem with the communications specification string.");
		Halt();
		wait_counter=60;
		return 1;
	}
	if (!SetCommState(Port, &dcb))
	{
		//puts("Couldn't set port parameters.");
		Halt();
		wait_counter=60;
		return 1;
	}

	return 0;
}


void BKBSerial::Halt()
{
	CloseHandle(Port);
	Port=INVALID_HANDLE_VALUE;
}

//===================================================================
// ������ ���������� 0 ��� 1 �� ���������� �������� (��� BKBCOM9Kbd)
//===================================================================
int BKBSerial::SendByte(char c, int _wait_counter)
{
	static char _c;
	static unsigned long size=0;

	_c=c;

	if(INVALID_HANDLE_VALUE==Port)
	{
		if(TryToOpen()) return 1; // ������������� ���� �� �������
	}

	// ���� ������ ��������, ���
	if(!WriteFile(Port,&_c,1,&size,0))
	{
		Halt();
		wait_counter=_wait_counter;
		return 1;
	}

	return 0;
}
