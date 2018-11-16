#ifndef __BKB_WHEELCHAIR
#define __BKB_WHEELCHAIR

// ���������� ���������������� �������-��������
// �������� ������ �������� ������ BKBGrid

class BKBWheelChair
{
public:
	static int IsItYours(); 
	static LPRECT PinkFrame(int _x, int _y, LONG width, LONG height);
	static LPRECT PinkFrame2(int _x, int _y, LONG width, LONG height); // ������ �������, ��� ������� ����
	static void PinkFrameMissed();
	static void OnPaint(HDC hdc, LONG width, LONG height);
	static void OnPaint2(HDC hdc, LONG width, LONG height);
	static int FixationLimit();
	static int PostFixationSkip();
	static int selected_cell;

//protected:
	
};

#endif