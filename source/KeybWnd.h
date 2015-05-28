#ifndef __BKB_KEYBWND
#define __BKB_KEYBWND

#include "Fixation.h"

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
	static void Init(HWND master_hwnd);
	static bool FixPoint(POINT *pnt);
	static void OnPaintStep0(HDC hdc=0);
	static void OnPaintStep1(HDC hdc=0);
	static bool IsItYours(POINT *p, BKB_MODE *bm);
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
	// Скорректировать положение клавиатуры (вдруг изменились настройки полный/неполный размер или положение тулбара)
	static void Place();
	// Розовая обводка нескольких клавиш, когла клавиша нажимается в два захода
	static LPRECT PinkFrame(int _x, int _y);
	
	static int step;
	static bool bottom_side;

	static bool Animate();
	static void OnAnimPaint(HDC hdc);

	static void BackSpace();

protected:
	static void ScanCodeButton(WORD scancode);
	static void PopulateCtrlAltShiftFn(); // В переключенной раскладке находим клавиши Ctrl, Alt, Shift, Fn
	static int ctrl_row,ctrl_column,alt_row,alt_column,shift_row,shift_column,fn_row,fn_column;

	static HWND Kbhwnd;
	static int current_pane;
	static int percentage;

	static POINT start_point, place_point;
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
	

	// Для подсветки розовым прямоугольником
	static RECT pink_rect;
	static int element_rows,element_columns, element_row, element_column; // element - это кусок клавиатуры 4x2 клавиши
	static int step1_element_row, step1_element_column; // выбор клавишт в увеличенной клавиатуре
	static int cropped_rows,cropped_columns, cropped_row, cropped_column; // кусок клавиатуры внутри элемента
	static float cropped_cell_width;
	// Для анимации
	static DWORD animation_start_time;
	static bool animation_started;
	static HWND AnimHwnd;
	static LONG anim_width, anim_height;
	static RECT extended_rect; // Прямоугольник, возможно с большей высотой, чем клавиатура, взгляд на который считается попаданием в клавиатуру
	static RECT element_rect; // Прямоугольник, который будет увеличен до целой клавиатуры на следующем шаге

};

#endif