#ifndef __BKB_SERIAL
#define __BKB_SERIAL

// ������ � ������ COM9. ������ ������, ���������. ��������.

class BKBSerial
{
public:
	static void SendByte(char c);
	static void Halt();
	static int TryToOpen();
protected:
	static int wait_counter; // ����� ������ �������� ����������� ���� �� �����, � ����� 30 ������� ������ � ����
	static HANDLE Port;
	static DCB dcb;
};

#endif