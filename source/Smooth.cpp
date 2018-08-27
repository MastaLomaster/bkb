#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include "Smooth.h"

// При выставленном BKB_MOVING_AVERAGE используется тупой алгоритм скользящего среднего
// При сброшенном BKB_MOVING_AVERAGE - рекурсивный фильтр НЧ, как в "PONTUS OLSSON/Real-time and Offline Filters for Eye Tracking"

#define BKB_MOVING_AVERAGE 1
int G_alpha_for_filtering=25;
static POINT last_point={0,0};

#define BKB_NUM_SMOOTH_POINTS 18
// Массив для сглаживания последних точек
static POINT spoints[2][BKB_NUM_SMOOTH_POINTS]={0};
// Массив квадратов отклонений от среднего
static double deviation[2][BKB_NUM_SMOOTH_POINTS]={0};

// Массив последних СГЛАЖЕННЫХ точек
static POINT processed_points[2][BKB_NUM_SMOOTH_POINTS]={0};
static int current_point[2]={0,0};
static double dispersion[2]={1000,1000};

extern int tracking_device;


//==============================================================
// Сглаживание данных от трекера
// левый глаз - 0, правый - 1
//==============================================================
double BKBSmooth(POINT *point, bool eye)
{
	int i;
	double mean_x=0.0, mean_y=0.0;

	// Сглаживаем BKB_NUM_SMOOTH_POINTS последних точек
	// Это не нужно для аэромыши
	// 30.11.2015 Нужно в режиме DEBUG, в котором имитируется нестабильность определения взгляда
#ifndef _DEBUG
	if(2!=tracking_device)
#endif
	{
		spoints[eye][current_point[eye]]=*point;
		
		// Старый алгоритм - тупое скользящее среднее
		if(BKB_MOVING_AVERAGE)
		{
			// 30.11.2015 Выкидываем две самые плохие точки
			// 1. Сначала вычисляем среднее
			for(i=0;i<BKB_NUM_SMOOTH_POINTS;i++)
			{
				mean_x+=spoints[eye][i].x;
				mean_y+=spoints[eye][i].y;
			}
			mean_x/=BKB_NUM_SMOOTH_POINTS;
			mean_y/=BKB_NUM_SMOOTH_POINTS;
			
			// 2. Для каждой точки находим квадрат отклонения
			for(i=0;i<BKB_NUM_SMOOTH_POINTS;i++)
			{
				deviation[eye][i]=(mean_x-spoints[eye][i].x)*(mean_x-spoints[eye][i].x)+(mean_y-spoints[eye][i].y)*(mean_y-spoints[eye][i].y);
			}
			
			// 3. Находим два самых больших оппортуниста
			int max_num1=0, max_num2=0; // Номера самых больших отклонений
			double max_dev1=0.0, max_dev2=0.0; // Величины самых больших отклонений

			for(i=0;i<BKB_NUM_SMOOTH_POINTS;i++)
			{
				if(deviation[eye][i]>max_dev1)
				{
					max_dev1=deviation[eye][i];
					max_num1=i;
				}
				else if(deviation[eye][i]>max_dev2)
				{
					max_dev2=deviation[eye][i];
					max_num2=i;
				}
			}

			// 4. Продолжаем, как было раньше, но игнорируем max1 и max2
			point->x=0;point->y=0;
			for(i=0;i<BKB_NUM_SMOOTH_POINTS;i++)
			{
				if((max_num1!=i)&&(max_num2!=i))
				{
					point->x+=spoints[eye][i].x;
					point->y+=spoints[eye][i].y;
				}
			}
			// Теперь с учётом двух выброшенных точек
			point->x/=BKB_NUM_SMOOTH_POINTS-2;
			point->y/=BKB_NUM_SMOOTH_POINTS-2;
		}
		else // рекурсивный фильтр НЧ, как в "PONTUS OLSSON/Real-time and Offline Filters for Eye Tracking"
		{
			point->x=(LONG)((point->x+G_alpha_for_filtering*last_point.x)/(1.0+G_alpha_for_filtering));
			point->y=(LONG)((point->y+G_alpha_for_filtering*last_point.y)/(1.0+G_alpha_for_filtering));
			last_point=*point;
		}



	}

	processed_points[eye][current_point[eye]]=*point;

	// Передвигаемся на следующую позицию в кольцевом буфере
	current_point[eye]++;
	if(current_point[eye]>=BKB_NUM_SMOOTH_POINTS) 
	{
		current_point[eye]=0;
	}
			
	mean_x=0.0, mean_y=0.0;
	// Ищем дисперсию в последних BKB_NUM_SMOOTH_POINTS СГЛАЖЕННЫХ точках
	// Сначала вычисляем среднее
	for(i=0;i<BKB_NUM_SMOOTH_POINTS;i++)
	{
		mean_x+=processed_points[eye][i].x;
		mean_y+=processed_points[eye][i].y;
	}
	mean_x/=BKB_NUM_SMOOTH_POINTS;
	mean_y/=BKB_NUM_SMOOTH_POINTS;
	// А теперь уже собственно дисперсию
	dispersion[eye]=0.0;
	for(i=0;i<BKB_NUM_SMOOTH_POINTS;i++)
	{
		dispersion[eye]+=(mean_x-processed_points[eye][i].x)*(mean_x-processed_points[eye][i].x);
		dispersion[eye]+=(mean_y-processed_points[eye][i].y)*(mean_y-processed_points[eye][i].y);
	}
	dispersion[eye]=sqrt(dispersion[eye]);
	
			// Для отладки и понимания пределов дисперсии (пока берём границу дисперсии за 100)
			//char msgbuf[1024];
			//sprintf(msgbuf,"%d %f\n",eye,dispersion[eye]);
			//OutputDebugString(msgbuf); 

	return dispersion[eye];
}
