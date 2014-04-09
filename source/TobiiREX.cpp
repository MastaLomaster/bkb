#include <Windows.h>
#include <process.h>
#include "BKBRepErr.h"
#include "TobiiREX.h"
#include "Smooth.h"
#include "Fixation.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "ToolWnd.h"
#include "Internat.h"

#define DISPERSION_LIMIT 100.0 // Для отслеживания фиксаций
#define DISPERSION_HIGH_LIMIT 300.0 // Для отслеживания быстрых перемещений
#define FIXATION_LIMIT 30 // Сколько последовательных точек с низкой дисперсией считать фиксацией
#define POSTFIXATION_SKIP 30 // сколько точек пропустить после фиксации, чтобы начать считать новую фиксацию
#define CURSOR_SMOOTHING 7; // Направление движения курсора меняется только раз в CURSOR_SMOOTHING отсчетов

// Заголовочные файлы из Tobii Gaze SDK
#include "tobiigaze_error_codes.h"
#include "tobiigaze.h"
#include "tobiigaze_config.h"

// Для динамической подгрузки библиотек
typedef  TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_config_init)(tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_config_get_default_eye_tracker_url)(char *url, uint32_t url_size, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API tobiigaze_eye_tracker* (TOBIIGAZE_CALL *type_tobiigaze_create)(const char *url, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_connect)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_start_tracking)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_gaze_listener gaze_callback, tobiigaze_error_code *error_code, void *user_data);


typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_stop_tracking)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_disconnect)(tobiigaze_eye_tracker *eye_tracker);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_break_event_loop)(tobiigaze_eye_tracker *eye_tracker);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_destroy)(tobiigaze_eye_tracker *eye_tracker);

typedef TOBIIGAZE_API const char* (TOBIIGAZE_CALL *type_tobiigaze_get_error_message)(tobiigaze_error_code error_code);

typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_run_event_loop)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_error_code *error_code);

HMODULE TobiiConfigDLL=0, TobiiCoreDLL=0;

// указатели на фунуции из DLL
type_tobiigaze_config_init fp_tobiigaze_config_init;
type_tobiigaze_config_get_default_eye_tracker_url fp_tobiigaze_config_get_default_eye_tracker_url;
type_tobiigaze_create fp_tobiigaze_create;
type_tobiigaze_connect fp_tobiigaze_connect;
type_tobiigaze_start_tracking fp_tobiigaze_start_tracking;
type_tobiigaze_stop_tracking fp_tobiigaze_stop_tracking;
type_tobiigaze_disconnect fp_tobiigaze_disconnect;
type_tobiigaze_break_event_loop fp_tobiigaze_break_event_loop;
type_tobiigaze_destroy fp_tobiigaze_destroy;
type_tobiigaze_get_error_message fp_tobiigaze_get_error_message=0;
type_tobiigaze_run_event_loop fp_tobiigaze_run_event_loop;

// Всякие переменные для работы с Tobii Gaze SDK видны только локально (static)
static tobiigaze_error_code tbg_error_code;
static char url[64];
static tobiigaze_eye_tracker* eye_tracker=0;

static uintptr_t tobii_thread_handler; // Хендлер потока для Gaze SDK
extern int screenX, screenY;


static int fixation_count=0; // количество точек, когда мышь почти не двигается
static int skip_count=0; // сколько точек осталось пропустить после фиксации, чтобы начать считать новую фиксацию

bool BKBTobiiREX::initialized(false);

//extern HWND	BKBhwnd;
extern int flag_using_airmouse;

//=====================================================================================
// Функция, возвращающая знак целого числа
//=====================================================================================
inline long signum(long x)
{
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}

//=====================================================================================
// Функция, которую вызывает REX, когда сообщает данные о глазах
// 01.02.04 Её может вызывать и аэромышь
//=====================================================================================
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data)
{
	//HDC hdc;
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
}

//=====================================================================================
// Поток, чья функция - запустить цикл обработки в Tobii Gaze SDK
//=====================================================================================
unsigned __stdcall TobiiREXThread(void *p)
{
	tobiigaze_error_code my_error_code;

    (*fp_tobiigaze_run_event_loop)((tobiigaze_eye_tracker*)eye_tracker, &my_error_code);
    if(my_error_code)
	{
		BKBReportError(my_error_code, __WIDEFILE__,L"tobiigaze_run_event_loop",__LINE__);
	}
	return 0;
}


//=====================================================================================
// Начало работы с устройством
//=====================================================================================
int BKBTobiiREX::Init()
{
	if(initialized) return 1; // уже инициализировали

	// -1. Загрузка DLL
	TobiiConfigDLL = LoadLibrary(L"TobiiGazeConfig32.dll");
	if(0==TobiiConfigDLL)
	{
		BKBReportError(Internat::Message(27,L"Не удалось загрузить библиотеку TobiiGazeConfig32.dll\r\nСкопируйте её в рабочий каталог программы"));
		return 1;
	}
	
	TobiiCoreDLL = LoadLibrary(L"TobiiGazeCore32.dll");
	if(0==TobiiCoreDLL)
	{
		BKBReportError(Internat::Message(28,L"Не удалось загрузить библиотеку TobiiGazeCore32.dll\r\nСкопируйте её в рабочий каталог программы"));
		return 1;
	}


	// 0. Загрузка функций из DLL
	// Пытаемся найти там нужные функции
	fp_tobiigaze_config_init=(type_tobiigaze_config_init)GetProcAddress(TobiiConfigDLL,"tobiigaze_config_init");
	fp_tobiigaze_config_get_default_eye_tracker_url=(type_tobiigaze_config_get_default_eye_tracker_url)GetProcAddress(TobiiConfigDLL,"tobiigaze_config_get_default_eye_tracker_url");
	fp_tobiigaze_create=(type_tobiigaze_create)GetProcAddress(TobiiCoreDLL,"tobiigaze_create");
	fp_tobiigaze_connect=(type_tobiigaze_connect)GetProcAddress(TobiiCoreDLL,"tobiigaze_connect");
	fp_tobiigaze_start_tracking=(type_tobiigaze_start_tracking)GetProcAddress(TobiiCoreDLL,"tobiigaze_start_tracking");
	fp_tobiigaze_stop_tracking=(type_tobiigaze_stop_tracking)GetProcAddress(TobiiCoreDLL,"tobiigaze_stop_tracking");
	fp_tobiigaze_disconnect=(type_tobiigaze_disconnect)GetProcAddress(TobiiCoreDLL,"tobiigaze_disconnect");
	fp_tobiigaze_break_event_loop=(type_tobiigaze_break_event_loop)GetProcAddress(TobiiCoreDLL,"tobiigaze_break_event_loop");
	fp_tobiigaze_destroy=(type_tobiigaze_destroy)GetProcAddress(TobiiCoreDLL,"tobiigaze_destroy");
	fp_tobiigaze_get_error_message=(type_tobiigaze_get_error_message)GetProcAddress(TobiiCoreDLL,"tobiigaze_get_error_message");
	fp_tobiigaze_run_event_loop=(type_tobiigaze_run_event_loop)GetProcAddress(TobiiCoreDLL,"tobiigaze_run_event_loop");

	if(!fp_tobiigaze_config_init||!fp_tobiigaze_config_get_default_eye_tracker_url||!fp_tobiigaze_create||
		!fp_tobiigaze_connect||!fp_tobiigaze_start_tracking||!fp_tobiigaze_stop_tracking||
		!fp_tobiigaze_disconnect||!fp_tobiigaze_break_event_loop||!fp_tobiigaze_destroy||
		!fp_tobiigaze_get_error_message||!fp_tobiigaze_run_event_loop)
	{
		BKBReportError(Internat::Message(29,L"Не удалось получить необходимые функции из TobiiGazeCore32.dll или TobiiGazeConfig32.dll"));
		return 1;
	}

	// 1. Содрано из Tobii Gaze SDK 
	// 1.1.
	(*fp_tobiigaze_config_init)(&tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __WIDEFILE__,L"tobiigaze_config_init",__LINE__);
		return 1;
	}

	// 1.2.
	(*fp_tobiigaze_config_get_default_eye_tracker_url)(url, 64, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __WIDEFILE__,L"tobiigaze_config_get_default_eye_tracker_url",__LINE__);
		return 1;
	}

	// 1.3.
	eye_tracker = (*fp_tobiigaze_create)(url, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __WIDEFILE__,L"tobiigaze_create",__LINE__);
		return 1;
	}

	// 1.4. Здесь создаём поток для Gaze SDK
	tobii_thread_handler=_beginthreadex(NULL,0,TobiiREXThread,NULL,0,NULL);
	if(1>tobii_thread_handler)
	{
		BKBReportError(__WIDEFILE__,L"start Tobii Gaze SDK loop thread",__LINE__);
		return 1;
	}

	// 1.5.
	(*fp_tobiigaze_connect)(eye_tracker, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __WIDEFILE__,L"tobiigaze_connect",__LINE__);
		return 1;
	}

	// В примере из SDK рекомендуют тут посмотреть детали устройства. Оставим на потом.
    // print_device_info(eye_tracker)

	// 1.6.
    (*fp_tobiigaze_start_tracking)(eye_tracker, &on_gaze_data, &tbg_error_code, 0);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __WIDEFILE__,L"tobiigaze_start_tracking",__LINE__);
		return 1;
	}

	initialized=true;
	return 0; // нормально отработали
}

//=====================================================================================
// Завершение работы с устройством
//=====================================================================================
int BKBTobiiREX::Halt()
{
	if(!initialized) return 1; // уже завершили работу


	// Содрано из Tobii Gaze SDK
	(*fp_tobiigaze_stop_tracking)(eye_tracker, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __WIDEFILE__,L"tobiigaze_stop_tracking",__LINE__);
		initialized=false; // На всякий случай
		return 1;
	}

    (*fp_tobiigaze_disconnect)(eye_tracker);
   
    (*fp_tobiigaze_break_event_loop)(eye_tracker);
   
    (*fp_tobiigaze_destroy)(eye_tracker);

	initialized=false;

	// Выгружаем DLL
	if(TobiiConfigDLL) FreeLibrary(TobiiConfigDLL);
	if(TobiiCoreDLL) FreeLibrary(TobiiCoreDLL);

	return 0; // нормально отработали
}


/*
// Старая версия, до динамически подключаемых DLL

//=====================================================================================
// Начало работы с устройством
//=====================================================================================
int BKBTobiiREX::Init()
{
	if(initialized) return 1; // уже инициализировали

	// 1. Содрано из Tobii Gaze SDK 
	// 1.1.
	tobiigaze_config_init(&tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_config_init",__LINE__);
		return 1;
	}

	// 1.2.
	tobiigaze_config_get_default_eye_tracker_url(url, 64, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_config_get_default_eye_tracker_url",__LINE__);
		return 1;
	}

	// 1.3.
	eye_tracker = tobiigaze_create(url, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_create",__LINE__);
		return 1;
	}

	// 1.4. Здесь создаём поток для Gaze SDK
	tobii_thread_handler=_beginthreadex(NULL,0,TobiiREXThread,NULL,0,NULL);
	if(1>tobii_thread_handler)
	{
		BKBReportError(__FILE__,"start Tobii Gaze SDK loop thread",__LINE__);
		return 1;
	}

	// 1.5.
	tobiigaze_connect(eye_tracker, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_connect",__LINE__);
		return 1;
	}

	// В примере из SDK рекомендуют тут посмотреть детали устройства. Оставим на потом.
    // print_device_info(eye_tracker)

	// 1.6.
    tobiigaze_start_tracking(eye_tracker, &on_gaze_data, &tbg_error_code, 0);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_start_tracking",__LINE__);
		return 1;
	}

	initialized=true;
	return 0; // нормально отработали
}

//=====================================================================================
// Завершение работы с устройством
//=====================================================================================
int BKBTobiiREX::Halt()
{
	if(!initialized) return 1; // уже завершили работу

	// Содрано из Tobii Gaze SDK
	tobiigaze_stop_tracking(eye_tracker, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_stop_tracking",__LINE__);
		initialized=false; // На всякий случай
		return 1;
	}

    tobiigaze_disconnect(eye_tracker);
   
    tobiigaze_break_event_loop(eye_tracker);
   
    tobiigaze_destroy(eye_tracker);

	initialized=false;
	return 0; // нормально отработали
}
*/