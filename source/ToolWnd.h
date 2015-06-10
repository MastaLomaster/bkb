#ifndef __BKB_TOOLWND
#define __BKB_TOOLWND

#include "Fixation.h"

class BKBToolWnd
{
public: 
	static HWND Init();
	static bool IsItYours(POINT *pnt, BKB_MODE *bm);
	static void OnPaint(HDC hdc=0);
	static void Reset(BKB_MODE *bm);
	static void ScrollCursor(POINT *p);
	static HWND GetHwnd(){return Tlhwnd;};
	static void SleepCheck(POINT *p);

	static bool tool_modifier[4];
	static int current_tool;
	static bool LeftSide() {return left_side;}
	static void Place();

	// Розовая обводка 
	static LPRECT PinkFrame(int _x, int _y);
	static int offset;

	// Устанавливает подсветку нужной клавиши в зависимости от режима
	static void SetCurrentTool(BKB_MODE bm);

protected:
	static TCHAR *tool_modifier_name[4];
	static HWND Tlhwnd;
	static bool left_side;
	static int height;

	static int PositionFromTool(int tool_num);
	static int ToolFromPosition(int position);

	static POINT place_point;
};

#endif