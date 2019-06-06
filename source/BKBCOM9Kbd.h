#ifndef __BKB_COM9KBD
#define __BKB_COM9KBD

class BKBCOM9Kbd
{
public:
	static int ScanCodeButton(WORD scancode, bool _shift_pressed, bool _ctrl_pressed, bool _alt_pressed);
	static int UnicodeButton(WORD unicode, bool _shift_pressed, bool _ctrl_pressed, bool _alt_pressed);
	static int Scroll(int scroll_value);
protected:
	static int ArduinoButton(unsigned char c, bool _shift_pressed, bool _ctrl_pressed, bool _alt_pressed);
};

#endif