#include <Windows.h>
#include "BKBgdi.h"

HPEN red_pen, green_pen, dkyellow_pen; // перья и кисти для рисования
HBRUSH dkblue_brush, blue_brush;
HFONT hfont;

int screenX, screenY;

double screen_scale=1.0;

void BKBgdiInit()
{
	//  Кисти создаём
	red_pen=CreatePen(PS_SOLID,1,RGB(255,100,100));
	green_pen=CreatePen(PS_SOLID,1,RGB(100,255,100));
	dkyellow_pen=CreatePen(PS_SOLID,1,RGB(227,198,2));

	dkblue_brush=CreateSolidBrush(RGB(45,62,90));
	blue_brush=CreateSolidBrush(RGB(188,199,216));

	hfont = CreateFont( -48, 0, 0, 0, FW_BOLD, 0, 0, 0,
		RUSSIAN_CHARSET,
		0, 0, 0, 0, "Arial");
	

	// Получим разрешение экрана
	screenX=GetSystemMetrics(SM_CXSCREEN);
	screenY=GetSystemMetrics(SM_CYSCREEN);

	// Козлиная система разрешений экрана в windows8.1...
	DEVMODE dm;
	ZeroMemory (&dm, sizeof (dm));
	EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm);

	screen_scale=((double)screenX)/dm.dmPelsWidth;
}

void BKBgdiHalt()
{
	// удаляем кисти
	DeleteObject(red_pen);
	DeleteObject(dkyellow_pen);
	DeleteObject(green_pen);

	DeleteObject(dkblue_brush);
	DeleteObject(blue_brush);

	DeleteObject(hfont);
}