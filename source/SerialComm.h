#ifndef __BKB_SERIAL
#define __BKB_SERIAL

// работа с портом COM9. Только запись, синхронно. Примитив.

class BKBSerial
{
public:
	static void SendByte(char c);
	static void Halt();
	static int TryToOpen();
protected:
	static int wait_counter; // после ошибки пытаемся переоткрыть порт не сразу, а через 30 попыток записи в порт
	static HANDLE Port;
	static DCB dcb;
};

#endif