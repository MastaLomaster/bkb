#ifndef __BKB_SERIAL
#define __BKB_SERIAL

// ������ � ������ COM9. ������ ������, ���������. ��������.

class BKBSerial
{
public:
	static int SendByte(char c, int _wait_counter=60);
	static void Halt();
	static int TryToOpen();
protected:
	static int wait_counter; // ����� ������ �������� ����������� ���� �� �����, � ����� 30 ������� ������ � ����
	static HANDLE Port;
	static DCB dcb;
};

#endif