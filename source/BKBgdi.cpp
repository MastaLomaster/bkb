#include <Windows.h>
#include "BKBgdi.h"
#include "resource.h"

HPEN red_pen, green_pen, dkyellow_pen, pink_pen, strange_pen; // перья и кисти для рисования
HBRUSH dkblue_brush, dkblue_brush2, blue_brush, dkyellow_brush, green_brush, red_brush;
HFONT hfont, hfont2;
HCURSOR hCursor;

int screenX, screenY, mouscreenX, mouscreenY;

HBITMAP hbm_bell;
extern HINSTANCE	BKBInst;


double screen_scale=1.0;

void BKBgdiInit()
{
	//  Кисти создаём
	red_pen=CreatePen(PS_SOLID,1,RGB(255,100,100));
	green_pen=CreatePen(PS_SOLID,1,RGB(100,255,100));
	dkyellow_pen=CreatePen(PS_SOLID,1,RGB(227,198,2));
	pink_pen=CreatePen(PS_SOLID,5,RGB(255,156,255));
	strange_pen=CreatePen(PS_SOLID,1,RGB(188,159,159));

	dkblue_brush=CreateSolidBrush(RGB(45,62,90));
	dkblue_brush2=CreateSolidBrush(RGB(100,72,100));
	blue_brush=CreateSolidBrush(RGB(188,199,216));
	dkyellow_brush=CreateSolidBrush(RGB(227,198,2));

	green_brush=CreateSolidBrush(RGB(100,255,100));
	red_brush=CreateSolidBrush(RGB(255,50,50));

	hfont = CreateFont( -48, 0, 0, 0, FW_BOLD, 0, 0, 0,
		//RUSSIAN_CHARSET,
		DEFAULT_CHARSET,
		0, 0, 0, 0, L"Arial");

	hfont2 = CreateFont( -14, 0, 0, 0, FW_NORMAL, 0, 0, 0,
		//RUSSIAN_CHARSET,
		DEFAULT_CHARSET,
		0, 0, 0, 0, L"Arial");


	//hfont2=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

	// Получим разрешение экрана
	screenX=GetSystemMetrics(SM_CXSCREEN);
	screenY=GetSystemMetrics(SM_CYSCREEN);

	// Козлиная система разрешений экрана в windows8.1...
	DEVMODE dm;
	ZeroMemory (&dm, sizeof (dm));
	EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm);

	screen_scale=((double)screenX)/dm.dmPelsWidth;

	// В windows 8 при HighDPI координаты курсора отличаются от координат точки на экране
	mouscreenX=dm.dmPelsWidth;
	mouscreenY=dm.dmPelsHeight;

	hCursor = LoadCursor(NULL, IDC_ARROW); 

	hbm_bell=LoadBitmap(BKBInst, MAKEINTRESOURCE(IDB_BITMAP_BELL));

}

void BKBgdiHalt()
{
	// удаляем кисти
	DeleteObject(red_pen);
	DeleteObject(dkyellow_pen);
	DeleteObject(green_pen);
	DeleteObject(pink_pen);
	DeleteObject(strange_pen);

	DeleteObject(dkblue_brush);
	DeleteObject(blue_brush);
	DeleteObject(dkblue_brush2);
	DeleteObject(dkyellow_brush);

	DeleteObject(red_brush);
	DeleteObject(green_brush);

	DeleteObject(hfont2);
	DeleteObject(hfont);
	DeleteObject(hCursor);

	DeleteObject(hbm_bell);
}