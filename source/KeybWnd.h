#ifndef __BKB_KEYBWND
#define __BKB_KEYBWND

#define BKB_KBD_NUM_CELLS 15

// Типы клавиш на клавиатуре
typedef enum {undefined=0,scancode,unicode,shift,control,alt,leftkbd,rightkbd,fn
	} BKB_KEY_TYPE;

// Клавиша на клавиатуре
typedef struct
{
	BKB_KEY_TYPE bkb_keytype; // тип клавиши
	WORD bkb_vscancode; // скакод (для обычных клавиш)
	WORD bkb_unicode; // юникод (для кириллицы)
	WORD bkb_unicode_uppercase; // юникод буквы в верхнем регистре
	char *label; // Временно: что писать на клавише
} BKB_key;


class BKBKeybWnd
{
public:
	static void Init();
	static bool FixPoint(POINT *pnt);
	static void OnPaint(HDC hdc=0);
	static bool IsItYours(POINT *p);
	static void WhiteSpot(POINT *p);
	static bool ProgressBar(POINT *p, int fixation_count, int _percentage); // Возвращает false, если соскочили с клавиши (для аэромыши)
	static void ProgressBarReset();
	static void Activate();
	static void DeActivate();
	static void OnSize(HWND hwnd, int _width, int _height);
	static void OnTimer();
	static void CreateWhiteSpot(HWND hwnd);
	static POINT whitespot_point;
protected:
	static void ScanCodeButton(WORD scancode);
	static HWND Kbhwnd;
	static int screen_x, screen_y, start_y;
	static float cell_size;
	static int current_pane;
	static int percentage;

	static POINT start_point;
	static bool fixation_approved;
	static int row, column, row_pressed, column_pressed;

	static bool shift_pressed, ctrl_pressed, alt_pressed, caps_lock_pressed, Fn_pressed;
	static int screen_num;

	static HDC memdc1, memdc2, whitespot_dc;
	static HBITMAP hbm1, hbm2, whitespot_bitmap; 
	volatile static LONG redraw_state; 
	static int width, height;
	
};

#endif