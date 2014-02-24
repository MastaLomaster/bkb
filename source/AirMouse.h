// Взаимодействие (гироскопической) мышью вместо Tobii REX
#ifndef __BKB_AIRMOUSE
#define __BKB_AIRMOUSE

class BKBAirMouse
{
public:
	static int Init(HWND hwnd); // Инициализация работы с устройством
	static int Halt(HWND hwnd); // Завершение работы с устройством
	static void OnTimer();
protected:
	static bool initialized;
};

#endif