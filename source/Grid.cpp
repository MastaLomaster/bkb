#include <Windows.h>
#include "BKBRepErr.h"
#include "Grid.h"
#include "ToolWnd.h"
#include "TranspWnd.h"

bool BKBGrid::f_minimized=false;
// Теперь это один из возможных режимов gBKB_GRID_WHEELCHAIR;
//bool BKBGrid::f_grid_only=false;
extern int gBKB_GRID_WHEELCHAIR;

int BKBGrid::selected_cell=-1;

BKBGrid BKBGrid::mg; // Master Grid - корневая таблица
BKBGrid *BKBGrid::current_grid=&mg; // Указатель на текущий grid

extern HINSTANCE BKBInst;
extern HBRUSH green_brush, red_brush;
extern int gBKB_TOOLBOX_WIDTH;
extern HBITMAP hbm_bell;

//===========================================================================
// рисует стрелку влево (по аналогии с функциями из ToolWnd.cpp)
//===========================================================================
void DrawRightArrow(HDC hdc,int x, int y)
{
	MoveToEx(hdc,x,y-35,NULL);
	LineTo(hdc,x+50,y);
	LineTo(hdc,x,y+35);
	LineTo(hdc,x,y+20);
	LineTo(hdc,x-50,y+20);
	LineTo(hdc,x-50,y-20);
	LineTo(hdc,x,y-20);
	LineTo(hdc,x,y-35);
}

void DrawLeftArrow(HDC hdc,int x, int y)
{
	MoveToEx(hdc,x,y-35,NULL);
	LineTo(hdc,x-50,y);
	LineTo(hdc,x,y+35);
	LineTo(hdc,x,y+20);
	LineTo(hdc,x+50,y+20);
	LineTo(hdc,x+50,y-20);
	LineTo(hdc,x,y-20);
	LineTo(hdc,x,y-35);
}

//==============================================================================================================
// Фактически, попадание в ToolBar уже определено. Остаётся выяснить, какая именно ячейка затронута 
// и остаёмся ли в режиме Grid. Если остаёмся, то возвращаем 0. Если не остаёмся, то 1.
// При обработка IsItYours в Grid используется определенное ранее (В PinkFrame) значение selected_cell. 
// Заново определять его не будем.
//==============================================================================================================
//int BKBGrid::IsItYours(LONG x, LONG y, LONG width, LONG height)
int BKBGrid::IsItYours()
{
	if(f_minimized)
	{
		// Единственное, что при этом может делаться - возвращение к исходному размеру
		f_minimized=false;
		// Теперь вместо этого - BKBGrid::ShowCursor()
		// BKBTranspWnd::Show();
		BKBToolWnd::Place();
		return 0; // Остаёмся в режиме Grid
	}

	//Обработка верхней строки
	if((selected_cell>=100)&&(selected_cell<=103))
	{
		switch(selected_cell-100)
		{
		case 0:
			// Возврат или минимизация.
			if(IsTopLevel())
			{
				if(1==gBKB_GRID_WHEELCHAIR) // возвращаться некуда - минимизируем
				{
					f_minimized=true;
					// Теперь вместо этого - BKBGrid::ShowCursor()
					//BKBTranspWnd::Hide(); // Пока мы в ожидании - стрелку по экрану не гоняем
					BKBToolWnd::Place();
					//InvalidateRect(BKBToolWnd::GetHwnd(),NULL,TRUE); // перерисовать
				}
				else
				{
					return 1; // единственное место, из которого мы покидаем режим Grid
				}
			}
			else
			{
				LevelUp();
				InvalidateRect(BKBToolWnd::GetHwnd(),NULL,TRUE); // перерисовать
			}
			break;

		case 1:
			// Сказать "Да"
			PlaySound(L"grid\\Y.wav", NULL, SND_FILENAME|SND_ASYNC);
			break;

		case 2:
			// Сказать "Нет"
			PlaySound(L"grid\\N.wav", NULL, SND_FILENAME|SND_ASYNC);
			break;

		case 3:
			// Запустить/выключить колокольчик (сигнал тревоги)
			PlaySound(L"grid\\bell.wav", NULL, SND_FILENAME|SND_ASYNC);
			break;
		}
	} // обработка верхней строки
	else // обработка основной таблицы
	{
		if((selected_cell>=0)&&(selected_cell<NumActiveCells())) Activate(selected_cell);
		// перерисовать окно
		InvalidateRect(BKBToolWnd::GetHwnd(),NULL,TRUE);
	}

	return 0; // Остаёмся в режиме Grid
}


//==============================================================================================================
// Фактически, попадание в ToolBar уже определено. Остаётся выяснить, какая именно ячейка затронута 
//==============================================================================================================
LPRECT BKBGrid::PinkFrame(int _x, int _y, LONG width, LONG height)
{
	static RECT r;
	int row,column;
	int cols,cwidth,rheight;

	// 0. Спец. случай - минимизированное окно
	if(f_minimized)
	{
		r.top=1; r.bottom=height-1;
		r.left=1; r.right=width-1;
		return &r;
	}

	// 1. Сначала проверим, не попали ли в верхнюю строку
	if(_y<height/5)
	{
		column=_x/(width/4);
		r.top=0; r.bottom=height/5;
		r.left=column*(width/4);
		r.right=(column+1)*(width/4);

		selected_cell=100+column; // Спец. значения для selected_cell
	}
	else
	{
		switch (NumCells())
		{
		case 2:
			cols=2,cwidth=width/2,rheight=height*4/5;
			break;

		case 4:
			cols=2,cwidth=width/2,rheight=height*2/5;
			break;

		case 8:
			cols=4,cwidth=width/4,rheight=height*2/5;
			break;

		case 12:
			cols=4,cwidth=width/4,rheight=height*4/15;
			break;

		case 16:
			cols=4,cwidth=width/4,rheight=height*1/5;
			break;
		}

		column=_x/cwidth;
		row=(_y-height/5)/rheight;

		selected_cell=row*cols+column; // кандидат на выбор
		if((NumActiveCells()<=selected_cell)||(selected_cell<0)) 
			return NULL; // обводить неактивную ячейку не будем

		r.top=height/5+row*rheight;
		r.bottom=height/5+(row+1)*rheight;
		r.left=column*cwidth;
		r.right=(column+1)*cwidth;
		

	}
	return &r;
}


//=============================================================================================
// Рисуем все ячейки и их содержимое
//=============================================================================================
void BKBGrid::OnPaint(HDC hdc, LONG width, LONG height)
{
	int i,j,cols,rows,cwidth,rheight;
	int gap=2, count=0;
	HDC memDC;

	if(f_minimized) // рисуем только значок больше/меньше
	{
		DrawRightArrow(hdc,gBKB_TOOLBOX_WIDTH, gBKB_TOOLBOX_WIDTH);
	}
	else
	{
		// 1. всегда обводим 4 ячейки в верхней строке
		// !!! Здесь возможно нужна подсветка да/нет !!!
		for(i=0;i<4;i++)
		{
			MoveToEx(hdc,i*width/4+gap,gap,NULL);
			LineTo(hdc,(i+1)*width/4-gap,gap);
			LineTo(hdc,(i+1)*width/4-gap,height/5-gap);
			LineTo(hdc,i*width/4+gap,height/5-gap);
			LineTo(hdc,i*width/4+gap,gap);
		}

		// 1.2. рисуем стрелку влево. Начало - середина первой ячейки
		DrawLeftArrow(hdc,width/8, height/10);

		// 1.3. Кружки "Да" и "Нет"
		HBRUSH hOld = (HBRUSH) SelectObject (hdc, green_brush);
        
		Ellipse(hdc,3*width/8-height/20,height/20,3*width/8+height/20,height*3/20);
		SelectObject (hdc, red_brush);
		Ellipse(hdc,5*width/8-height/20,height/20,5*width/8+height/20,height*3/20);
		
		SelectObject (hdc,hOld) ;

		// 1.4. Колокол
		memDC=CreateCompatibleDC(hdc);

		SelectObject(memDC, (HGDIOBJ) hbm_bell);
		BitBlt(hdc,width*7/8+gap*2-80,gap*2, 160,155, memDC, 0,0, SRCCOPY); 

		// 2. обводим все картинки
		switch (NumCells())
		{
		case 2:
			cols=2,rows=1,cwidth=width/2,rheight=height*4/5;
			break;

		case 4:
			cols=2,rows=2,cwidth=width/2,rheight=height*2/5;
			break;

		case 8:
			cols=4,rows=2,cwidth=width/4,rheight=height*2/5;
			break;

		case 12:
			cols=4,rows=3,cwidth=width/4,rheight=height*4/15;
			break;

		case 16:
			cols=4,rows=4,cwidth=width/4,rheight=height*1/5;
			break;
		}


		
		for(i=0;i<rows;i++)
		{
			for(j=0;j<cols;j++)
			{
				// Обводим только активные ячейки!
				if(count>=NumActiveCells()) { DeleteDC(memDC); return;}

				// здесь же будет отрисовка битмапа и подсветка выбранного
				if(current_grid->child[count]) // У этой ячейки прописан объект grid и может быть изображение
				{
					BKBGrid *g=current_grid->child[count]; // для укорачивания записи
					if(g->hbm) // да, изображение есть
					{
						int Xdest,Ydest,Bwidth,Bheight,Xsrc,Ysrc;

						// Если ширина ячейки (с учетом отступов) больше битмапа
						if(cwidth-gap*4>g->hbm_width)
						{
							Xdest=j*cwidth+(cwidth-g->hbm_width)/2;
							Bwidth=g->hbm_width;
							Xsrc=0;
						}
						else
						{
							Xdest=j*cwidth+gap*2;
							Bwidth=cwidth-gap*4;
							Xsrc=(g->hbm_width-(cwidth-gap*4))/2;
						}

						// Если высота ячейки (с учетом отступов) больше битмапа
						if(rheight-gap*4>g->hbm_height)
						{
							Ydest=i*rheight+(rheight-g->hbm_height)/2+height/5;
							Bheight=g->hbm_height;
							Ysrc=0;
						}
						else
						{
							Ydest=i*rheight+gap*2+height/5;
							Bheight=rheight-4*gap;
							Ysrc=(g->hbm_height-(rheight-4*gap))/2;
						}

						SelectObject(memDC, (HGDIOBJ) current_grid->child[count]->hbm);
						//BitBlt(hdc,j*cwidth+gap*2,i*rheight+gap*2+height/5, cwidth-gap*4, rheight-gap*4, memDC, 0,0, SRCCOPY); 
						BitBlt(hdc,Xdest,Ydest, Bwidth, Bheight, memDC, Xsrc,Ysrc, SRCCOPY); 
					} // да, изображение есть
					else // Объект есть, а изображения нет. Рисуем вопросительный знак
					{
						TextOut(hdc,j*cwidth+gap*2+20,i*rheight+gap*2+height/5+20,L"?",1);
					}
				} // У этой ячейки прописан объект grid и может быть изображение
				else // Объекта нет вообще
				{
					TextOut(hdc,j*cwidth+gap*2+20,i*rheight+gap*2+height/5+20,L"?",1);
				}
				
				count++;

				// обводка
				MoveToEx(hdc,j*cwidth+gap,i*rheight+gap+height/5,NULL);
				LineTo(hdc,(j+1)*cwidth-gap,i*rheight+gap+height/5);
				LineTo(hdc,(j+1)*cwidth-gap,(i+1)*rheight-gap+height/5);
				LineTo(hdc,j*cwidth+gap,(i+1)*rheight-gap+height/5);
				LineTo(hdc,j*cwidth+gap,i*rheight+gap+height/5);

				
			}
		}
		DeleteDC(memDC);
	}
}

//================================================================================================
// Сработала фиксация на активной ячейке
//================================================================================================
void BKBGrid::Activate(int cell)
{
	TCHAR cmd_filename[16];

	if(current_grid->child[cell]) // может быть вариант, когда объекта нет (с вопросительным знаком)
	{
		// Есть ли звук для проигрывания?
		if(current_grid->child[cell]->soundfile[0])
			PlaySound(current_grid->child[cell]->soundfile, NULL, SND_FILENAME|SND_ASYNC);

		// 11.10.2018
		// Выполняет внешнюю программу
		// Формируем реальное имя файла
		wcscpy_s(cmd_filename,L"grid\\"); // Имя файла всегда начинается с каталога grid
		wcscat_s(cmd_filename,current_grid->child[cell]->filename_template);
		wcscat_s(cmd_filename,L".cmd");

		BOOL result;
		STARTUPINFO cif;
		ZeroMemory(&cif,sizeof(STARTUPINFO));
		PROCESS_INFORMATION pi;

		result=CreateProcess(cmd_filename,NULL,NULL,NULL,FALSE,NULL,NULL,NULL,&cif,&pi);
		if(result)
		{
			f_minimized=true; // Теперь может минимизироваться не только на верхнем уровне, если запускает программу
			BKBToolWnd::Place();
		}
		// [ конец изменений 11.10.2018 ]

		// Нужно ли делать переход на следующий уровень?
		if(current_grid->child[cell]->num_active>0)
			current_grid=current_grid->child[cell];

		

	}
}

//================================================================================================
// Разное.
// Конструктор с загрузкой битмапа и имени файла в нужную ячейку 
// Если она уже занята - выгрузить и выдать сообщение об ошибке
//================================================================================================
BKBGrid::BKBGrid(BKBGrid *_parent, int cell, TCHAR *_soundfile, HBITMAP _hbm):parent(_parent),child(),num_active(0),soundfile(), hbm(_hbm)
{
	// пропускаем проверку аргументов на адекватность. Жаль, но на это нет времени.
	if(parent->child[cell]) // ячейка занята!
	{
#ifdef _DEBUG
		BKBReportError(__WIDEFILE__,L"Parent cell is occupied",__LINE__);
#endif
		delete parent->child[cell]; 
	}

	parent->child[cell]=this;

	if(cell>=parent->num_active) parent->num_active=cell+1;

	// определение размеров битмапа
	if(_hbm)
	{
		BITMAP bm;
		GetObject(_hbm, sizeof(bm), &bm);

		hbm_width=bm.bmWidth;
		hbm_height=bm.bmHeight;
	}

	if (_soundfile)
	{
		wcscpy_s(soundfile,_soundfile);
	}

}

BKBGrid::~BKBGrid()
{
	int i;
	
	// сначала стереть все дочерние
	for(i=0;i<15;i++)
	{
		if(child[i]) delete child[i];
	}

   // soundfile теперь стирать не надо, ибо статика
	
	if(hbm) DeleteObject(hbm);
}

int BKBGrid::NumCells()
{
	int n=NumActiveCells();
	if(n>12) return 16;
	else if(n>8) return 12;
	else if(n>4) return 8;
	else if(n>2) return 4;
	else return 2;
}

//===================================================================
// Загрузка тестовых данных для отладки
//===================================================================
void BKBGrid::TestData()
{
	HBITMAP h;
	TCHAR *tc;
	BKBGrid *grid2;
	
/*	h=(HBITMAP)LoadImage(BKBInst,L".\\grid\\0.bmp",IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	new BKBGrid(current_grid, 0,L"grid\\0.wav",h); // Картинку на первой позиции

	h=(HBITMAP)LoadImage(BKBInst,L"grid\\1.bmp",IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	new BKBGrid(current_grid, 1,L".\\grid\\1.wav",h); // Картинку на второй позиции

	h=(HBITMAP)LoadImage(BKBInst,L"grid\\3.bmp",IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	grid2=new BKBGrid(current_grid, 3,L".\\grid\\3.wav",h); // Картинку на ЧЕТВЁРТОЙ позиции

	// Проверка 2-го уровня
	h=(HBITMAP)LoadImage(BKBInst,L"grid\\30.bmp",IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	new BKBGrid(grid2,0,L".\\grid\\30.wav",h); 

	int i;
	i=NumActiveCells();
*/
	Load();
}

//===============================================================================================================================================
// При переборе файлов типа bmp или wav:
// Как определить, подходит ли нам имя файла?
// 1. длина имени файла не более 3 символов + расширение, то есть 8 
// 2. Отбросить расширение (поставить ноль туда, где была точка)
// 3. Перебирать символы один за другим, проверить, что символ в диапазоне 0..f или A..F, получить его значение для всех (максимум 3) символа.
// И получить общее число символов.
// Затем вызвать функцию создания (или поиска) GRID с индексом i1,i2,i3. Получить на него указатель, добавить туда битмап или звук.
//===============================================================================================================================================
void BKBGrid::Load()
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	int step,len,i,c[4];
	TCHAR *ext[2]={L"grid\\*.bmp", L"grid\\*.wav"};
	BKBGrid *g;
	TCHAR filename[16];
	HBITMAP _hbm;

	for(step=0;step<2;step++) // в первом шаге перебираем bmp, во втором - wav
	{
		hFind = FindFirstFile(ext[step], &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE) 
		{
#ifdef _DEBUG
			BKBReportError(__WIDEFILE__,L"Invalid File Handle",__LINE__);
#endif
			return;
		}

		// просматриваем все файлы
		do 
		{
			len=wcslen(FindFileData.cFileName);
			if((len<5)||(len>7)) continue;

			// перебираем символы до точки
			for(i=0;i<len-4;i++)
			{
				if((FindFileData.cFileName[i]>=L'0')&&(FindFileData.cFileName[i]<=L'9'))
					c[i]=FindFileData.cFileName[i]-L'0';
				else if((FindFileData.cFileName[i]>=L'a')&&(FindFileData.cFileName[i]<=L'f'))
					c[i]=10+FindFileData.cFileName[i]-L'a';
				else if((FindFileData.cFileName[i]>=L'A')&&(FindFileData.cFileName[i]<=L'F'))
					c[i]=10+FindFileData.cFileName[i]-L'A';
				else
					goto next_file; // Меня зовут Михаил и я зависим от операторов goto...
			}

			g=FindOrCreate(c[0],c[1],c[2],len-4);

			if(NULL!=g)
			{
				// 11.10.2018 Добавили сохранение имени файла без расширения
				wcsncpy_s(g->filename_template, FindFileData.cFileName, len-4);

				// Формируем реальное имя файла
				wcscpy_s(filename,L"grid\\"); // Имя файла всегда начинается с каталога grid
				wcscat_s(filename,FindFileData.cFileName);

				if(0==step) // Это файл .bmp, скармливаем битмап, как в конструкторе
				{
					_hbm=(HBITMAP)LoadImage(BKBInst,filename,IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					if(_hbm)
					{
						BITMAP bm;
						GetObject(_hbm, sizeof(bm), &bm);

						g->hbm=_hbm;
						g->hbm_width=bm.bmWidth;
						g->hbm_height=bm.bmHeight;
					}
				}
				else // просто копируем имя файла
				{
					wcscpy_s(g->soundfile, filename);
				}
			}

next_file: ;
		} while (FindNextFile(hFind, &FindFileData)); // просматриваем все файлы этого типа
		FindClose(hFind);
	}
}

//========================================================================================================
// Находит среди существующих или создаёт grid c путём l1->l2->l3
//========================================================================================================
BKBGrid *BKBGrid::FindOrCreate(int l0, int l1, int l2, int _num_levels)
{
	BKBGrid *g;
	int l[3]={l0,l1,l2};
	int i;

	// проверим аргументы на ошибки
	if((_num_levels<1)||(_num_levels>3)||(l0<0)||(l0>15)||
		(_num_levels>1)&&((l1<0)||(l1>15)) ||
		(_num_levels>2)&&((l2<0)||(l2>15)))
	{
#ifdef _DEBUG
			BKBReportError(__WIDEFILE__,L"BKBGrid::FindOrCreate bad arguments",__LINE__);
#endif
		return 0;
	}

	// 1. Находим grid или создаём
	g=&mg;
	for(i=0;i<_num_levels;i++)
	{
		if(NULL==g->child[l[i]])
		{
			// Нет такого, создаём...
			// конструктор правильно обновляет num_active у родительского grid
			new BKBGrid(g,l[i],0,0);
		}
		g=g->child[l[i]]; // продвигаемся дальше по ветке
	}
	
	return g;
}

//================================================================================================
// В режиме минимизиро ванного окна показывает курсор, только когда он внутри окна
// В режиме, когда видно всё, двигает курсор
// Слизано с BKBToolWnd::ScrollCursor(POINT *p)
//================================================================================================
void BKBGrid::ShowCursor(POINT *_pnt)
{
	static bool mouse_inside_toolbar=true, last_mouse_inside_toolbar=true; // Для скрытия второго курсора при перемещении за область тулбара
	
	// Попала ли точка фиксации в границы окна?
	POINT pnt=*_pnt;
	RECT crect;
	GetClientRect(BKBToolWnd::GetHwnd(),&crect);
	ScreenToClient(BKBToolWnd::GetHwnd(),&pnt);
	if(((pnt.x>=0)&&(pnt.x<crect.right)&&(pnt.y>0)&&(pnt.y<crect.bottom))||(!f_minimized))
	{
		// Попали в тулбокс, покажите курсор
		mouse_inside_toolbar=true;
		if(false==last_mouse_inside_toolbar) BKBTranspWnd::Show(); // Показать стрелку
		BKBTranspWnd::Move(_pnt->x,_pnt->y);
	}
	else
	{
		// мимо тулбара
		mouse_inside_toolbar=false;
		if(true==last_mouse_inside_toolbar) BKBTranspWnd::Hide(); // Убрать стрелку
	}
	last_mouse_inside_toolbar=mouse_inside_toolbar;
}