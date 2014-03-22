#ifndef __BKB_TOOLWND
#define __BKB_TOOLWND

#include "Fixation.h"

class BKBToolWnd
{
public:
	static void Init();
	static bool IsItYours(POINT *pnt, BKB_MODE *bm);
	static void OnPaint(HDC hdc=0);
	static void Reset(BKB_MODE *bm);
	static void ScrollCursor(POINT *p);
protected:
	static HWND Tlhwnd;
	static int screen_x, screen_y;
	static int current_tool;
	static bool left_side;
};

#endif