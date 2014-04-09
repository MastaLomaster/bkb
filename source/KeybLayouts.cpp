#include <Windows.h>
#include "KeybWnd.h"

int BKBKeybWnd::columns=15,BKBKeybWnd::rows=3,BKBKeybWnd::panes=3;
//BKB_key *BKBKeybWnd::layout определяется после RefBKBKeybLayouts

BKB_key RefBKBKeybLayouts[135]=
{
	// Первый экран с клавиатурой (кириллица)
	// верхний ряд клавиш
		
			{leftkbd,0,0,0,0,L"<<сюда",0}, // переход на клавиатуру слева
			{unicode,0,0x0439,0x0419,0,L"Й",0}, 
			{unicode,0,0x0446,0x0426,0,L"Ц",0}, 
			{unicode,0,0x0443,0x0423,0,L"У",0}, 
			{unicode,0,0x043A,0x041A,0,L"К",0}, 
			{unicode,0,0x0435,0x0415,0,L"Е",0}, 
			{unicode,0,0x043D,0x041D,0,L"Н",0}, 
			{unicode,0,0x0433,0x0413,0,L"Г",0}, 
			{unicode,0,0x0448,0x0428,0,L"Ш",0}, 
			{unicode,0,0x0449,0x0429,0,L"Щ",0}, 
			{unicode,0,0x0437,0x0417,0,L"З",0}, 
			{unicode,0,0x0445,0x0425,0,L"Х",0}, 
			{unicode,0,0x044A,0x042A,0,L"Ъ",0}, 
			{scancode,VK_RETURN,0,0,0,L"Enter",0}, 
			{rightkbd,0,0,0,0,L"туда>>",0}, // переход на клавиатуру справа

			// средний ряд клавиш
			{unicode,0,0x0451,0x0401,0,L"Ё",0}, 
			{unicode,0,0x0444,0x0424,0,L"Ф",0}, 
			{unicode,0,0x044B,0x042B,0,L"Ы",0}, 
			{unicode,0,0x0432,0x0412,0,L"В",0}, 
			{unicode,0,0x0430,0x0410,0,L"А",0}, 
			{unicode,0,0x043F,0x041F,0,L"П",0}, 
			{unicode,0,0x0440,0x0420,0,L"Р",0}, 
			{unicode,0,0x043E,0x041E,0,L"О",0}, 
			{unicode,0,0x043B,0x041B,0,L"Л",0}, 
			{unicode,0,0x0434,0x0414,0,L"Д",0}, 
			{unicode,0,0x0436,0x0416,0,L"Ж",0}, 
			{unicode,0,0x044D,0x042D,0,L"Э",0},
			{unicode,VK_OEM_COMMA,0x02C,0x02C,0,L",",0},
			{unicode,0,0x021,0x021,0,L"!",0},
			{unicode,0,0x03A,0x03A,0,L":",0}, 
		
			// нижний ряд клавиш
			{shift,0,0,0,0,L"Shift",0}, 
			{unicode,0,0x044F,0x042F,0,L"Я",0}, 
			{unicode,0,0x0447,0x0427,0,L"Ч",0}, 
			{unicode,0,0x0441,0x0421,0,L"С",0}, 
			{unicode,0,0x043C,0x041C,0,L"М",0}, 
			{unicode,0,0x0438,0x0418,0,L"И",0}, 
			{unicode,0,0x0442,0x0422,0,L"Т",0}, 
			{unicode,0,0x044C,0x042C,0,L"Ь",0}, 
			{unicode,0,0x0431,0x0411,0,L"Б",0}, 
			{unicode,0,0x044E,0x042E,0,L"Ю",0}, 
			{scancode,VK_SPACE,0,0,0,L"space",0},
			{scancode,VK_BACK,0,0,0,L"Backspace",0}, 
			{unicode,VK_OEM_PERIOD,0x02E,0x02E,0,L".",0}, 
			{unicode,0,0x03F,0x03F,0,L"?",0}, 
			{unicode,0,0x03B,0x03B,0,L";",0},

			// Второй экран с клавиатурой (латинница)
			// верхний ряд клавиш
			{leftkbd,0,0,0,0,L"<<сюда",0}, // переход на клавиатуру слева
			{scancode,'Q',0,0,0,L"Q",0},
			{scancode,'W',0,0,0,L"W",0},
			{scancode,'E',0,0,0,L"E",0},
			{scancode,'R',0,0,0,L"R",0},
			{scancode,'T',0,0,0,L"T",0},
			{scancode,'Y',0,0,0,L"Y",0},
			{scancode,'U',0,0,0,L"U",0},
			{scancode,'I',0,0,0,L"I",0},
			{scancode,'O',0,0,0,L"O",0},
			{scancode,'P',0,0,0,L"P",0},
			{unicode,0,0x027,0x027,0,L"'",0},
			{unicode,0,0x022,0x022,0,L"\"",0},
			{scancode,VK_RETURN,0,0,0,L"Enter",0}, 
			{rightkbd,0,0,0,0,L"туда>>",0}, // переход на клавиатуру справа
		
			// средний ряд клавиш
			{scancode,VK_ESCAPE,0,0,0,L"Esc",0},
			{scancode,'A',0,0,0,L"A",0},
			{scancode,'S',0,0,0,L"S",0},
			{scancode,'D',0,0,0,L"D",0},
			{scancode,'F',0,0,0,L"F",0},
			{scancode,'G',0,0,0,L"G",0},
			{scancode,'H',0,0,0,L"H",0},
			{scancode,'J',0,0,0,L"J",0},
			{scancode,'K',0,0,0,L"K",0},
			{scancode,'L',0,0,0,L"L",0},
			{unicode,0,0x03C,0x03C,0,L"<",0},
			{unicode,0,0x03E,0x03E,0,L">",0},
			//{unicode,0,0x07B,0x07B,0,L"{",0},
			//{unicode,0,0x07D,0x07D,0,L"}",0},
			//{unicode,0,0x05B,0x05B,0,L"[",0},
			//{unicode,0,0x05D,0x05D,0,L"]",0},
			{scancode,VK_LWIN,0,0,0,L"Win",0},
			{top_down,0,0,0,0,L"Вверх-Вниз",0},


			{alt,0,0,0,0,L"Alt",0}, 
		
			// нижний ряд клавиш
			{shift,0,0,0,0,L"Shift",0}, 
			{scancode,'Z',0,0,0,L"Z",0},
			{scancode,'X',0,0,0,L"X",0},
			{scancode,'C',0,0,0,L"C",0},
			{scancode,'V',0,0,0,L"V",0},
			{scancode,'B',0,0,0,L"B",0},
			{scancode,'N',0,0,0,L"N",0},
			{scancode,'M',0,0,0,L"M",0},

			{unicode,VK_OEM_COMMA,0x02C,0x02C,0,L",",0},
			//{unicode,0,0x03C,0x03C,0,L"<",0},
			//{unicode,0,0x03E,0x03E,0,L">",0},
			{unicode,VK_OEM_PERIOD,0x02E,0x02E,0,L".",0}, 

			{scancode,VK_SPACE,0,0,0,L"space",0},
			{scancode,VK_BACK,0,0,0,L"Backspace",0}, 

			{unicode,0,0x02F,0x02F,0,L"/",0},
			{unicode,0,0x05C,0x05C,0,L"\\",0},

			{control,0,0,0,0,L"Ctrl",0},

			// третий экран с клавиатурой (цифры и проч.)
			// верхний ряд клавиш
	
			{leftkbd,0,0,0,0,L"<<сюда",0}, // переход на клавиатуру слева
			{scancode,VK_TAB,0,0,0,L"TAB",0},
			{unicode,0,0x040,0x040,0,L"@",0},
			{unicode,0,0x023,0x023,0,L"#",0},
			{unicode,0,0x024,0x024,0,L"$",0},
			{unicode,0,0x025,0x025,0,L"%",0},
			{unicode,0,0x05E,0x05E,0,L"^",0},
			{unicode,0,0x026,0x026,0,L"&",0},
			{unicode,0,0x02A,0x02A,0,L"*",0},
			{unicode,0,0x028,0x028,0,L"(",0},
			{unicode,0,0x029,0x029,0,L")",0},
			{unicode,VK_SUBTRACT,0x02D,0x02D,0,L"-",0},
			{unicode,VK_ADD,0x02B,0x02B,0,L"+",0},
			{unicode,0,0x03D,0x03D,0,L"=",0},

			{rightkbd,0,0,0,0,L"туда>>",0}, // переход на клавиатуру справа
	
			// средний ряд клавиш
			{fn,0,0,0,0,L"Fn",0},
			{unicode,0x31,0x031,0x031,VK_F1,L"1",L"F1"},
			{unicode,0x32,0x032,0x032,VK_F2,L"2",L"F2"},
			{unicode,0x33,0x033,0x033,VK_F3,L"3",L"F3"},
			{unicode,0x34,0x034,0x034,VK_F4,L"4",L"F4"},
			{unicode,0x35,0x035,0x035,VK_F5,L"5",L"F5"},
			{unicode,0x36,0x036,0x036,VK_F6,L"6",L"F6"},
			{unicode,0x37,0x037,0x037,VK_F7,L"7",L"F7"},
			{unicode,0x38,0x038,0x038,VK_F8,L"8",L"F8"},
			{unicode,0x39,0x039,0x039,VK_F9,L"9",L"F9"},
			{unicode,0x30,0x030,0x030,VK_F10,L"0",L"F10"},
			{scancode,VK_PRIOR,0,0,VK_F11,L"PgUp",L"F11"},
			{scancode,VK_UP,0,0,VK_F12,L"Up",L"F12"},
			{scancode,VK_NEXT,0,0,0,L"PgDown",0},

			{alt,0,0,0,0,L"Alt",0}, 
		
			// нижний ряд клавиш
			{shift,0,0,0,0,L"Shift",0}, 
			{unicode,0,0x07E,0x07E,0,L"~",0},
			{unicode,0,0x07C,0x07C,0,L"|",0},
			{unicode,0,0x060,0x060,0,L"`",0},
			{unicode,0,0x05F,0x05F,0,L"_",0},

			{unicode,0,0x07B,0x07B,VK_SNAPSHOT,L"{",L"PrntSc"},
			{unicode,0,0x07D,0x07D,VK_PAUSE,L"}",L"Pause"},
			{unicode,0,0x05B,0x05B,VK_HOME,L"[",L"Home"},
			{unicode,0,0x05D,0x05D,VK_END,L"]",L"End"},
			//{scancode,VK_SNAPSHOT,0,0,0,L"PrntSc",0},
			//{scancode,VK_PAUSE,0,0,0,L"Pause",0},
			//{scancode,VK_HOME,0,0,0,L"Home",0},
			//{scancode,VK_END,0,0,0,L"End",0},
			{scancode,VK_INSERT,0,0,0,L"Ins",0},
			{scancode,VK_DELETE,0,0,0,L"Del",0},
			{scancode,VK_LEFT,0,0,0,L"<--",0},
			{scancode,VK_DOWN,0,0,0,L"down",0},
			{scancode,VK_RIGHT,0,0,0,L"-->",0},

			{control,0,0,0,0,L"Ctrl",0} 
		
};


BKB_key *BKBKeybWnd::layout=RefBKBKeybLayouts;

/*
// Раскладки трёх экранов клавиатур
BKB_key RefBKBKeybLayouts [3][3][BKB_KBD_NUM_CELLS]=
{
	// Первый экран с клавиатурой (кириллица)
	{
		// верхний ряд клавиш
		{
			{leftkbd,0,0,0,0,L"<<сюда",0}, // переход на клавиатуру слева
			{unicode,0,0x0439,0x0419,0,L"Й",0}, 
			{unicode,0,0x0446,0x0426,0,L"Ц",0}, 
			{unicode,0,0x0443,0x0423,0,L"У",0}, 
			{unicode,0,0x043A,0x041A,0,L"К",0}, 
			{unicode,0,0x0435,0x0415,0,L"Е",0}, 
			{unicode,0,0x043D,0x041D,0,L"Н",0}, 
			{unicode,0,0x0433,0x0413,0,L"Г",0}, 
			{unicode,0,0x0448,0x0428,0,L"Ш",0}, 
			{unicode,0,0x0449,0x0429,0,L"Щ",0}, 
			{unicode,0,0x0437,0x0417,0,L"З",0}, 
			{unicode,0,0x0445,0x0425,0,L"Х",0}, 
			{unicode,0,0x044A,0x042A,0,L"Ъ",0}, 
			{scancode,VK_RETURN,0,0,0,L"Enter",0}, 
			{rightkbd,0,0,0,0,L"туда>>",0} // переход на клавиатуру справа
		},
		// средний ряд клавиш
		{
			{unicode,0,0x0451,0x0401,0,L"Ё",0}, 
			{unicode,0,0x0444,0x0424,0,L"Ф",0}, 
			{unicode,0,0x044B,0x042B,0,L"Ы",0}, 
			{unicode,0,0x0432,0x0412,0,L"В",0}, 
			{unicode,0,0x0430,0x0410,0,L"А",0}, 
			{unicode,0,0x043F,0x041F,0,L"П",0}, 
			{unicode,0,0x0440,0x0420,0,L"Р",0}, 
			{unicode,0,0x043E,0x041E,0,L"О",0}, 
			{unicode,0,0x043B,0x041B,0,L"Л",0}, 
			{unicode,0,0x0434,0x0414,0,L"Д",0}, 
			{unicode,0,0x0436,0x0416,0,L"Ж",0}, 
			{unicode,0,0x044D,0x042D,0,L"Э",0},
			{unicode,0,0x02C,0x02C,0,L",",0}, 
			{unicode,0,0x021,0x021,0,L"!",0},
			{unicode,0,0x03A,0x03A,0,L":",0} 
		},
		// нижний ряд клавиш
		{
			{shift,0,0,0,0,L"Shift",0}, 
			{unicode,0,0x044F,0x042F,0,L"Я",0}, 
			{unicode,0,0x0447,0x0427,0,L"Ч",0}, 
			{unicode,0,0x0441,0x0421,0,L"С",0}, 
			{unicode,0,0x043C,0x041C,0,L"М",0}, 
			{unicode,0,0x0438,0x0418,0,L"И",0}, 
			{unicode,0,0x0442,0x0422,0,L"Т",0}, 
			{unicode,0,0x044C,0x042C,0,L"Ь",0}, 
			{unicode,0,0x0431,0x0411,0,L"Б",0}, 
			{unicode,0,0x044E,0x042E,0,L"Ю",0}, 
			{scancode,VK_SPACE,0,0,0,L"space",0},
			{scancode,VK_BACK,0,0,0,L"Backspace",0}, 
			{unicode,0,0x02E,0x02E,0,L".",0}, 
			{unicode,0,0x03F,0x03F,0,L"?",0}, 
			{unicode,0,0x03B,0x03B,0,L";",0} 
		}
	},
	// Второй экран с клавиатурой (латинница)
	{
		// верхний ряд клавиш
		{
			{leftkbd,0,0,0,0,L"<<сюда",0}, // переход на клавиатуру слева
			{scancode,'Q',0,0,0,L"Q",0},
			{scancode,'W',0,0,0,L"W",0},
			{scancode,'E',0,0,0,L"E",0},
			{scancode,'R',0,0,0,L"R",0},
			{scancode,'T',0,0,0,L"T",0},
			{scancode,'Y',0,0,0,L"Y",0},
			{scancode,'U',0,0,0,L"U",0},
			{scancode,'I',0,0,0,L"I",0},
			{scancode,'O',0,0,0,L"O",0},
			{scancode,'P',0,0,0,L"P",0},
			{unicode,0,0x027,0x027,0,L"'",0},
			{unicode,0,0x022,0x022,0,L"\"",0},
			{scancode,VK_RETURN,0,0,0,L"Enter",0}, 
			{rightkbd,0,0,0,0,L"туда>>",0} // переход на клавиатуру справа
		},
		// средний ряд клавиш
		{
			{scancode,VK_ESCAPE,0,0,0,L"Esc",0},
			{scancode,'A',0,0,0,L"A",0},
			{scancode,'S',0,0,0,L"S",0},
			{scancode,'D',0,0,0,L"D",0},
			{scancode,'F',0,0,0,L"F",0},
			{scancode,'G',0,0,0,L"G",0},
			{scancode,'H',0,0,0,L"H",0},
			{scancode,'J',0,0,0,L"J",0},
			{scancode,'K',0,0,0,L"K",0},
			{scancode,'L',0,0,0,L"L",0},
			{unicode,0,0x07B,0x07B,0,L"{",0},
			{unicode,0,0x07D,0x07D,0,L",0}",0},
			{unicode,0,0x05B,0x05B,0,L"[",0},
			{unicode,0,0x05D,0x05D,0,L"]",0},

			{alt,0,0,0,0,L"Alt",0} 
		},
		// нижний ряд клавиш
		{
			{shift,0,0,0,0,L"Shift",0}, 
			{scancode,'Z',0,0,0,L"Z",0},
			{scancode,'X',0,0,0,L"X",0},
			{scancode,'C',0,0,0,L"C",0},
			{scancode,'V',0,0,0,L"V",0},
			{scancode,'B',0,0,0,L"B",0},
			{scancode,'N',0,0,0,L"N",0},
			{scancode,'M',0,0,0,L"M",0},

			{unicode,0,0x03C,0x03C,0,L"<",0},
			{unicode,0,0x03E,0x03E,0,L">",0},

			{scancode,VK_SPACE,0,0,0,L"space",0},
			{scancode,VK_BACK,0,0,0,L"Backspace",0}, 

			{unicode,0,0x02F,0x02F,0,L"/",0},
			{unicode,0,0x05C,0x05C,0,L"\\",0},

			{control,0,0,0,0,L"Ctrl",0} 
		}
	},
	// третий экран с клавиатурой (цифры и проч.)
	{
		// верхний ряд клавиш
		{
			{leftkbd,0,0,0,0,L"<<сюда",0}, // переход на клавиатуру слева
			{scancode,VK_TAB,0,0,0,L"TAB",0},
			{unicode,0,0x040,0x040,0,L"@",0},
			{unicode,0,0x023,0x023,0,L"#",0},
			{unicode,0,0x024,0x024,0,L"$",0},
			{unicode,0,0x025,0x025,0,L"%",0},
			{unicode,0,0x05E,0x05E,0,L"^",0},
			{unicode,0,0x026,0x026,0,L"&",0},
			{unicode,0,0x02A,0x02A,0,L"*",0},
			{unicode,0,0x028,0x028,0,L"(",0},
			{unicode,0,0x029,0x029,0,L")",0},
			{unicode,0,0x02D,0x02D,0,L"-",0},
			{unicode,0,0x02B,0x02B,0,L"+",0},
			{unicode,0,0x03D,0x03D,0,L"=",0},

			{rightkbd,0,0,0,0,L"туда>>",0} // переход на клавиатуру справа
		},
		// средний ряд клавиш
		{
			{fn,0,0,0,0,L"Fn",0},
			{unicode,0,0x031,0x031,0,L"1",0},
			{unicode,0,0x032,0x032,0,L"2",0},
			{unicode,0,0x033,0x033,0,L"3",0},
			{unicode,0,0x034,0x034,0,L"4",0},
			{unicode,0,0x035,0x035,0,L"5",0},
			{unicode,0,0x036,0x036,0,L"6",0},
			{unicode,0,0x037,0x037,0,L"7",0},
			{unicode,0,0x038,0x038,0,L"8",0},
			{unicode,0,0x039,0x039,0,L"9",0},
			{unicode,0,0x030,0x030,0,L"0",0},
			{scancode,VK_PRIOR,0,0,0,L"PgUp",0},
			{scancode,VK_UP,0,0,0,L"Up",0},
			{scancode,VK_NEXT,0,0,0,L"PgDown",0},

			{alt,0,0,0,0,L"Alt",0} 
		},
		// нижний ряд клавиш
		{
			{shift,0,0,0,0,L"Shift",0}, 
			{unicode,0,0x07E,0x07E,0,L"~",0},
			{unicode,0,0x07C,0x07C,0,L"|",0},
			{unicode,0,0x060,0x060,0,L"`",0},
			{unicode,0,0x05F,0x05F,0,L"_",0},

			{scancode,VK_SNAPSHOT,0,0,0,L"PrntSc",0},
			{scancode,VK_PAUSE,0,0,0,L"Pause",0},
			{scancode,VK_HOME,0,0,0,L"Home",0},
			{scancode,VK_END,0,0,0,L"End",0},
			{scancode,VK_INSERT,0,0,0,L"Ins",0},
			{scancode,VK_DELETE,0,0,0,L"Del",0},
			{scancode,VK_LEFT,0,0,0,L"<--",0},
			{scancode,VK_DOWN,0,0,0,L"down",0},
			{scancode,VK_RIGHT,0,0,0,L"-->",0},

			{control,0,0,0,0,L"Ctrl",0} 
		}
	}
};
*/

/*
{
	{leftkbd,0,0,0,0,L"<<сюда",0}, // переход на клавиатуру слева
	{unicode,0,0x0439,0x0419,VK_F1,L"Й",L"F1"}, 

	{shift,0,0,0,0,L"Shift",0},
	//{unicode,0,0x0446,0x0426,0,0,L"Ц",0}, 
	{fn,0,0,0,0,L"Fn",0},
	
	{unicode,0,0x0439,0x0419,0,L"Й"}, 
	{top_down,0,0,0,0,L"top-down"},

	{unicode,0,0x0439,0x0419,0,L"Й",0}, 
	{leftkbd,0,0,0,0,L"<<сюда",0},
};
*/