#ifndef __BKB_HOOKPROC
#define __BKB_HOOKPROC

LRESULT  CALLBACK HookProc2(int disabled,WPARAM wParam,LPARAM lParam) ;
extern volatile bool BKB_MBUTTON_PRESSED;

#endif