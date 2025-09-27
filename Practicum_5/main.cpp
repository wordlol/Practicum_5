#include "Windows.h"
#include "math.h"

//��������� ��� ��������� ������ � windows ����
struct
{
	//�����������, ���������� � ������� ��� windows
	RECT rc;
	HINSTANCE hIns;
	HWND hWnd;
	HDC dev_cont, contx;
	MSG msg;
	BOOL gbool = true;

	//���������� ������ ������ � ����� ��������
	int width = GetSystemMetrics(SM_CXSCREEN), height = GetSystemMetrics(SM_CYSCREEN);
} window;

//��������� � ������� � �����������
struct
{
	float dx, dy, dz, CenterX, CenterY;
	float firstX, firstY, firstZ ,secondX, secondY, secondZ;
	int width, height, depth;
	float angleX, angleY, angleZ;
	int timer;
	int cameraDist;
} Transform;

//��������� ������ ���������
static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
};

//�������� windows ����
void InitWindow()
{
	//��� ������ ����
	const char* NameClass = "Window";

	//������ ����
	window.rc = { 0,0,window.width,window.height
	};

	//���� �������
	AdjustWindowRect(&window.rc, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	//���������� ������ ����
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = NameClass;
	wc.hInstance = window.hIns;
	wc.lpfnWndProc = &WindowProc;

	//����������� ������ ����
	auto NameClassId = RegisterClassEx(&wc);

	//���������� ����
	window.hWnd = CreateWindowEx(
		NULL,
		MAKEINTATOM(NameClassId),
		"practicum5",
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		window.rc.right - window.rc.left,
		window.rc.bottom - window.rc.top,
		NULL,
		NULL,
		window.hIns,
		NULL
	);

	//����� ����
	ShowWindow(window.hWnd, SW_SHOW);
}

//��������� ����������� .bmp
void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall)
{
	HBITMAP hbm, hOldbm;
	HDC hMemDC;
	BITMAP bm;

	hMemDC = CreateCompatibleDC(hDC);
	hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);

	if (hOldbm)
	{
		GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm);
		StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		SelectObject(hMemDC, hOldbm);
	}

	DeleteDC(hMemDC);
}

//������� ��������� ���� ������
void DrawTriangle(float X1, float X2, float X3 , float Y1, float Y2, float Y3, int Color)
{
	int R = 0;
	int G = 0;
	int B = 0;
	//������� �����
	if (Color == 1)
	{R = 255; G = 0; B = 0;}
	if (Color == 2)
	{R = 0; G = 255; B = 0;}	
	if (Color == 3)
	{R = 0; G = 0; B = 255;}
	if (Color == 4)
	{R = 255; G = 255; B = 0;}
	if (Color == 5)
	{R = 255; G = 0; B = 255;}
	if (Color == 6)
	{R = 0; G = 255; B = 255;}

	int AY = Y1 * Transform.height / 2 + Transform.CenterY;
	int BY = Y2 * Transform.height / 2 + Transform.CenterY;
	int CY = Y3 * Transform.height / 2 + Transform.CenterY;

	int AX = X1 * Transform.width  / 2 + Transform.CenterX;
	int BX = X2 * Transform.width  / 2 + Transform.CenterX;
	int CX = X3 * Transform.width  / 2 + Transform.CenterX;

	int temp;
	//���������� 3 ������ �� minY � maxY
	if (AY > BY)
	{
		temp = AY;
		AY = BY;
		BY = temp;
	}
	if (AY > CY)
	{
		temp = AY;
		AY = CY;
		CY = temp;
	}
	if (BY > CY)
	{
		temp = BY;
		BY = CY;
		CY = temp;
	}

	//�������� ������� ������������ ���������� ������
	for (int i = AY; i < CY; i++)
	{
		int x1 = AX + (i - AY) * (CX - AX) / (CY - AY);
		int x2;
		if (i < BY)
		{
			x2 = AX + (i - AY) * (BX - AX) / (BY - AY);
		}
		else
		{
			if (CY == BY)
			{
				x2 = BX;
			}
			else
			{
				x2 = BX + (i - BY) * (CX - BX) / (CY - BY);
			}
		}
		if (x1 > x2)
		{
			temp = x1;
			x1 = x2;
			x2 = temp;
		}
		for (int j = x1; j < x2; j++)
		{
			SetPixel(window.contx, j, i, RGB(R, G, B));
		}
	}
}

//�������� ������� ����������
void InitApp()
{
	//�������� � ����������� ��������� ��������� � ������ ����������
	window.dev_cont = GetDC(window.hWnd);
	window.contx = CreateCompatibleDC(window.dev_cont);
	SelectObject(window.contx, CreateCompatibleBitmap(window.dev_cont, window.width, window.height));
}

//���������� ����������
void UpdateApp()
{
	//������
	Transform.timer++;

	//��������� ������ �� ������ xyz
	Transform.cameraDist = 400;

	//������ �������� ������ ��������
	Transform.width  = 100;
	Transform.height = 100;
	Transform.depth  = 0;

	//����� ������
	Transform.CenterX = window.width  / 2;
	Transform.CenterY = window.height / 2;

	//���� �������� ��������
	Transform.angleX = 0 * (3.14 / 180.);
	Transform.angleY = Transform.timer * (3.14 / 180.);
	Transform.angleZ = 0 * (3.14 / 180.);

	//���������� ��������� ������ � ���������� �������
	float Vector3[8][3] = {
		{-1,	-1,   1},
		{-1,	 1,   1},
		{ 1,	 1,   1},
		{ 1,	-1,   1},

		{-1,	-1,  -1},
		{-1,	 1,  -1},
		{ 1,	 1,  -1},
		{ 1,	-1,  -1},
	};

	//���������� ������ � ����� ��������� ������
	int Index[36][2] = {
		{1,2}, 
		{2,3}, 
		{3,1}, 

		{1,3},
		{3,4},
		{4,1},

		{5,6}, 
		{6,7}, 
		{7,5}, 

		{5,7},
		{7,8},
		{8,5},

		{1,5}, 
		{5,6}, 
		{6,1}, 

		{1,6},
		{6,2},
		{2,1},

		{2,6}, 
		{6,7}, 
		{7,2}, 

		{2,7},
		{7,3},
		{3,2},

		{3,7}, 
		{7,8}, 
		{8,3}, 

		{3,8},
		{8,4},
		{4,3},

		{4,8}, 
		{8,5}, 
		{5,4}, 

		{4,5},
		{5,1},
		{1,4},
	};

	//������������ + ���� + z-buffer
 	float Poligon[12][5] =
	{
		//front //red
		{1,2,3 ,1 ,0},
		{1,3,4 ,1 ,0},

		//back //green
		{5,6,7 ,2 ,0},
		{5,7,8 ,2 ,0},
		
		//left //blue
		{1,2,6 ,3 ,0},
		{1,6,5 ,3 ,0},
				
		//right //red green
		{8,7,3 ,4 ,0},
		{8,3,4 ,4 ,0},
				  
		//top //red blue
		{2,6,7 ,5 ,0},
		{2,7,3 ,5 ,0},
		
		//bottom //green blue
		{5,1,4 ,6 ,0},
		{5,4,8 ,6 ,0},
	};

	//�������������� ������ �� 3 ����� ��������
	for (int i = 0; i < sizeof(Vector3)/sizeof(Vector3[0]); i++)
	{
		//���������� ��� ��������� ������ �� ������� ��������
		float x = Vector3[i][0];
		float y = Vector3[i][1];
		float z = Vector3[i][2];

		//�������������� �� X
		Vector3[i][1] = y * cos(Transform.angleX) + z * -sin(Transform.angleX);
		Vector3[i][2] = y * sin(Transform.angleX) + z * cos(Transform.angleX);
		
		x = Vector3[i][0];
		z = Vector3[i][2];

		//�������������� �� Y
		Vector3[i][0] = x *  cos(Transform.angleY) + z * sin(Transform.angleY);
		Vector3[i][2] = x * -sin(Transform.angleY) + z * cos(Transform.angleY);

		x = Vector3[i][0];
		y = Vector3[i][1];

		//�������������� �� Z
		Vector3[i][0] = x * cos(Transform.angleZ) + y * -sin(Transform.angleZ) ;
		Vector3[i][1] = x * sin(Transform.angleZ) + y *  cos(Transform.angleZ) ;

	}

	//���������� �������� z ��� z-buffer
	for (int i = 0; i < sizeof(Poligon) / sizeof(Poligon[0]); i++)
	{
		float Z1 = Vector3[(int)Poligon[i][0] - 1][2];
		float Z2 = Vector3[(int)Poligon[i][1] - 1][2];
		float Z3 = Vector3[(int)Poligon[i][2] - 1][2];

		float Zmidle = (float)(Z1 + Z2 + Z3) / (float)3;
		Poligon[i][4] = Zmidle;
	}

	//����������� ���������� ��������� z-buffer
	bool sort = false;
	while (!sort)
	{
		sort = true;
		for (int i = 1; i < sizeof(Poligon) / sizeof(Poligon[0]); i++)
		{
			float prev = Poligon[i - 1][4];
			float current = Poligon[i][4];

			if (prev > current)
			{
				float temp[5];
				for (int j = 0; j < sizeof(Poligon[0]) / sizeof(Poligon[0][0]); j++)
				{
					temp[j] = Poligon[i - 1][j];
					Poligon[i - 1][j] = Poligon[i][j];
					Poligon[i][j] = temp[j];
					sort = false;
				}
			}
		}
	}

	//���� ������� ������ �� ������� ���������
	for (int i = 0; i < sizeof(Poligon) / sizeof(Poligon[0]); i++)
	{
		float Z1 = Vector3[(int)Poligon[i][0] - 1][2];
		float Z2 = Vector3[(int)Poligon[i][1] - 1][2];
		float Z3 = Vector3[(int)Poligon[i][2] - 1][2];

		float X1 = Vector3[(int)Poligon[i][0] - 1][0] * Transform.cameraDist / (Z1 + Transform.cameraDist);
		float X2 = Vector3[(int)Poligon[i][1] - 1][0] * Transform.cameraDist / (Z2 + Transform.cameraDist);
		float X3 = Vector3[(int)Poligon[i][2] - 1][0] * Transform.cameraDist / (Z3 + Transform.cameraDist);
													 
		float Y1 = Vector3[(int)Poligon[i][0] - 1][1] * Transform.cameraDist / (Z1 + Transform.cameraDist);
		float Y2 = Vector3[(int)Poligon[i][1] - 1][1] * Transform.cameraDist / (Z2 + Transform.cameraDist);
		float Y3 = Vector3[(int)Poligon[i][2] - 1][1] * Transform.cameraDist / (Z3 + Transform.cameraDist);
		
		DrawTriangle(X1, X2, X3, Y1, Y2, Y3, Poligon[i][3]);
	}

	//���� ���������� �� ������� ���������� ����� ��������
	for (int i = 0; i < sizeof(Index) / sizeof(Index[0]); i++)
	{
		//������������� �������������� ������ ��������
		float firstPerX  = Vector3[Index[i][0] - 1][0] * Transform.cameraDist / (Vector3[Index[i][0] - 1][2] + Transform.cameraDist);
		float firstPerY  = Vector3[Index[i][0] - 1][1] * Transform.cameraDist / (Vector3[Index[i][0] - 1][2] + Transform.cameraDist);
		float firstPerZ  = Vector3[Index[i][0] - 1][2] * Transform.cameraDist / (Vector3[Index[i][0] - 1][2] + Transform.cameraDist);

		float secondPerX = Vector3[Index[i][1] - 1][0] * Transform.cameraDist / (Vector3[Index[i][1] - 1][2] + Transform.cameraDist);
		float secondPerY = Vector3[Index[i][1] - 1][1] * Transform.cameraDist / (Vector3[Index[i][1] - 1][2] + Transform.cameraDist);
		float secondPerZ = Vector3[Index[i][1] - 1][2] * Transform.cameraDist / (Vector3[Index[i][1] - 1][2] + Transform.cameraDist);

		//������ ��������� �����
		Transform.firstX = firstPerX * Transform.width  / 2;
		Transform.firstY = firstPerY * Transform.height / 2;
		Transform.firstZ = firstPerZ * Transform.depth  / 2;

		//����� ��������� �����
		Transform.secondX = secondPerX * Transform.width  / 2;
		Transform.secondY = secondPerY * Transform.height / 2;
		Transform.secondZ = secondPerZ * Transform.depth  / 2;

		//��������� ������ ����� ���������
		Transform.dx = Transform.secondX - Transform.firstX;
		Transform.dy = Transform.secondY - Transform.firstY;
		Transform.dz = Transform.secondZ - Transform.firstZ;

		//����������� ����� ���������� �� ������� x, y, z �� ������� ��������
		int length = sqrt(pow(Transform.dx, 2) + pow(Transform.dy, 2) + pow(Transform.dz, 2));

		//���������� �����
		for (int j = 0; j < length; j++)
		{
			//���������� ���� ��������� �������� ��� ������ ��������� ����������
			int PixelPointX = Transform.dx * j / length + Transform.firstX + Transform.CenterX;
			int PixelPointY = Transform.dy * j / length + Transform.firstY + Transform.CenterY;

			//��������� �������� �� ������ ����
			SetPixel(window.contx, PixelPointX, PixelPointY, RGB(255, 255, 255));
		}
	}
}

//��������� ������ ��������� �����
void UpdateKeyCode()
{
	//����� �� ���������� �� ESC
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		window.msg.message = WM_QUIT;
	}
}

//���������� �����������
void UpdateImage()
{
	BitBlt(window.dev_cont, 0, 0, window.width, window.height, window.contx, 0, 0, SRCCOPY);
	//��������� ������� ����
	ShowBitmap(window.contx, 0, 0, window.width, window.height, (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
}

//���� � ���������
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	InitWindow();
	InitApp();

	//�������� ���� ���������� ����������
	while (window.gbool)
	{
		//��������� �������� ��� ����
		while (PeekMessage(&window.msg, NULL, 0, 0, PM_REMOVE))
		{
			UpdateKeyCode();

			//���������� ���������
			if (window.msg.message == WM_QUIT)
			{
				window.gbool = false;
				break;
			}
			TranslateMessage(&window.msg);
			DispatchMessage(&window.msg);
		}

		UpdateImage();
		UpdateApp();

	}
	return 0;
}