#ifndef __BKB_METRICSWND
#define __BKB_METRICSWND

class BKBMetricsWnd
{
public:
	static void Init(HWND master_hwnd);
	static void OnTick(float dispersion);
	static void Show(bool _show);
protected:
	static HWND MTXhwnd;
	static HDC memdc1, memdc2;
	static HBITMAP hbm1, hbm2; 
	static bool show;
};

#endif