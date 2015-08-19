#ifndef __BKB_TURTLE
#define __BKB_TURTLE

class BKBTurtle
{
public: 
	static void Init(HWND master_hwnd);
	static void Place();
	static void OnPaintArrowWnd(HDC hdc);
	static void OnPaintCenter(HDC hdc);
	static void Show(int nCmdShow);
	static bool IsItYours(POINT *pnt);

	// Розовая обводка 
	static LPRECT PinkFrame(int _x, int _y);
	static LPRECT PinkFrameCenter(int _x, int _y);

	static HDC memdc;
	static HBITMAP hbm;
	static POINT p;
	static int swap_step; // Три раза подряд надо смотреть на центр, чтобы черепашка переместилась
protected:
	static void MoveArrow(int dx, int dy); // Двигать курсор 
	static HWND hwndCenter, hwndArrow[4]; // Nord,West,South,East
	static bool flag_right_bottom;
	// смещение всей конструкции 
	static int pos_x, pos_y;
	static int element_size;
	static bool center_visible;
/*	
	static bool IsItYours(POINT *pnt, BKB_MODE *bm);
	static void OnPaint(HDC hdc=0);
	static void Reset(BKB_MODE *bm);
	static void ScrollCursor(POINT *p);
	static HWND GetHwnd(){return Tlhwnd;};
	static void SleepCheck(POINT *p);

	static bool tool_modifier[4];
	static int current_tool;
	static bool LeftSide() {return left_side;}


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

	static POINT place_point; */
};

#endif