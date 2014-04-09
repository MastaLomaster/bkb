// Взаимодействие c TheEyeTribe 
#ifndef __BKB_TET
#define __BKB_TET

class BKBTET
{
public:
	static int Init(); // Инициализация работы с устройством
	static int Halt(); // Завершение работы с устройством
protected:
	static bool initialized;
};

#endif