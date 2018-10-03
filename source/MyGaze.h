// Взаимодействие с прибором MyGaze
#ifndef __BKB_MYGAZE
#define __BKB_MYGAZE

class BKBMyGaze
{
public:
	static int Init(); // Инициализация работы с устройством
	static int Halt(); // Завершение работы с устройством
protected:
	static bool initialized;
};

#endif