#include "Windows.h"
#include "math.h"

//стурктура где хран€тьс€ данные о windows окне
struct
{
	//дескрипторы, контейнеры и буфферы дл€ windows
	RECT rc;
	HINSTANCE hIns;
	HWND hWnd;
	HDC dev_cont, contx;
	MSG msg;
	BOOL gbool = true;

	//определ€ет размер экрана в вашей сиситеме
	int width = GetSystemMetrics(SM_CXSCREEN), height = GetSystemMetrics(SM_CYSCREEN);
} window;

//структура с данными о координатах
struct
{
	int dx, dy, dz, CenterX, CenterY;
	int firstX, firstY, firstZ ,secondX, secondY, secondZ;
	int width, height, depth;
} Transform;

//обработка потока сообщений
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

//создани€ windows окна
void InitWindow()
{
	//им€ класса окна
	const char* NameClass = "Window";

	//размер окна
	window.rc = { 0,0,window.width,window.height
	};

	//учет размера
	AdjustWindowRect(&window.rc, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	//дескриптор класса окна
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = NameClass;
	wc.hInstance = window.hIns;
	wc.lpfnWndProc = &WindowProc;

	//регистраци€ класса окна
	auto NameClassId = RegisterClassEx(&wc);

	//деструктор окна
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

	//показ окна
	ShowWindow(window.hWnd, SW_SHOW);
}

//отрисовка изображений .bmp
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

//загрузка модулей приложени€
void InitApp()
{
	//создание и иниализаци€ контекста устройсва и девайс устройства
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
	while (true)
	{
		SetPixel(window.contx, Transform.x0, Transform.y0, RGB(255, 0, 0)); /// отрисовки линии

		if (Transform.direction > -Transform.dy)
		{
			Transform.direction -= Transform.dy; /// изменение направлени€ по y
			Transform.x0 += Transform.signX; /// применени€ сдвига пиксел€ на x
		}
		if (Transform.direction < Transform.dx)
		{
			Transform.direction += Transform.dx; /// изменение направлени€ по x
			Transform.y0 += Transform.signY; /// применени€ сдвига пиксел€ на y
		}

		if (Transform.x0 == Transform.x1 || Transform.y0 == Transform.y1) /// выход из цикла
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


//обновление приложени€
void UpdateApp()
{
	//размер итоговой фигуры квадрата
	Transform.width  = 500;
	Transform.height = 500;
	Transform.depth  = 0;

	//центр экрана
	Transform.CenterX = window.width  / 2;
	Transform.CenterY = window.height / 2;

	//определ€ем положени€ вершин в декартовой системе
	int Vector3[4][3] = {
		{-1,	-1 ,   0},
		{-1,	 1 ,   0},
		{ 1,	 1 ,   0},
		{ 1,	-1 ,   0},
	};

	//определ€ем начало и конец рисовани€ вершин
	int Index[4][2] = {
		{1,2},
		{2,3},
		{3,4},
		{4,1}
	};

	//цикл открисовки по размеру количества наших индексов
	for (int i = 0; i < sizeof(Index) / sizeof(Index[0]); i++)
	{
		//начало рисовани€ линии
		Transform.firstX = Vector3[Index[i][0]-1][0] * Transform.width  / 2;
		Transform.firstY = Vector3[Index[i][0]-1][1] * Transform.height / 2;
		Transform.firstZ = Vector3[Index[i][0]-1][2] * Transform.depth  / 2;

		//конец рисовани€ линии
		Transform.secondX = Vector3[Index[i][1]-1][0] * Transform.width  / 2;
		Transform.secondY = Vector3[Index[i][1]-1][1] * Transform.height / 2;
		Transform.secondZ = Vector3[Index[i][1]-1][2] * Transform.depth  / 2;

		//вычисл€ем дельту между вершинами
		Transform.dx = Transform.secondX - Transform.firstX;
		Transform.dy = Transform.secondY - Transform.firstY;
		Transform.dz = Transform.secondZ - Transform.firstZ;

		//определение длины гипотенузы по катитам x, y, z по теореме пифагора
		int length = sqrt(pow(Transform.dx, 2) + pow(Transform.dy, 2) + pow(Transform.dz, 2));

		for (int j = 0; j < length; j++)
		{
			//¬ычислени€ шага отрисовки пикселей при помощи алгоритма Ѕрезенхэма
			int PixelPointX = Transform.dx * j / length + Transform.firstX + Transform.CenterX;
			int PixelPointY = Transform.dy * j / length + Transform.firstY + Transform.CenterY;

			//отрисовка пикселей на экране окна
			SetPixel(window.contx, PixelPointX, PixelPointY, RGB(255, 0, 0));
		}
	}
}

//обработка команд устройств ввода
void UpdateKeyCode()
{
	//выход из приложени€ на ESC
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		window.msg.message = WM_QUIT;
	}
}

//обновление изображений
void UpdateImage()
{
	BitBlt(window.dev_cont, 0, 0, window.width, window.height, window.contx, 0, 0, SRCCOPY);
	//отрисовка заднего фона
	ShowBitmap(window.contx, 0, 0, window.width, window.height, (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
}

//вход в программу
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	InitWindow();
	InitApp();

	//основной цикл обновлени€ приложени€
	while (window.gbool)
	{
		//обработка соощений дл€ окна
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

		//задержка обновлени€
		Sleep(16);
	}
	return 0;
}