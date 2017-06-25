// Взаимодействие с прибором Tobii REX
#ifndef __BKB_TOBIIREX
#define __BKB_TOBIIREX

#include <stdint.h>

class BKBTobiiREX
{
public:
	static int Init(); // Инициализация работы с устройством
	static int Halt(); // Завершение работы с устройством
	//static int GetCoordinates();
protected:
	static bool initialized;
};



struct _2dpoint
{
    double x;
    double y;
};


struct _3dpoint 
{
    double x;
    double y;
    double z;
};

struct toit_eye
{
    struct _3dpoint unused1;
    struct _3dpoint unused2;
    struct _3dpoint unused3;
    struct _2dpoint bingo;    
};

struct toit_gaze_data
{
    uint64_t timestamp;
    int toit_status;
    struct toit_eye left;
    struct toit_eye right;
};


#endif