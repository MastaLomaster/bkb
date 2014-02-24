// Взаимодействие с прибором Tobii REX
#ifndef __BKB_TOBIIREX
#define __BKB_TOBIIREX

class BKBTobiiREX
{
public:
	static int Init(); // Инициализация работы с устройством
	static int Halt(); // Завершение работы с устройством
	//static int GetCoordinates();
protected:
	static bool initialized;
};

#endif