#include <Windows.h>
#include <stdio.h>
#include "Internat.h"

static TCHAR buf[4096];

TCHAR *Internat::messages[BKB_MAX_MESSAGES]={0};

//============================================================================================
// Убирает перевод строки 0d0a
//============================================================================================
void Strip(TCHAR *b)
{
	int l=wcslen(b);
	if(l>=2)
	{
		if((0xd==b[l-2])&&(0xa==b[l-1]))
		{
			b[l-2]=0;
		}
	}
}

//============================================================================================
// Загружает строки из внешнего файла
//============================================================================================
void Internat::LoadMessages()
{
	int index,i,l,l2,offset;

	// 1. открываем файл
	FILE *fin;
	if(0!=fopen_s(&fin,"messages.bkb","rb"))
	{
		return;
	}

	while(NULL!=fgetws(buf,4095,fin))
	{
		// Убираем перевод строки
		Strip(buf);

		// отделяем номер от остальной строки
		l=wcslen(buf);

		for(i=0;i<l;i++)
		{
			if((L' '==buf[i])||(TCHAR(9)==buf[i]))  // Пробел или табуляция, разделяющий номер ресурса и строку
			{
				//OutputDebugString(L"Space found\n");
				if(TCHAR(0xFEFF)==buf[0]) offset=1; else offset=0; // Ищем BOM
				swscanf_s(buf+offset,L"%d",&index); // Пропускаем BOM, считываем номер ресурса
				if((index<0)||(index>=BKB_MAX_MESSAGES)) goto cleanup; // Файл битый, дальше не читаем
				if(messages[index]) goto cleanup; // Строка уже загружена => файл битый, дальше не читаем

				l2=wcslen(&buf[i+1])+1; // Длина оставшейся строки + место под ноль в конце
				messages[index]=new TCHAR[l2];
				wcscpy_s(messages[index],l2,&buf[i+1]);

				break; // поиск пробела-табуляции прекращён
			}
		} // for
	}

	// закрываем файл
cleanup:	fclose(fin);
}

//=======================================================================================================
// Возвращает строку из ресурса, а если таковой нет - default string
//=======================================================================================================
TCHAR *Internat::Message(int i, TCHAR *default_string)
{
	if((i<0)||(i>=BKB_MAX_MESSAGES)) return default_string;
	else if(messages[i]) return messages[i];
	else return default_string;
}

//=======================================================================================================
// Освобождает память, занятую строками (нужно ли это, всё равно программа завершается/куча разрушается?)
//=======================================================================================================
void Internat::Unload()
{
	int i;

	for(i=0;i<BKB_MAX_MESSAGES;i++)
	{
		if(messages[i]) delete messages[i];
	}
}