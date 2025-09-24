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
	float x, y, startX, startY, dx, dy;
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
	//������ ������ �� ������ � ������ � ��� �� ��������� ����� ������ ��������� �����
	Transform.startX = window.width / 2;
	Transform.startY = window.height / 2; 

	//����� ���� ����� ���������� �����
	Transform.x = 1600;
	Transform.y = 400;

	//������ ���������
	Transform.dx = Transform.x - Transform.startX;
	Transform.dy = Transform.y - Transform.startY;

	//����������� ����� ���������� �� ������� x, y �� ������� ��������
	int length = sqrt(pow(Transform.dx, 2) + pow(Transform.dy, 2));

	for (int i = 0; i < length; i++)
	{
		//���������� ���� ��������� �������� ��� ������ ��������� ����������
		int PixelPointX = Transform.dx * i / length + Transform.startX;
		int PixelPointY = Transform.dy * i / length + Transform.startY;

		//��������� �������� �� ������ ����
		SetPixel(window.contx, PixelPointX, PixelPointY, RGB(255, 0, 0));
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