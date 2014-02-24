#ifndef __BKB_TRANSPWND
#define __BKB_TRANSPWND


class BKBTranspWnd
{
public:
	static void Init();
	static void Move(int x, int y);
	static void ToTop(){ SetActiveWindow(Trhwnd); BringWindowToTop(Trhwnd); }
	static void Show();
	static void Hide();
	static bool flag_show_transp_window; // В режиме [Аэро]мышь окно отключено через этот флаг
protected:
	static HWND Trhwnd;
	static int screen_x, screen_y;
};

#endif