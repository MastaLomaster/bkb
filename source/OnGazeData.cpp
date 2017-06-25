#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <time.h>
#include "BKBRepErr.h"
#include "Smooth.h"
#include "Fixation.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "ToolWnd.h"
#include "Internat.h"
#include "WM_USER_messages.h"
#include "BKBHookProc.h"
#include "BKBProgressWnd.h"
#include "BKBMetricsWnd.h"
#include "TobiiREX.h"

int FIXATION_LIMIT=30; // Сколько последовательных точек с низкой дисперсией считать фиксацией (для клавиатуры)
int NOTKBD_FIXATION_LIMIT=30; // Сколько последовательных точек с низкой дисперсией считать фиксацией (не для клавиатуры)
int POSTFIXATION_SKIP=30; // сколько точек пропустить после фиксации, чтобы начать считать новую фиксацию (для клавиатуры)
int NOTKBD_POSTFIXATION_SKIP=30; // сколько точек пропустить после фиксации, чтобы начать считать новую фиксацию (не для клавиатуры)
bool gBKB_2STEP_KBD_MODE=false;
bool flag_Pink_approved;
bool flag_Activemouse=false;

static bool mouse_inside_keyboard=false, last_mouse_inside_keyboard=false; // Для скрытия второго курсора при перемещении в область клавиатуры
// Время фиксации (и паузы между фиксациями) зависит от того, используем ли мы клавиатуру или нет
static int funcFIXATION_LIMIT()
{
	if((BKB_MODE_KEYBOARD==Fixation::CurrentMode())&&(true==mouse_inside_keyboard)) return FIXATION_LIMIT;
	else return NOTKBD_FIXATION_LIMIT;
}

static int funcPOSTFIXATION_SKIP()
{
	if((BKB_MODE_KEYBOARD==Fixation::CurrentMode())&&(true==mouse_inside_keyboard)) return POSTFIXATION_SKIP;
	else return NOTKBD_POSTFIXATION_SKIP;
}


#define CURSOR_SMOOTHING 7; // Направление движения курсора меняется только раз в CURSOR_SMOOTHING отсчетов

extern int screenX, screenY;
extern int gBKB_MBUTTONFIX; // Как реагировать на среднюю кнопку мыши
extern int gBKB_DISP_PERCENT; // Дисперсия в процентах от высоты экрана

static int fixation_count=0; // количество точек, когда мышь почти не двигается
static int skip_count=0; // сколько точек осталось пропустить после фиксации, чтобы начать считать новую фиксацию

//extern HWND	BKBhwnd;
extern int tracking_device;

static toit_gaze_data TGD_interchange; // Буфер, куда записывается пришедшее значение для передачи в другую очередь
static volatile long TGD_is_processing=0; // Типа мьютекса для InterlockedCompareExchange

extern DWORD last_mouse_time;
static DWORD this_time;

//#define DISPERSION_LIMIT 100.0 // Для отслеживания фиксаций
static double funcDISPERSION_LIMIT()
{
	//double d=gBKB_DISP_PERCENT/100.0*screenY;
	//return d;
	return gBKB_DISP_PERCENT/100.0*screenY;
}
//#define DISPERSION_HIGH_LIMIT 300.0 // Для отслеживания быстрых перемещений
static double funcDISPERSION_HIGH_LIMIT()
{
	// Меняется от 0.27 до 0.5 высоты экрана при изменении дисперсии от 10 до 25 процентов
	return screenY*(0.27+(gBKB_DISP_PERCENT-10)*0.23/15);
}

//=====================================================================================
// Функция, возвращающая знак целого числа
//=====================================================================================
inline long signum(long x)
{
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}


//===========================================================================================================
// Функция, которую вызывает REX, когда сообщает данные о глазах
// 01.02.14 Её может вызывать и аэромышь
// 14.04.14 Теперь посылает сообщение другому потоку, если тот уже закончил обработку предыдущего сообщения
//===========================================================================================================
FILE *debug_fout=0;
void on_gaze_data(const toit_gaze_data* gazedata, void *user_data)
{
#ifdef _DEBUG
	// 30.11.2015 Запишем, что сможем
	if(!debug_fout)
	{
		time_t mytime = time(0); /* not 'long' */
		TCHAR ctbuf[1024];
		_wctime_s(ctbuf,1023,&mytime);
		ctbuf[13]=L'-';
		ctbuf[16]=L'-';
		ctbuf[24]=0;
		//wcscat_s(ctbuf,1023,L"_dbg.txt");
		_wfopen_s(&debug_fout,ctbuf,L"wb");
	}
	else fwrite(gazedata,sizeof(gazedata),1,debug_fout);
	
#endif


	// Сбагриваем очередные данные, только если старые уже обработаны
	// Нет нужды в хитрых сравнениях, если мы пропустим один отсчёт, ровным счетом ничего не произойдёт
	// достаточно было бы if(0==TGD_is_processing)
	// но вдруг эту функцию вызовут одновременно два потока? Перестрахуемся.
	if(0==InterlockedCompareExchange(&TGD_is_processing,1,0))
	{
		HWND htb=BKBToolWnd::GetHwnd();
		if(0!=htb)
		{
			TGD_interchange=*gazedata;
		
			if(0==PostMessage(BKBToolWnd::GetHwnd(), WM_USER_DATA_READY, 0, 0))
			{
				//BKBReportError(L"Failed to Post a Message");
			}
		}
		else TGD_is_processing=0;
	}
	
}

//==========================================================================================================
// Обработка в основном потоке. Здесь можно делать sleep, обрабатываем неспешно, не влияя на другие потоки
//==========================================================================================================
void on_gaze_data_main_thread()
{
	toit_gaze_data* gazedata=&TGD_interchange;
	

	static POINT point_left={0,0}, point_right={0,0}, point={0,0}; //, last_point={0,0}, tmp_point;
	double disp1,disp2; // дисперсия в последних отсчетах левого и правого глаза
	static POINT  screen_cursor_point; //cursor_position={0,0};
	static double cursor_position_x, cursor_position_y;
	static int cursor_linear_move_counter=CURSOR_SMOOTHING; // Столько отсчетов курсор будет двигаться линейно 
	static double cursor_speed_x=0.0, cursor_speed_y=0.0; 
	static uint64_t last_timestamp=0;
	static bool last_flag_Activemouse=false;
	
	
		// Для проверки рисуем точку на экране
	// Но только если отследили оба глаза!!
	//if (gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED)
	//if (gazedata->toit_status != TOBIIGAZE_TRACKING_STATUS_NO_EYES_TRACKED)
	if (gazedata->toit_status != 0)
	{
		//hdc=GetDC(BKBhwnd);
		// Этот кусок не отрабатывает, если выше мы ограничились только  TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED
		switch(gazedata->toit_status)
		{
		//case TOBIIGAZE_TRACKING_STATUS_ONLY_LEFT_EYE_TRACKED:
		//case TOBIIGAZE_TRACKING_STATUS_ONE_EYE_TRACKED_PROBABLY_LEFT:
		case 2:
		case 3:
			gazedata->right.bingo.x=gazedata->left.bingo.x;
			gazedata->right.bingo.y=gazedata->left.bingo.y;
			break;

		//case TOBIIGAZE_TRACKING_STATUS_ONE_EYE_TRACKED_UNKNOWN_WHICH:
		case 4:
			TGD_is_processing=0;
			return;
			break;
			
		//case TOBIIGAZE_TRACKING_STATUS_ONE_EYE_TRACKED_PROBABLY_RIGHT:
		//case TOBIIGAZE_TRACKING_STATUS_ONLY_RIGHT_EYE_TRACKED:
		case 5:
		case 6:
		
			gazedata->left.bingo.x=gazedata->right.bingo.x;
			gazedata->left.bingo.y=gazedata->right.bingo.y;
			break;
		}


		// Трекинг левого глаза 
		point_left.x=screenX*gazedata->left.bingo.x;
		point_left.y=screenY*gazedata->left.bingo.y;
		disp1=BKBSmooth(&point_left, 0);
		
		// Трекинг правого глаза 
		point_right.x=screenX*gazedata->right.bingo.x;
		point_right.y=screenY*gazedata->right.bingo.y;
		disp2=BKBSmooth(&point_right, 1);
		
		point.x=(point_right.x+point_left.x)/2;
		point.y=(point_right.y+point_left.y)/2;
	
		//=================================================================================
		// Теперь о перемещениях курсора
		// Сглаживание не нужно для аэромыши
		if((disp1>funcDISPERSION_HIGH_LIMIT())&&(disp2>funcDISPERSION_HIGH_LIMIT())||(2==tracking_device))
		{
			// Курсор перемещаем быстро
			//cursor_position=point;
			cursor_position_x=point.x;
			cursor_position_y=point.y;
			cursor_linear_move_counter=0; // В следующем такте нужно будет пересчитать скорость
		}
		else // Курсор перемещаем вяло, беря только каждую CURSOR_SMOOTHING (пятую) опорную точку
		{
			// Пора ли посчитать новое направление движения курсора?
			if(cursor_linear_move_counter>0) // нет, не пора
			{
				cursor_linear_move_counter--;
			}
			else // новое направление движения
			{
				cursor_linear_move_counter=CURSOR_SMOOTHING;
				cursor_speed_x=(point.x-cursor_position_x)/(double)CURSOR_SMOOTHING;
				cursor_speed_y=(point.y-cursor_position_y)/(double)CURSOR_SMOOTHING;
			}
			
			// Всплыли-таки ошибки округления, поменяли cursor_position на double
			cursor_position_x+=cursor_speed_x; // Здесь будут рудименты округления, пока забьём
			cursor_position_y+=cursor_speed_y; 
		} // вялое перемещение курсора


		// Что получится?
		screen_cursor_point.x=cursor_position_x+0.5; // Теперь переводим в POINT
		screen_cursor_point.y=cursor_position_y+0.5; // 0.5 компенсирует ошибку округления при переводе в целое	


		//==============================================================================
		// Перекочевало сюда из BKBAirMouse::OnTimer(), чтобы поддерживать все трекеры
		// Проверяет, не сорвалось ли трёхкратное нажатие для засыпания/просыпания
		BKBToolWnd::SleepCheck(&screen_cursor_point);

		//===============================================================================================
		// Обработка скролла
		if(BKB_MODE_SCROLL==Fixation::CurrentMode())
		{
			if(screen_cursor_point.y>screenY*3/4) // Скролл вниз
			{
				Fixation::Scroll(gazedata->timestamp-last_timestamp,-1);
			}
			else if(screen_cursor_point.y<screenY/4) // Скролл вверх
			{
				Fixation::Scroll(gazedata->timestamp-last_timestamp,1);
			}
			last_timestamp=gazedata->timestamp;
		}
		
		// Курсор при скролле и клавиатуре двигается только в отдельных случаях, обрабатываемых ниже
		// 13.06.2015 Двигаем курсор, только если это не аэромышь. Аэромышь двигает его в режиме черепашки
		// 30.11.2015 Ещё двигаем курсор в режиме DEBUG
#ifdef _DEBUG
		if((BKB_MODE_SCROLL!=Fixation::CurrentMode())&&(BKB_MODE_KEYBOARD!=Fixation::CurrentMode()))
				BKBTranspWnd::Move(screen_cursor_point.x,screen_cursor_point.y); 
#else
		if((2!=tracking_device)&&(BKB_MODE_SCROLL!=Fixation::CurrentMode())&&(BKB_MODE_KEYBOARD!=Fixation::CurrentMode()))
				BKBTranspWnd::Move(screen_cursor_point.x,screen_cursor_point.y); 
#endif
		
		// Рисуем окно со стрелкой или белое пятно на клавиатуре?
		if(BKB_MODE_KEYBOARD==Fixation::CurrentMode()) 
		{
			mouse_inside_keyboard=BKBKeybWnd::WhiteSpot(&screen_cursor_point);
			if((true==mouse_inside_keyboard)&&(false==last_mouse_inside_keyboard)) // пришли в клавиатуру
				BKBTranspWnd::Hide(); // Убрать стрелку
			else if((false==mouse_inside_keyboard)&&(true==last_mouse_inside_keyboard)) // вышли из клавиатуры
				BKBTranspWnd::Show(); // Показать стрелку

			last_mouse_inside_keyboard=mouse_inside_keyboard;
#ifdef _DEBUG
			if((!mouse_inside_keyboard)) 	// 30.11.2015 Для отладки при имитации неточного определения взгляда
#else
			if((!mouse_inside_keyboard)&&(2!=tracking_device)) 	// 13.06.2015 Двигаем курсор, только если это не аэромышь. Аэромышь двигает его в режиме черепашки
#endif
				BKBTranspWnd::Move(screen_cursor_point.x,screen_cursor_point.y);
		}

		// При скролле рисуем прозрачное окно только тогда, когда оно попадает на тулбар
		if(BKB_MODE_SCROLL==Fixation::CurrentMode()) 
		{
			BKBToolWnd::ScrollCursor(&screen_cursor_point);
		}

//=================================================================================
		// Может, ассистент работает мышью? Тогда не нужно отрабатывать фиксацию
		if(2!=tracking_device)
		{
			this_time=timeGetTime();
			if(this_time-last_mouse_time<1500UL) // Полторы секунды ждём после остановки мыши
			{
				flag_Activemouse=true; // Это для перерисовки окна с курсором. При активной мыши он становится оранжевым.
				fixation_count=0; // первым делом сбросим эту переменную
				//skip_count=POSTFIXATION_SKIP;
			}
			else flag_Activemouse=false; 
			// Перерисовываем окно с курсором, если поменялась активность мыши (успокоилась или начала двигаться)
			if(last_flag_Activemouse!=flag_Activemouse) InvalidateRect(BKBTranspWnd::Trhwnd,NULL,FALSE);
			last_flag_Activemouse=flag_Activemouse;
		}
//=================================================================================

		// Средняя кнопка в режиме 2 подавляет реальную фиксацию 
		if(2==gBKB_MBUTTONFIX) fixation_count=0;

		//========================================================================================================================================
		// Добавление в версии D - розовое окно прогресса
		//========================================================================================================================================
		// ? любое ли false==flag_Pink_approved должно приводить к сбросу счётчика фиксации fixation_count=0?
		// Нет, не любое, например, фиксация для левого клика в произвольном месте экрана вообще не связана с flag_Pink_approved
		// Поэтому сброс фиксации проходит только при пепреходе flag_Pink_approved: true -> false
		// Вводим переменную для хранения предыдущего значения флага
		static bool prev_flag_Pink_approved=false;
		
		flag_Pink_approved=BKBProgressWnd::TryToShow(screen_cursor_point.x,screen_cursor_point.y, 100*fixation_count/funcFIXATION_LIMIT());
		//if(!flag_Pink_approved&&prev_flag_Pink_approved)
		//09.06.2015 переход false->true также должен сбрасывать fixation_count
		if(flag_Pink_approved!=prev_flag_Pink_approved)
		{
			fixation_count=0;
		}
		prev_flag_Pink_approved=flag_Pink_approved;
		
		//========================================================================================================================================
		// Искать фиксацию, только если уже оправились от предыдущей фиксации (или явно нажали на среднюю кнопку мыши), иначе уменьшаем skip_count
		if((skip_count<=0)||(true==BKB_MBUTTON_PRESSED))
		{
			// Теперь засчитывается также фиксация, если прямоугольник ProgressWindow не перемещался
			if((disp1<funcDISPERSION_LIMIT())&&(disp2<funcDISPERSION_LIMIT())||(true==BKB_MBUTTON_PRESSED)||(true==flag_Pink_approved)) 
			{
				fixation_count++;

				// Рисуем прогресс прямо на клавиатуре - только в старом режиме
				if(!gBKB_2STEP_KBD_MODE&&(BKB_MODE_KEYBOARD==Fixation::CurrentMode()))
				{
					// Показывать прогресс нажатия на клавиатуре
					POINT point_screen=point;
					if(!BKBKeybWnd::ProgressBar(&point_screen,fixation_count,100*fixation_count/funcFIXATION_LIMIT()))
					{
						fixation_count=0; // копили-копили, ан нет, сорвалось
						BKBKeybWnd::ProgressBarReset();
					}
				}

				// ---!!! Сюда же добавить Progress Bar у Transparent Window !!!
			}
			else
			{
				fixation_count=0; // копили-копили, ан нет, сорвалось
				// Корректировка: появился новый режим с розовым прямоугольником. При нём нет нужды сбрасывать Progress Bar
				if(!gBKB_2STEP_KBD_MODE&&(BKB_MODE_KEYBOARD==Fixation::CurrentMode())) BKBKeybWnd::ProgressBarReset();
			}
		}
		else
		{
			if(skip_count>0) skip_count--;
		}
		// Замечена попытка фиксации взгляда
		// Фиксация увеличивается вдвое при работе без зума
		// Добавлена возможность имитировать фиксацию нажатием средней кнопки мыши
		if((fixation_count>=funcFIXATION_LIMIT()*2)||((fixation_count>=funcFIXATION_LIMIT())&&(!BKBToolWnd::tool_modifier[3]))||
			(true==BKB_MBUTTON_PRESSED)) 
		{
			
			fixation_count=0; // первым делом сбросим эту переменную
			skip_count=funcPOSTFIXATION_SKIP();
			if(BKBToolWnd::tool_modifier[3]) skip_count*=2; // Фиксация увеличивается вдвое при работе без зума

			if(true==BKB_MBUTTON_PRESSED)
			{
				// Флаг надо сбросить, иначе начнется бесконечный круг фиксаций
				BKB_MBUTTON_PRESSED=false;
				skip_count=0; // Не ждём (важно для клавиатуры)
			}

			// Далее обрабатываем фиксацию в зависимости от текущего режима.
			Fixation::Fix(screen_cursor_point);
			BKBTranspWnd::ToTop(); // После фиксации могут всплыть окна более близкие в z-order'e
		}
		BKBMetricsWnd::OnTick((disp1+disp2)/2.0f); // Рисуем в окне метрик 100%=300 пикселов
	}
	else // Трекинг глаз не удался
	{
		//OutputDebugString(L"ONE EYE\n\r");
		BKBMetricsWnd::OnTick(-1.0f); // Пустое место отрисуем
	}

	TGD_is_processing=0; // Без этого новые данные не поступят !
}
