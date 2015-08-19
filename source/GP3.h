// Взаимодействие c Gazepoint GP3 
#ifndef __BKB_GP3
#define __BKB_GP3

class BKBGP3
{
public:
	static int Init(); // Инициализация работы с устройством
	static int Halt(); // Завершение работы с устройством
protected:
	static bool initialized;
};

#endif