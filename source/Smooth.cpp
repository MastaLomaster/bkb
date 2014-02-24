#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include "Smooth.h"

#define BKB_NUM_SMOOTH_POINTS 12
// Массив для сглаживания последних точек
static POINT spoints[2][BKB_NUM_SMOOTH_POINTS]={};
// Массив последних СГЛАЖЕННЫХ точек
static POINT processed_points[2][BKB_NUM_SMOOTH_POINTS]={};
static int current_point[2]={0,0};
static double dispersion[2]={1000,1000};

//==============================================================
// Сглаживание данных от трекера
// левый глаз - 0, правый - 1
//==============================================================
double BKBSmooth(POINT *point, bool eye)
{
	int i;
	

	// Сглаживаем BKB_NUM_SMOOTH_POINTS последних точек
	spoints[eye][current_point[eye]]=*point;
		
	point->x=0;point->y=0;
	for(i=0;i<BKB_NUM_SMOOTH_POINTS;i++)
	{
		point->x+=spoints[eye][i].x;
		point->y+=spoints[eye][i].y;
	}
		point->x/=BKB_NUM_SMOOTH_POINTS;
		point->y/=BKB_NUM_SMOOTH_POINTS;

		processed_points[eye][current_point[eye]]=*point;

		// Передвигаемся на следующую позицию в кольцевом буфере
		current_point[eye]++;
		if(current_point[eye]>=BKB_NUM_SMOOTH_POINTS) 
		{
			current_point[eye]=0;
		}
			
		double mean_x=0.0, mean_y=0.0;
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
