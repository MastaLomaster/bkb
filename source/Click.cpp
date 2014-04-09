#include <Windows.h>
#include <mmsystem.h>
#include <math.h>
#include "Click.h"
#include "BKBRepErr.h"
#include "Internat.h"

#define HARMONIC false
#define PI 3.14159

HWAVEOUT BKBClick::hWaveOut=0;
unsigned short BKBClick::samples[BKB_NUM_CLICKS][BKB_CLICK_SAMPLES];
WAVEHDR BKBClick::waveheader;

// до ми соль до
static float freq[BKB_NUM_CLICKS]={523.25f,659.26f,784.00f,1046.50f};

//=====================================================================
// Заполняем массив семплов данными
//=====================================================================
void BKBClick::FillSamples(bool harmonic)
{
	int i,j;

	if(!harmonic)
	{
		freq[0]=700.0f;
		freq[1]=750.0f;
		freq[2]=800.0f;
		freq[3]=850.0f;
	}
	else
	{
		freq[0]=523.25f;
		freq[1]=659.26f;
		freq[2]=784.00f;
		freq[3]=1046.50f;
	}

	for(i=0;i<BKB_NUM_CLICKS;i++)
	{
		for(j=0;j<BKB_CLICK_SAMPLES;j++)
		{
			// Огибающая - берём косинус + 1 в интервале от -PI до PI
			// Сам сигнал: sin(2*PI*freq[i]) (Учитывая, что сигнал занимает 660/44100=0.015 секунды)
			//samples[i][j]=10000*(1+cos(-PI+2*PI/BKB_CLICK_SAMPLES*j))*sin(freq[i]*0.015*(2.0*PI/BKB_CLICK_SAMPLES*j));
			// Попробуем чуток уводить частоту вверх
			samples[i][j]=10000*(1+cos(-PI+2*PI/BKB_CLICK_SAMPLES*j))*sin((freq[i]+freq[i]*0.5*j/BKB_CLICK_SAMPLES)*0.015*(2.0*PI/BKB_CLICK_SAMPLES*j));
		}
	} // for i


}

//======================================================================
// Инициализация звуков
//======================================================================
void BKBClick::Init()
{
	WAVEFORMATEX wfx;
	
	FillSamples(HARMONIC); // Тут заполним красивыми звуками

	ZeroMemory(&waveheader, sizeof(WAVEHDR));

	// Подготовка устройства для воспроизведения звуков
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16; 
    wfx.nChannels = 1; 
    wfx.cbSize = 0;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = 2;
    wfx.nAvgBytesPerSec = 88200;

	// Сезам,...
    if(MMSYSERR_NOERROR!=waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL))
	{
		BKBReportError(Internat::Message(30,L"Работать будем беззвучно. Что-то не заладилось со звуковым устройством..."));
		hWaveOut=0;
		return;
	}

	// Создадим waveheader с первым звуком, чтоб не пустовал
	waveheader.dwBufferLength = (DWORD)(BKB_CLICK_SAMPLES/2); // Ибо два байта на отсчёт
	waveheader.lpData = (char *) (&samples[0][0]);

	waveOutPrepareHeader(hWaveOut, &waveheader, sizeof(WAVEHDR));
}

//======================================================================
// Чистим за собой
//======================================================================
void BKBClick::Halt()
{
	waveOutReset(hWaveOut);
	waveOutUnprepareHeader(hWaveOut,&waveheader,sizeof(waveheader));
	waveOutClose(hWaveOut);
}

//======================================================================
// Играем
//======================================================================
void  BKBClick::Play(int sound_num)
{
	if(!hWaveOut) return; // Звук не инициализирован или отсутствует

	if(-1==sound_num)
	{
		sound_num=(rand()*BKB_NUM_CLICKS-1)/RAND_MAX;
	}

	// Защита от дураков
	if((sound_num<0)||(sound_num>=BKB_NUM_CLICKS)) sound_num=0;

	// Если что играли - перестаём
	waveOutReset(hWaveOut);
	waveOutUnprepareHeader(hWaveOut,&waveheader,sizeof(waveheader));

	// Играем новый звук
	waveheader.lpData = (char *) (&samples[sound_num][0]);
	waveOutPrepareHeader(hWaveOut, &waveheader, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &waveheader, sizeof(WAVEHDR));
}
