#ifndef __BKB_TRANSPWND
#define __BKB_TRANSPWND


class BKBTranspWnd
{
public:
	static void Init(HWND master_hwnd);
	static void Move(int x, int y);
	static void ToTop();
	static void Show();
	static void Hide();
	static bool flag_show_transp_window; // В режиме [Аэро]мышь окно отключено через этот флаг
	static HWND Trhwnd;
protected:
	static int screen_x, screen_y;
};

#endif