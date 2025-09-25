#include "Windows.h"
#include "math.h"

//стурктура где храняться данные о windows окне
struct
{
	//дескрипторы, контейнеры и буфферы для windows
	RECT rc;
	HINSTANCE hIns;
	HWND hWnd;
	HDC dev_cont, contx;
	MSG msg;
	BOOL gbool = true;

	//определяет размер экрана в вашей сиситеме
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

//создания windows окна
void InitWindow()
{
	//имя класса окна
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

	//регистрация класса окна
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

//загрузка модулей приложения
void InitApp()
{
	//создание и иниализация контекста устройсва и девайс устройства
	window.dev_cont = GetDC(window.hWnd);
	window.contx = CreateCompatibleDC(window.dev_cont);
	SelectObject(window.contx, CreateCompatibleBitmap(window.dev_cont, window.width, window.height));
}

//обновление приложения
void UpdateApp()
{
	//размер итоговой фигуры квадрата
	Transform.width  = 500;
	Transform.height = 500;
	Transform.depth  = 0;

	//центр экрана
	Transform.CenterX = window.width  / 2;
	Transform.CenterY = window.height / 2;

	//определяем положения вершин в декартовой системе
	int Vector3[4][3] = {
		{-1,	-1 ,   0},
		{-1,	 1 ,   0},
		{ 1,	 1 ,   0},
		{ 1,	-1 ,   0},
	};

	//определяем начало и конец рисования вершин
	int Index[4][2] = {
		{1,2},
		{2,3},
		{3,4},
		{4,1}
	};

	//цикл открисовки по размеру количества наших индексов
	for (int i = 0; i < sizeof(Index) / sizeof(Index[0]); i++)
	{
		//начало рисования линии
		Transform.firstX = Vector3[Index[i][0]-1][0] * Transform.width  / 2;
		Transform.firstY = Vector3[Index[i][0]-1][1] * Transform.height / 2;
		Transform.firstZ = Vector3[Index[i][0]-1][2] * Transform.depth  / 2;

		//конец рисования линии
		Transform.secondX = Vector3[Index[i][1]-1][0] * Transform.width  / 2;
		Transform.secondY = Vector3[Index[i][1]-1][1] * Transform.height / 2;
		Transform.secondZ = Vector3[Index[i][1]-1][2] * Transform.depth  / 2;

		//вычисляем дельту между вершинами
		Transform.dx = Transform.secondX - Transform.firstX;
		Transform.dy = Transform.secondY - Transform.firstY;
		Transform.dz = Transform.secondZ - Transform.firstZ;

		//определение длины гипотенузы по катитам x, y, z по теореме пифагора
		int length = sqrt(pow(Transform.dx, 2) + pow(Transform.dy, 2) + pow(Transform.dz, 2));

		for (int j = 0; j < length; j++)
		{
			//Вычисления шага отрисовки пикселей при помощи алгоритма Брезенхэма
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
	//выход из приложения на ESC
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

	//основной цикл обновления приложения
	while (window.gbool)
	{
		//обработка соощений для окна
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

		//задержка обновления
		Sleep(16);
	}
	return 0;
}