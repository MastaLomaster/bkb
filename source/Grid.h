#ifndef __BKB_GRID
#define __BKB_GRID

class BKBGrid
{
public:
	static bool f_grid_only; // �����, ��� ������� ������������ ������ Grid

	static bool IsMinimized(){return f_minimized;}
	static int NumCells();
	static int NumActiveCells(){return current_grid->num_active;}

	//static int IsItYours(LONG x, LONG y, LONG width, LONG height); 
	static int IsItYours(); 
	static LPRECT PinkFrame(int _x, int _y, LONG width, LONG height);
	static void OnPaint(HDC hdc, LONG width, LONG height);
	
	static void Load();
	static void ShowCursor(POINT *p);
	
	// ���� ��� �������
	static void TestData();
	
protected:
	static bool IsTopLevel() {if(!current_grid->parent) return true; else return false;}
	static void LevelUp(){if(current_grid->parent) current_grid=current_grid->parent;}
	static void Activate(int cell);
	static BKBGrid *FindOrCreate(int l0, int l1, int l2, int _num_levels);

	static bool f_minimized; // � ��������� ������ ���� Grid ��������������
	
	static int selected_cell;

	static BKBGrid mg; // Master Grid - �������� �������
	static BKBGrid *current_grid; // ��������� �� ������� grid

	// �� ������ ����������
	BKBGrid *parent;
	BKBGrid *child[16];
	int num_active;
	TCHAR soundfile[16]; // � ���������� "grid\XXXX.WAV" - �������� 14 ��������, ������� ����������� ����
	HBITMAP hbm;
	int hbm_width, hbm_height;

	BKBGrid():parent(0),child(),num_active(0),soundfile(),hbm(0){};
	BKBGrid(BKBGrid *_parent, int cell, TCHAR *_soundfile, HBITMAP _hbm);
	~BKBGrid();

};

#endif