#ifndef __BKB_PROGRESSWND
#define __BKB_PROGRESSWND

class BKBProgressWnd
{
public:
	static void Init(HWND master_hwnd);
	static bool TryToShow(int _x, int _y, int _percentage); // true = Pink Approves
	static void Hide(){ShowWindow(PRhwnd, SW_HIDE);} // Чтобы анимация клавиатуры могла отключить розовый прямоугольник
	static int percentage;
protected:
	static HWND PRhwnd;
	

	//static int width,height;
};

#endif