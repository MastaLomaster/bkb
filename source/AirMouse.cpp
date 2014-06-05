#include <Windows.h>
#include "AirMouse.h"
#include "TranspWnd.h"

// Заголовочные файлы из Tobii Gaze SDK
#include "tobiigaze_error_codes.h"
#include "tobiigaze.h"
//#include "tobiigaze_config.h" - такой файл был в Gaze SDK 2.0

// Прототип callback-функции из TobiiREX.cpp
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data);

extern int screenX, screenY;
//============================================================================
// Запускает таймер, по которому якобы приходят координаты курсора от Tobii
// 10 раз в секунду
//============================================================================
int BKBAirMouse::Init(HWND hwnd)
{
	screenX=GetSystemMetrics(SM_CXSCREEN);
	screenY=GetSystemMetrics(SM_CYSCREEN);

	SetTimer(hwnd,1,25,NULL);

	// Не показывать прозрачное окно, ибо курсор сам движется
	BKBTranspWnd::flag_show_transp_window=false;
	return 0;
}

//============================================================================
// Убивает таймер, по которому якобы приходят координаты курсора от Tobii
// 10 раз в секунду
//============================================================================
int BKBAirMouse::Halt(HWND hwnd)
{
	KillTimer(hwnd,1);
	return 0;
}

//============================================================================
// При срабатывании таймера имитирует сигнал от Tobii REX
//============================================================================
void BKBAirMouse::OnTimer()
{
	tobiigaze_gaze_data gd;

	POINT p;
	if(0==GetCursorPos(&p))
	{
		//OutputDebugString("Bad cursor position\n");
	}
	else
	{
		gd.tracking_status = TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED;
		gd.left.gaze_point_on_display_normalized.x=p.x/(double)screenX;
		gd.left.gaze_point_on_display_normalized.y=p.y/(double)screenY;

		gd.right.gaze_point_on_display_normalized.x=gd.left.gaze_point_on_display_normalized.x;
		gd.right.gaze_point_on_display_normalized.y=gd.left.gaze_point_on_display_normalized.y;

		gd.timestamp=1000UL*timeGetTime(); // Используется скроллом

		on_gaze_data(&gd, NULL);
	}
}