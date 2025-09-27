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
	Transform.cameraDist = 150;

	//������ �������� ������ ��������
	Transform.width  = 500;
	Transform.height = 500;
	Transform.depth  = 500;

	//����� ������
	Transform.CenterX = window.width  / 2;
	Transform.CenterY = window.height / 2;

	//���� �������� ��������
	Transform.angleX = Transform.timer * (3.14 / 180.);
	Transform.angleY = Transform.timer * (3.14 / 180.);
	Transform.angleZ = Transform.timer * (3.14 / 180.);

	//���������� ��������� ������ � ���������� �������
	float Vector3[8][3] = {
		{-1,	-1 ,   0},
		{-1,	 1 ,   0},
		{ 1,	 1 ,   0},
		{ 1,	-1 ,   0},

		{-1,	-1 ,  -2},
		{-1,	 1 ,  -2},
		{ 1,	 1 ,  -2},
		{ 1,	-1 ,  -2},
	};

	//���������� ������ � ����� ��������� ������
	int Index[12][2] = {
		{1,2},
		{2,3},
		{3,4},
		{4,1},

		{5,6},
		{6,7},
		{7,8},
		{8,5},

		{1,5},
		{2,6},
		{3,7},
		{4,8},
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

	//���� ���������� �� ������� ���������� ����� ��������
	for (int i = 0; i < sizeof(Index) / sizeof(Index[0]); i++)
	{
		//������������� �������������� ������ ��������
		float firstPerX = Vector3[Index[i][0] - 1][0] * Transform.cameraDist / (Vector3[Index[i][0] - 1][2] + Transform.cameraDist);
		float firstPerY = Vector3[Index[i][0] - 1][1] * Transform.cameraDist / (Vector3[Index[i][0] - 1][2] + Transform.cameraDist);
		float firstPerZ = Vector3[Index[i][0] - 1][2] * Transform.cameraDist / (Vector3[Index[i][0] - 1][2] + Transform.cameraDist);

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

		for (int j = 0; j < length; j++)
		{
			//���������� ���� ��������� �������� ��� ������ ��������� ����������
			int PixelPointX = Transform.dx * j / length + Transform.firstX + Transform.CenterX;
			int PixelPointY = Transform.dy * j / length + Transform.firstY + Transform.CenterY;

			//��������� �������� �� ������ ����
			SetPixel(window.contx, PixelPointX, PixelPointY, RGB(255, 0, 0));
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

		//�������� ����������
		Sleep(16);
	}
	return 0;
}