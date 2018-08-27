// Обрабатывает фиксацию взгляда в контексте текущего режима
#ifndef __BKB_FIXATION
#define __BKB_FIXATION

#include <stdint.h> // Это для uint64_t

typedef enum {BKB_MODE_LCLICK, BKB_MODE_LCLICK_PLUS, BKB_MODE_RCLICK, BKB_MODE_DOUBLECLICK, BKB_MODE_DOUBLECLICK_PLUS, BKB_MODE_DRAG, BKB_MODE_SCROLL, 
	BKB_MODE_KEYBOARD, BKB_MODE_NONE, BKB_MODE_SWAP, BKB_MODE_SLEEP, BKB_MODE_TURTLE, BKB_MODE_GRID} BKB_MODE;

class Fixation 
{
	friend class BKBToolWnd;
public:
	static bool Fix(POINT p); // Взгляд зафиксировался
	static BKB_MODE CurrentMode(){return BKB_Mode; };
	static void Scroll(uint64_t timelag, int direction);
	static bool drag_in_progress;
	static void SetGridMode(){BKB_Mode=BKB_MODE_GRID;}
protected:
	static void LeftClick(POINT p, bool skip_modifier_press=false, bool skip_modifier_unpress=false);
	static void RightClick(POINT p);
	static void DoubleClick(POINT p);
	static bool Drag(POINT p); // true - значит закончил, можно сбрасывать режим
	static void ClickModifiers(bool press); // нажать-отпустить модификаторы клика

	static BKB_MODE BKB_Mode;
};

#endif