#include <Windows.h>
#include <stdio.h>
#include "KeybWnd.h"
#include "BKBRepErr.h"
#include "Internat.h"


#define BKB_MAX_PANES 5
#define BKB_MAX_ROWS 8
#define BKB_MAX_COLUMNS 24

extern BKB_key RefBKBKeybLayouts[];

// Убирает перевод строки 0d0a -- функция определена в Internat.cpp. Оттуда вообще всё передрано.
void Strip(TCHAR *b);

static TCHAR buf[4096];
static TCHAR tmp_bkb_keytype[4096];
static TCHAR tmp_label[4096];
static TCHAR tmp_fn_label[4096];
static WORD tmp_bkb_vscancode; // скакод (для обычных клавиш)
static WORD tmp_bkb_unicode; // юникод (для кириллицы)
static WORD tmp_bkb_unicode_uppercase; // юникод буквы в верхнем регистре
static WORD tmp_bkb_fn_vscancode; // Код при нажатой кнопке fn

// Для сканирования (scanf) enum-типа
#define BKB_NUM_KEYTYPES 10
struct
{
	TCHAR *bkb_keytype_tchar;
	BKB_KEY_TYPE bkb_keytype;
} scan_BKB_KEY_TYPE[BKB_NUM_KEYTYPES]=
{
	{L"undefined",undefined},
	{L"scancode",scancode},
	{L"unicode",unicode},
	{L"shift",shift},
	{L"control",control},
	{L"alt",alt},
	{L"leftkbd",leftkbd},
	{L"rightkbd",rightkbd},
	{L"fn",fn},
	{L"top_down",top_down}
};

static void KBDLoadError(int line)
{
	TCHAR mybuf[4096];
	swprintf_s(mybuf,_countof(mybuf),L"%s: %d\r\n%s",
		Internat::Message(31,L"Ошибка в файле конфигурации клавиатуры, в строке номер"),
		line,
		Internat::Message(32,L"Откатываемся к клавиатуре по умолчанию."));

	BKBReportError(mybuf);
}

// Здесь только часть класса KeybWnd, которая занимается загрузкой раскладки из файла
void BKBKeybWnd::Load()
{
	// Работаем с файлом
	int offset,tmp_panes,tmp_rows,tmp_columns;
	int i,j,line=1;
	bool keytype_found;

	//int index,i,l,l2,offset;

	// 1. открываем файл
	FILE *fin;
	if(0!=fopen_s(&fin,"keyboard.bkb","rb"))
	{
		// Нет файла - ну и бог с ним. Тихо возвращаеся.
		return;
	}

	// 1.5. Обнуляем layout. Ибо в cleanup мы должны знать, нужно ему делать delete[] или нет
	layout=0;

	// 2. В первой строке читаем характеристики клавиатуры: число раскладок, строк и столбцов
	if(NULL==fgetws(buf,4095,fin)) goto cleanup;
	if(TCHAR(0xFEFF)==buf[0]) offset=1; else offset=0; // Ищем BOM
// !!! Сюда добавить поиск комментариев !!!?? Или запретить их в первой строке??
	if(3!=swscanf_s(buf+offset,L"%d %d %d",&tmp_panes,&tmp_rows,&tmp_columns))  
	{
		KBDLoadError(line);
		goto cleanup; // Пропускаем BOM, считываем параметры
	}

	if((tmp_panes<0)||(tmp_panes>BKB_MAX_PANES))
	{
		KBDLoadError(line);
		goto cleanup;
	}

	if((tmp_rows<0)||(tmp_rows>BKB_MAX_ROWS))
	{
		KBDLoadError(line);
		goto cleanup;
	}

	if((tmp_columns<0)||(tmp_columns>BKB_MAX_COLUMNS))
	{
		KBDLoadError(line);
		goto cleanup;
	}

	// 3. Создаём массив BKB_Key и читаем в него данные
	// Указатель на раскладку обнулим.
	layout=new BKB_key[tmp_panes*tmp_rows*tmp_columns];

	// Здесь очень сложно...
	for(i=0,line=2;i<tmp_panes*tmp_rows*tmp_columns;line++)
	{
		if(NULL==fgetws(buf,4095,fin)) 
		{
			// Неожиданный конец файла
			BKBReportError(Internat::Message(33,L"Неожиданный конец файла конфигурации клавиатуры. Откатываемся к клавиатуре по умолчанию."));
			goto cleanup;
		}
		// Убираем перевод строки
		Strip(buf);
		// Игнорируем пустые строки и строки, начинающиеся с #
		if(0==wcslen(buf)) continue;
		if('#'==buf[0]) continue;
		if(' '==buf[0]) continue;
		// При continue i не увеличивается

		// Вот дошли до ссканфа! 
		if(7!=swscanf_s(buf,L"%s %hx %hx %hx %hx %s %s",tmp_bkb_keytype, 4095,
			&tmp_bkb_vscancode, &tmp_bkb_unicode, &tmp_bkb_unicode_uppercase, &tmp_bkb_fn_vscancode, 
			tmp_label, 4095,
			tmp_fn_label, 4095))
		{
			KBDLoadError(line);
			goto cleanup;
		}

		// Находим keytype
		keytype_found=false;
		for(j=0;j<BKB_NUM_KEYTYPES;j++)
		{
			if(0==wcscmp(tmp_bkb_keytype,scan_BKB_KEY_TYPE[j].bkb_keytype_tchar))
			{
				// нашли !!!
				layout[i].bkb_keytype=scan_BKB_KEY_TYPE[j].bkb_keytype;
				keytype_found=true;
				break;
			}
		}
		if(!keytype_found)
		{
			KBDLoadError(line);
			goto cleanup;
		}

		layout[i].bkb_vscancode=tmp_bkb_vscancode;
		layout[i].bkb_unicode=tmp_bkb_unicode;
		layout[i].bkb_unicode_uppercase=tmp_bkb_unicode_uppercase;
		layout[i].bkb_fn_vscancode=tmp_bkb_fn_vscancode;
		
		// Отводим место под строки и копируем их
		layout[i].label=new TCHAR[wcslen(tmp_label)+1];
		wcscpy_s(layout[i].label,wcslen(tmp_label)+1,tmp_label);
		
		if(tmp_bkb_fn_vscancode)
		{
			layout[i].fn_label=new TCHAR[wcslen(tmp_fn_label)+1];
			wcscpy_s(layout[i].fn_label,wcslen(tmp_fn_label)+1,tmp_fn_label);
		}
		else layout[i].fn_label=0;

		i++; // i увеличиваем, только если это реальная строка, а не комментарий
	}
	
	// Всё нормально, работаем
	panes=tmp_panes;
	rows=tmp_rows;
	columns=tmp_columns;

	// закрываем файл

	fclose(fin);
	return;

cleanup:	
	if(layout)
	{
		// Бесполезно. Нужно ещё строки подчищать...
		//delete[] layout;
	}
	layout=RefBKBKeybLayouts; // Hardcoded раскладка клавиатуры
	fclose(fin);
}