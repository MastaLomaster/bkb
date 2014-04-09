#ifndef __BKB_KEYBWND
#define __BKB_KEYBWND

// Типы клавиш на клавиатуре
typedef enum {undefined=0,scancode,unicode,shift,control,alt,leftkbd,rightkbd,fn,top_down
	} BKB_KEY_TYPE;

// Клавиша на клавиатуре
typedef struct
{
	BKB_KEY_TYPE bkb_keytype; // тип клавиши
	WORD bkb_vscancode; // скакод (для обычных клавиш)
	WORD bkb_unicode; // юникод (для кириллицы)
	WORD bkb_unicode_uppercase; // юникод буквы в верхнем регистре
	WORD bkb_fn_vscancode; // Код при нажатой кнопке fn
	TCHAR *label; // Временно: что писать на клавише
	TCHAR *fn_label;
} BKB_key;


class BKBKeybWnd
{
public:
	static void Init();
	static bool FixPoint(POINT *pnt);
	static void OnPaint(HDC hdc=0);
	static bool IsItYours(POINT *p);
	static bool WhiteSpot(POINT *p);
	static bool ProgressBar(POINT *p, int fixation_count, int _percentage); // Возвращает false, если соскочили с клавиши (для аэромыши)
	static void ProgressBarReset();
	static void Activate();
	static void DeActivate();
	static void OnSize(HWND hwnd, int _width, int _height);
	static void OnTimer();
	static void CreateWhiteSpot(HWND hwnd);
	static POINT whitespot_point;
	static void OnTopDown();
	static void Load();
protected:
	static void ScanCodeButton(WORD scancode);
	static void PopulateCtrlAltShiftFn(); // В переключенной раскладке находим клавиши Ctrl, Alt, Shift, Fn
	static int ctrl_row,ctrl_column,alt_row,alt_column,shift_row,shift_column,fn_row,fn_column;

	static HWND Kbhwnd;
	//static int screen_x, screen_y, start_y;
	static int screen_x, screen_y;
	//static float cell_size;
	static int current_pane;
	static int percentage;

	static POINT start_point;
	static bool fixation_approved;
	static int row, column, row_pressed, column_pressed;

	static bool shift_pressed, ctrl_pressed, alt_pressed, caps_lock_pressed, Fn_pressed;
	//static int screen_num;

	static HDC memdc1, memdc2, whitespot_dc;
	static HBITMAP hbm1, hbm2, whitespot_bitmap; 
	volatile static LONG redraw_state; 
	static int width, height;

	// Эти задаются в файле KeybLayouts
	static BKB_key *layout;
	static float cell_width, cell_height;
	static int columns,rows,panes;
	static bool bottom_side;

	
	
};

#endif