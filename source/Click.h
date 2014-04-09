// Создаёт и проигрывает по требованию звуки при нажатии на клавиши
#ifndef __BKB_CLICK
#define __BKB_CLICK

#define BKB_NUM_CLICKS 4
#define BKB_CLICK_SAMPLES 660

class BKBClick
{
public:
	static void Init(); // Инициализация звуков
	static void Halt(); // Чистка памяти
	static void Play(int sound_num=-1);
protected:
	static void FillSamples(bool harmonic=true);
	static HWAVEOUT hWaveOut;
	static WAVEHDR waveheader;
	static unsigned short samples[BKB_NUM_CLICKS][BKB_CLICK_SAMPLES];
};

#endif