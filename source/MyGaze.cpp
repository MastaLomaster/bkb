// THE CODE WAS NEVER TESTED !!!
// ЭТОТ КОД НИКОГДА НЕ ТЕСТИРОВАЛСЯ !!!
// Код, в основном, позаимствован из OptiKey
#include <Windows.h>
#include <stdint.h>
#include <float.h>
#include "BKBRepErr.h"
#include "MyGaze.h"
#include "Internat.h"
#include "TobiiREX.h"

// Прототип callback-функции из OnGazeData.cpp
void on_gaze_data(const toit_gaze_data* gazedata, void *user_data);

struct EyeDataStruct
{
	double gazeX;
    double gazeY;
    double diam;
    double eyePositionX;
    double eyePositionY;
    double eyePositionZ;
};

struct SampleStruct
{
		uint64_t timestamp;
        EyeDataStruct leftEye;
        EyeDataStruct rightEye;
};

typedef int (__stdcall *type_mygaze_sample_callback_handler)(struct SampleStruct sampleData);
//typedef __declspec(dllimport) int (__cdecl *type_mygaze_connect) ();
typedef __declspec(dllimport) int (__stdcall *type_mygaze_connect) ();
//typedef __declspec(dllimport) int (__cdecl *type_mygaze_disconnect) ();
typedef __declspec(dllimport) int (__stdcall *type_mygaze_disconnect) ();
typedef __declspec(dllimport) int (__stdcall *type_mygaze_start) ();
typedef __declspec(dllimport) int (__stdcall *type_mygaze_set_connection_timeout) (int);

typedef __declspec(dllimport) int (__stdcall *type_mygaze_set_sample_callback)(type_mygaze_sample_callback_handler pSampleCallbackFunction); 


type_mygaze_start fp_mygaze_start;
type_mygaze_connect fp_mygaze_connect;
type_mygaze_disconnect fp_mygaze_disconnect;
type_mygaze_set_sample_callback fp_mygaze_set_sample_callback;
type_mygaze_set_connection_timeout fp_mygaze_set_connection_timeout;

bool BKBMyGaze::initialized(false);

HMODULE MyGazeDLL=0;

//=====================================================================================
// Сюда будем получать данные
//=====================================================================================
static int __stdcall myGazeHandler(struct SampleStruct sd)
{
	toit_gaze_data gd;

	bool left_valid=false, right_valid=false;

	double leftX = sd.leftEye.gazeX;
    double leftY = sd.leftEye.gazeY;
    double rightX = sd.rightEye.gazeX;
    double rightY = sd.rightEye.gazeY;

	// проверяем, что нам передали что-то осмысленное
	if (!_isnan(leftX)&&!_isnan(leftY))
	{
		// Так в OptiKey делается. Не знаю, надо ли...
		if((leftX>0.0)&&(leftY>0.0))
		{
			left_valid=true;
		}
	}
	if (!_isnan(rightX)&&!_isnan(rightY))
	{
		// Так в OptiKey делается. Не знаю, надо ли...
		if((rightX>0.0)&&(rightY>0.0))
		{
			right_valid=true;
		}
	}

	if((!left_valid)&&(!right_valid))
	{
		// какую-то фигню прислали, дальше ничего не передаём
		return 0;
	}
	else
	{
		// левый
		if(left_valid)
		{
			gd.left.bingo.x=leftX;
			gd.left.bingo.y=leftY;

			if(!right_valid) // делимся с отстающими
			{
				gd.right.bingo.x=leftX;
				gd.right.bingo.y=leftY;

			}
		}

		// правый
		if(right_valid)
		{
			gd.right.bingo.x=rightX;
			gd.right.bingo.y=rightY;

			if(!left_valid) // делимся с отстающими
			{
				gd.left.bingo.x=rightX;
				gd.left.bingo.y=rightY;

			}
		}

		gd.toit_status=1; // TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED
		gd.timestamp=1000UL*timeGetTime(); // Используется скроллом
		on_gaze_data(&gd, NULL);
	}

	return 0;
}

//=====================================================================================
// Начало работы с устройством
//=====================================================================================
int BKBMyGaze::Init()
{
	int result;

	if(initialized) return 1; // уже инициализировали

	MyGazeDLL = LoadLibrary(L"myGazeAPI.dll");
	if(0==MyGazeDLL)
	{
		BKBReportError(Internat::Message(81,L"Не удалось загрузить библиотеку myGazeAPI.dll\r\nСкопируйте её в рабочий каталог программы"));
		return 1;
	}

	fp_mygaze_start=(type_mygaze_start)GetProcAddress(MyGazeDLL,"iV_Start");
	fp_mygaze_connect=(type_mygaze_connect)GetProcAddress(MyGazeDLL,"iV_Connect");
	fp_mygaze_disconnect=(type_mygaze_disconnect)GetProcAddress(MyGazeDLL,"iV_Disconnect");
	fp_mygaze_set_sample_callback=(type_mygaze_set_sample_callback) GetProcAddress(MyGazeDLL,"iV_SetSampleCallback");
	fp_mygaze_set_connection_timeout=(type_mygaze_set_connection_timeout) GetProcAddress(MyGazeDLL,"iV_SetConnectionTimeout");
	
	if(!fp_mygaze_start||!fp_mygaze_connect||!fp_mygaze_disconnect||!fp_mygaze_set_sample_callback)
	{
		BKBReportError(Internat::Message(82,L"Не удалось получить необходимые функции из myGazeAPI.dll"));
		return 1;
	}

	// Запускаем!
/*	result=(*fp_mygaze_start)();
	if(1!=result)
	{
		BKBReportError(result, __WIDEFILE__,L"myGaze: iV_Start",__LINE__);
		return 1;
	}
*/
/*
	//int __stdcall iV_SetConnectionTimeout(int time);
	result=(*fp_mygaze_set_connection_timeout)(15);
	if(1!=result)
	{
		BKBReportError(result, __WIDEFILE__,L"myGaze: iV_SetConnectionTimeout",__LINE__);
		return 1;
	}
*/	
	result=(*fp_mygaze_connect)();
	if(1!=result)
	{
		BKBReportError(result, __WIDEFILE__,L"myGaze: iV_Connect",__LINE__);
		return 1;
	}

	// Устанавливаем Callback-функцию
	if(1!=(*fp_mygaze_set_sample_callback)(myGazeHandler))
	{
		BKBReportError( __WIDEFILE__,L"myGaze: iV_SetSampleCallback",__LINE__);
		return 1;
	}

	initialized=true;
	return 0; // нормально отработали
}

//=====================================================================================
// Завершение работы с устройством
//=====================================================================================
int BKBMyGaze::Halt()
{
	if(!initialized) return 1; // уже завершили работу

	initialized=false;

	 (*fp_mygaze_disconnect)();

	// Выгружаем DLL
	if(MyGazeDLL) FreeLibrary(MyGazeDLL);

	return 0; // нормально отработали
}
