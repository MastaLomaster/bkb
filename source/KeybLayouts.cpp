#include <Windows.h>
#include "KeybWnd.h"

// Раскладки трёх экранов клавиатур
BKB_key BKBKeybLayouts [3][3][BKB_KBD_NUM_CELLS]=
{
	// Первый экран с клавиатурой (кириллица)
	{
		// верхний ряд клавиш
		{
			{leftkbd,0,0,0,"<<сюда"}, // переход на клавиатуру слева
			{unicode,0,0x0439,0x0419,"Й"}, 
			{unicode,0,0x0446,0x0426,"Ц"}, 
			{unicode,0,0x0443,0x0423,"У"}, 
			{unicode,0,0x043A,0x041A,"К"}, 
			{unicode,0,0x0435,0x0415,"Е"}, 
			{unicode,0,0x043D,0x041D,"Н"}, 
			{unicode,0,0x0433,0x0413,"Г"}, 
			{unicode,0,0x0448,0x0428,"Ш"}, 
			{unicode,0,0x0449,0x0429,"Щ"}, 
			{unicode,0,0x0437,0x0417,"З"}, 
			{unicode,0,0x0445,0x0425,"Х"}, 
			{unicode,0,0x044A,0x042A,"Ъ"}, 
			{scancode,VK_RETURN,0,0,"Enter"}, 
			{rightkbd,0,0,0,"туда>>"} // переход на клавиатуру справа
		},
		// средний ряд клавиш
		{
			{unicode,0,0x0451,0x0401,"Ё"}, 
			{unicode,0,0x0444,0x0424,"Ф"}, 
			{unicode,0,0x044B,0x042B,"Ы"}, 
			{unicode,0,0x0432,0x0412,"В"}, 
			{unicode,0,0x0430,0x0410,"А"}, 
			{unicode,0,0x043F,0x041F,"П"}, 
			{unicode,0,0x0440,0x0420,"Р"}, 
			{unicode,0,0x043E,0x041E,"О"}, 
			{unicode,0,0x043B,0x041B,"Л"}, 
			{unicode,0,0x0434,0x0414,"Д"}, 
			{unicode,0,0x0436,0x0416,"Ж"}, 
			{unicode,0,0x044D,0x042D,"Э"},
			{unicode,0,0x02C,0x02C,","}, 
			{unicode,0,0x021,0x021,"!"},
			{unicode,0,0x03A,0x03A,":"} 
		},
		// нижний ряд клавиш
		{
			{shift,0,0,0,"Shift"}, 
			{unicode,0,0x044F,0x042F,"Я"}, 
			{unicode,0,0x0447,0x0427,"Ч"}, 
			{unicode,0,0x0441,0x0421,"С"}, 
			{unicode,0,0x043C,0x041C,"М"}, 
			{unicode,0,0x0438,0x0418,"И"}, 
			{unicode,0,0x0442,0x0422,"Т"}, 
			{unicode,0,0x044C,0x042C,"Ь"}, 
			{unicode,0,0x0431,0x0411,"Б"}, 
			{unicode,0,0x044E,0x042E,"Ю"}, 
			{scancode,VK_SPACE,0,0,"space"},
			{scancode,VK_BACK,0,0,"Backspace"}, 
			{unicode,0,0x02E,0x02E,"."}, 
			{unicode,0,0x03F,0x03F,"?"}, 
			{unicode,0,0x03B,0x03B,";"} 
		}
	},
	// Второй экран с клавиатурой (латинница)
	{
		// верхний ряд клавиш
		{
			{leftkbd,0,0,0,"<<сюда"}, // переход на клавиатуру слева
			{scancode,'Q',0,0,"Q"},
			{scancode,'W',0,0,"W"},
			{scancode,'E',0,0,"E"},
			{scancode,'R',0,0,"R"},
			{scancode,'T',0,0,"T"},
			{scancode,'Y',0,0,"Y"},
			{scancode,'U',0,0,"U"},
			{scancode,'I',0,0,"I"},
			{scancode,'O',0,0,"O"},
			{scancode,'P',0,0,"P"},
			{unicode,0,0x027,0x027,"'"},
			{unicode,0,0x022,0x022,"\""},
			{scancode,VK_RETURN,0,0,"Enter"}, 
			{rightkbd,0,0,0,"туда>>"} // переход на клавиатуру справа
		},
		// средний ряд клавиш
		{
			{scancode,VK_ESCAPE,0,0,"Esc"},
			{scancode,'A',0,0,"A"},
			{scancode,'S',0,0,"S"},
			{scancode,'D',0,0,"D"},
			{scancode,'F',0,0,"F"},
			{scancode,'G',0,0,"G"},
			{scancode,'H',0,0,"H"},
			{scancode,'J',0,0,"J"},
			{scancode,'K',0,0,"K"},
			{scancode,'L',0,0,"L"},
			{unicode,0,0x07B,0x07B,"{"},
			{unicode,0,0x07D,0x07D,"}"},
			{unicode,0,0x05B,0x05B,"["},
			{unicode,0,0x05D,0x05D,"]"},

			{alt,0,0,0,"Alt"} 
		},
		// нижний ряд клавиш
		{
			{shift,0,0,0,"Shift"}, 
			{scancode,'Z',0,0,"Z"},
			{scancode,'X',0,0,"X"},
			{scancode,'C',0,0,"C"},
			{scancode,'V',0,0,"V"},
			{scancode,'B',0,0,"B"},
			{scancode,'N',0,0,"N"},
			{scancode,'M',0,0,"M"},

			{unicode,0,0x03C,0x03C,"<"},
			{unicode,0,0x03E,0x03E,">"},

			{scancode,VK_SPACE,0,0,"space"},
			{scancode,VK_BACK,0,0,"Backspace"}, 

			{unicode,0,0x02F,0x02F,"/"},
			{unicode,0,0x05C,0x05C,"\\"},

			{control,0,0,0,"Ctrl"} 
		}
	},
	// третий экран с клавиатурой (цифры и проч.)
	{
		// верхний ряд клавиш
		{
			{leftkbd,0,0,0,"<<сюда"}, // переход на клавиатуру слева
			{scancode,VK_TAB,0,0,"TAB"},
			{unicode,0,0x040,0x040,"@"},
			{unicode,0,0x023,0x023,"#"},
			{unicode,0,0x024,0x024,"$"},
			{unicode,0,0x025,0x025,"%"},
			{unicode,0,0x05E,0x05E,"^"},
			{unicode,0,0x026,0x026,"&"},
			{unicode,0,0x02A,0x02A,"*"},
			{unicode,0,0x028,0x028,"("},
			{unicode,0,0x029,0x029,")"},
			{unicode,0,0x02D,0x02D,"-"},
			{unicode,0,0x02B,0x02B,"+"},
			{unicode,0,0x03D,0x03D,"="},

			{rightkbd,0,0,0,"туда>>"} // переход на клавиатуру справа
		},
		// средний ряд клавиш
		{
			{fn,0,0,0,"Fn"},
			{unicode,0,0x031,0x031,"1"},
			{unicode,0,0x032,0x032,"2"},
			{unicode,0,0x033,0x033,"3"},
			{unicode,0,0x034,0x034,"4"},
			{unicode,0,0x035,0x035,"5"},
			{unicode,0,0x036,0x036,"6"},
			{unicode,0,0x037,0x037,"7"},
			{unicode,0,0x038,0x038,"8"},
			{unicode,0,0x039,0x039,"9"},
			{unicode,0,0x030,0x030,"0"},
			{scancode,VK_PRIOR,0,0,"PgUp"},
			{scancode,VK_UP,0,0,"Up"},
			{scancode,VK_NEXT,0,0,"PgDown"},

			{alt,0,0,0,"Alt"} 
		},
		// нижний ряд клавиш
		{
			{shift,0,0,0,"Shift"}, 
			{unicode,0,0x07E,0x07E,"~"},
			{unicode,0,0x07C,0x07C,"|"},
			{unicode,0,0x060,0x060,"`"},
			{unicode,0,0x05F,0x05F,"_"},

			{scancode,VK_SNAPSHOT,0,0,"PrntSc"},
			{scancode,VK_PAUSE,0,0,"Pause"},
			{scancode,VK_HOME,0,0,"Home"},
			{scancode,VK_END,0,0,"End"},
			{scancode,VK_INSERT,0,0,"Ins"},
			{scancode,VK_DELETE,0,0,"Del"},
			{scancode,VK_LEFT,0,0,"<--"},
			{scancode,VK_DOWN,0,0,"down"},
			{scancode,VK_RIGHT,0,0,"-->"},

			{control,0,0,0,"Ctrl"} 
		}
	}
};
