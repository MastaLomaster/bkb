#include <Windows.h>
#include <process.h>
#include "BKBRepErr.h"
#include "Smooth.h"
#include "Fixation.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "ToolWnd.h"
#include "Internat.h"
#include "WM_USER_messages.h"

#define DISPERSION_LIMIT 100.0 // Для отслеживания фиксаций
#define DISPERSION_HIGH_LIMIT 300.0 // Для отслеживания быстрых перемещений
#define FIXATION_LIMIT 30 // Сколько последовательных точек с низкой дисперсией считать фиксацией
#define POSTFIXATION_SKIP 30 // сколько точек пропустить после фиксации, чтобы начать считать новую фиксацию
#define CURSOR_SMOOTHING 7; // Направление движения курсора меняется только раз в CURSOR_SMOOTHING отсчетов

extern int screenX, screenY;


static int fixation_count=0; // количество точек, когда мышь почти не двигается
static int skip_count=0; // сколько точек осталось пропустить после фиксации, чтобы начать считать новую фиксацию

//extern HWND	BKBhwnd;
extern int flag_using_airmouse;

static tobiigaze_gaze_data TGD_interchange; // Буфер, куда записывается пришедшее значение для передачи в другую очередь
static volatile long TGD_is_processing=0; // Типа мьютекса для InterlockedCompareExchange

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
// 01.02.04 Её может вызывать и аэромышь
// 14.04.04 Теперь посылает сообщение другому потоку, если тот уже закончил обработку предыдущего сообщения
//===========================================================================================================
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data)
{
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
	tobiigaze_gaze_data* gazedata=&TGD_interchange;
	

	static POINT point_left={0,0}, point_right={0,0}, point={0,0}; //, last_point={0,0}, tmp_point;
	double disp1,disp2; // дисперсия в последних отсчетах левого и правого глаза
	static POINT  screen_cursor_point; //cursor_position={0,0};
	static double cursor_position_x, cursor_position_y;
	static int cursor_linear_move_counter=CURSOR_SMOOTHING; // Столько отсчетов курсор будет двигаться линейно 
	static double cursor_speed_x=0.0, cursor_speed_y=0.0; 
	static uint64_t last_timestamp=0;
	static bool mouse_inside_keyboard=false, last_mouse_inside_keyboard=false; // Для скрытия второго курсора при перемещении в область клавиатуры
	
		// Для проверки рисуем точку на экране
	// Но только если отследили оба глаза!!
	if (gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED)
	{
		//hdc=GetDC(BKBhwnd);

		// Трекинг левого глаза 
		point_left.x=screenX*gazedata->left.gaze_point_on_display_normalized.x;
		point_left.y=screenY*gazedata->left.gaze_point_on_display_normalized.y;
		disp1=BKBSmooth(&point_left, 0);
		
		// Трекинг правого глаза 
		point_right.x=screenX*gazedata->right.gaze_point_on_display_normalized.x;
		point_right.y=screenY*gazedata->right.gaze_point_on_display_normalized.y;
		disp2=BKBSmooth(&point_right, 1);
		
		point.x=(point_right.x+point_left.x)/2;
		point.y=(point_right.y+point_left.y)/2;
	
		//=================================================================================
		// Теперь о перемещениях курсора
		// Сглаживаниеы не нужно для аэромыши
		if((disp1>DISPERSION_HIGH_LIMIT)&&(disp2>DISPERSION_HIGH_LIMIT)||(2==flag_using_airmouse))
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
		if((BKB_MODE_SCROLL!=Fixation::CurrentMode())&&(BKB_MODE_KEYBOARD!=Fixation::CurrentMode()))
				BKBTranspWnd::Move(screen_cursor_point.x,screen_cursor_point.y); 
		
		// Рисуем окно со стрелкой или белое пятно на клавиатуре?
		if(BKB_MODE_KEYBOARD==Fixation::CurrentMode()) 
		{
			mouse_inside_keyboard=BKBKeybWnd::WhiteSpot(&screen_cursor_point);
			if((true==mouse_inside_keyboard)&&(false==last_mouse_inside_keyboard)) // пришли в клавиатуру
				BKBTranspWnd::Hide(); // Убрать стрелку
			else if((false==mouse_inside_keyboard)&&(true==last_mouse_inside_keyboard)) // вышли из клавиатуры
				BKBTranspWnd::Show(); // Показать стрелку

			last_mouse_inside_keyboard=mouse_inside_keyboard;
			if(!mouse_inside_keyboard)
				BKBTranspWnd::Move(screen_cursor_point.x,screen_cursor_point.y);
		}

		// При скролле рисуем прозрачное окно только тогда, когда оно попадает на тулбар
		if(BKB_MODE_SCROLL==Fixation::CurrentMode()) 
		{
			BKBToolWnd::ScrollCursor(&screen_cursor_point);
		}

		//===============================================================================================
		// Искать фиксацию, только если уже оправились от предыдущей фиксации, иначе уменьшаем skip_count
		if(skip_count<=0)
		{
			if((disp1<DISPERSION_LIMIT)&&(disp2<DISPERSION_LIMIT)) 
			{
				fixation_count++;
				if(BKB_MODE_KEYBOARD==Fixation::CurrentMode())
				{
					// Показывать прогресс нажатия на клавиатуре
					POINT point_screen=point;
					//ClientToScreen(BKBhwnd,&point_screen); ОТОВИЗМ
					//BKBKeybWnd::ProgressBar(&point_screen,fixation_count,100*fixation_count/FIXATION_LIMIT);
					if(!BKBKeybWnd::ProgressBar(&point_screen,fixation_count,100*fixation_count/FIXATION_LIMIT))
					{
						fixation_count=0; // копили-копили, ан нет, сорвалось
						BKBKeybWnd::ProgressBarReset();
					}
				}
			}
			else
			{
				fixation_count=0; // копили-копили, ан нет, сорвалось
				if(BKB_MODE_KEYBOARD==Fixation::CurrentMode()) BKBKeybWnd::ProgressBarReset();
			}
		}
		else
		{
			skip_count--;
		}
		// Замечена попытка фиксации взгляда
		if(fixation_count>=FIXATION_LIMIT) 
		{
			fixation_count=0; // первым делом сбросим эту переменную
			skip_count=POSTFIXATION_SKIP;

			// Далее обрабатываем фиксацию в зависимости от текущего режима.
			Fixation::Fix(screen_cursor_point);
			BKBTranspWnd::ToTop(); // После фиксации могут всплыть окна более близкие в z-order'e
		}


		
	}
	TGD_is_processing=0; // Без этого новые данные не поступят !
}
