#include "Windows.h"
#include "math.h"

/** стурктура где хран€тьс€ данные о windows окне
*/
struct
{
	/// дескрипторы, контейнеры и буфферы дл€ windows
	RECT rc;
	HINSTANCE hIns;
	HWND hWnd;
	HDC dev_cont, contx;
	MSG msg;
	BOOL gbool = true;

	int width = GetSystemMetrics(SM_CXSCREEN), height = GetSystemMetrics(SM_CYSCREEN); /// определ€ет размер экрана в вашей сиситеме
} window;

/** структура с данными о координатах
*/
struct
{
	int x0,
		y0,
		x1,
		y1,
		dx, 
		dy,
		signX,
		signY,
		direction;
} Transform;

/** обработка потока сообщений
*/
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

/** создани€ windows окна
*/
void InitWindow()
{
	const char* NameClass = "Window"; /// им€ класса окна

	window.rc = { 0,0,window.width,window.height}; /// размер окна

	AdjustWindowRect(&window.rc, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE); /// учет размера

	WNDCLASSEX wc = { 0 }; /// дескриптор класса окна
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = NameClass;
	wc.hInstance = window.hIns;
	wc.lpfnWndProc = &WindowProc;

	auto NameClassId = RegisterClassEx(&wc); /// регистраци€ класса окна

	window.hWnd = CreateWindowEx( /// деструктор окна
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

	ShowWindow(window.hWnd, SW_SHOW); /// показ окна
}

/** отрисовка изображений .bmp
*/
void ShowBitmap(HDC hDC, int x, int y, int x0, int y0, HBITMAP hBitmapBall)
{
	HBITMAP hbm, hOldbm;
	HDC hMemDC;
	BITMAP bm;

	hMemDC = CreateCompatibleDC(hDC);
	hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);

	if (hOldbm)
	{
		GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm);
		StretchBlt(hDC, x, y, x0, y0, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		SelectObject(hMemDC, hOldbm);
	}

	DeleteDC(hMemDC);
}

/** загрузка модулей приложени€
*/
void InitApp()
{
	/// создание и иниализаци€ контекста устройсва и девайс устройства
	window.dev_cont = GetDC(window.hWnd);
	window.contx = CreateCompatibleDC(window.dev_cont);
	SelectObject(window.contx, CreateCompatibleBitmap(window.dev_cont, window.width, window.height));
}


/** ‘ункци€ дл€ загрузки данных дл€ построени€ линии
*/
void InitTransformData(int x0, int y0, int x1, int y1)
{
	Transform.x0 = x0;
	Transform.y0 = y0;
	Transform.x1 = x1;
	Transform.y1 = y1;

	Transform.dx = abs(x1 - x0);
	Transform.dy = abs(y1 - y0);

	if (x0 < x1)
		Transform.signX = 1;
	else
		Transform.signX = -1;

	if (y0 < y1)
		Transform.signY = 1;
	else
		Transform.signY = -1;

	Transform.direction = Transform.dx - Transform.dy;
}

/** ‘ункци€ отрисвоки линии на экране окна по Ѕрезенхэйму
*/
void DrawLine()
{
	bool StatusX = false;
	bool StatusY = false;

	while (true)
	{
		SetPixel(window.contx, Transform.x0, Transform.y0, RGB(255, 0, 0)); /// отрисовки линии

		if (Transform.direction > -Transform.dy)
		{
			if (Transform.x0 != Transform.x1)
			{
				Transform.direction -= Transform.dy; /// изменение направлени€ по y
				Transform.x0 += Transform.signX; /// применени€ сдвига пиксел€ на x
			}
		}
		if (Transform.direction < Transform.dx)
		{
			if (Transform.y0 != Transform.y1)
			{
				Transform.direction += Transform.dx; /// изменение направлени€ по x
				Transform.y0 += Transform.signY; /// применени€ сдвига пиксел€ на y
			}
		}

		/// проверка на завершение отрисовки линии
		if (Transform.x0 == Transform.x1)
			StatusX = true;
		if (Transform.y0 == Transform.y1)
			StatusY = true;

		if (StatusX && StatusY)
			break;
	}
}

/** ќбновление приложени€
*/
void UpdateApp()
{
	InitTransformData(100, 300, 1900, 200);
	DrawLine();
}


/** обработка команд устройств ввода
*/
void UpdateKeyCode()
{
	//выход из приложени€ на ESC
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		window.msg.message = WM_QUIT;
	}
}

/** обновление изображений
*/
void UpdateImage()
{
	BitBlt(window.dev_cont, 0, 0, window.width, window.height, window.contx, 0, 0, SRCCOPY);
	ShowBitmap(window.contx, 0, 0, window.width, window.height, (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE)); /// отрисовка заднего фона
}

/** вход в программу
*/
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	InitWindow();
	InitApp();

	/// основной цикл обновлени€ приложени€
	while (window.gbool)
	{
		/// обработка соощений дл€ окна
		while (PeekMessage(&window.msg, NULL, 0, 0, PM_REMOVE))
		{
			UpdateKeyCode();

			//отбработка сообщений
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

		Sleep(16); /// задержка обновлени€ кадров
	}
	return 0;
}