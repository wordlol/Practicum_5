#include "Windows.h"
#include "math.h"

#define PI 3.14
#define RAD PI / 180

/** стурктура где храняться данные о windows окне
*/
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

/** структура с данными о координатах
*/
struct
{
	int x0, y0, x1, y1;
	int	dx, dy;
	int	signX, signY;
	int	direction;
	int	CenterX = window.width / 2;
	int	CenterY = window.height / 2;
	float angleX, angleY, angleZ;
	int sizeSquare;
	/// точки вертексов квадрата
	float Vertex[4][3] =
	{
				{-1,	-1 ,   0},
				{-1,	 1 ,   1},
				{ 1,	 1 ,   0},
				{ 1,	-1 ,   0},
	};

	/// последовательность отрисовки квадрата
	int Index[4][2] =
	{
				{1,2},
				{2,3},
				{3,4},
				{4,1}
	};
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

/** создания windows окна
*/
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

/** отрисовка изображений .bmp
*/
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

/** Функция для загрузки данных для построения линии
*/
void InitTransformData(int x0, int y0, int x1, int y1, int size)
{
	Transform.sizeSquare = size;
	/// выстовляем квадрат в центр экрана и так же задаем ширину и высоту
	Transform.x0 = Transform.CenterX + x0;
	Transform.y0 = Transform.CenterY + y0;
	Transform.x1 = Transform.CenterX + x1;
	Transform.y1 = Transform.CenterY + y1;

	/// вычисляем дельты
	Transform.dx = abs(Transform.x1 - Transform.x0);
	Transform.dy = abs(Transform.y1 - Transform.y0);

	/// опеределяем начало отрисовки
	if (Transform.x0 < Transform.x1)
		Transform.signX = 1;
	else
		Transform.signX = -1;

	if (Transform.y0 < Transform.y1)
		Transform.signY = 1;
	else
		Transform.signY = -1;

	/// опеределяем направление рисования
	Transform.direction = Transform.dx - Transform.dy;
}

/** Функция для применеия преобразования буфера вертексов повороту
*/
void InitAngleTransform(int angleX, int angleY, int angleZ)
{
	Transform.angleX = angleX * RAD;
	Transform.angleY = angleY * RAD;
	Transform.angleZ = angleZ * RAD;

	/// преобразование вершин по 3 углам поворота
	for (int i = 0; i < sizeof(Transform.Vertex) / sizeof(Transform.Vertex[0]); i++)
	{
		/// переменные для умножения вершин на матрицу поворота
		float x = Transform.Vertex[i][0];
		float y = Transform.Vertex[i][1];
		float z = Transform.Vertex[i][2];

		/// преобразование по X
		Transform.Vertex[i][1] = y * cos(Transform.angleX) + z * -sin(Transform.angleX);
		Transform.Vertex[i][2] = y * sin(Transform.angleX) + z * cos(Transform.angleX);

		x = Transform.Vertex[i][0];
		z = Transform.Vertex[i][2];

		/// преобразование по Y
		Transform.Vertex[i][0] = x * cos(Transform.angleY) + z * sin(Transform.angleY);
		Transform.Vertex[i][2] = x * -sin(Transform.angleY) + z * cos(Transform.angleY);

		x = Transform.Vertex[i][0];
		y = Transform.Vertex[i][1];

		/// преобразование по Z
		Transform.Vertex[i][0] = x * cos(Transform.angleZ) + y * -sin(Transform.angleZ);
		Transform.Vertex[i][1] = x * sin(Transform.angleZ) + y * cos(Transform.angleZ);
	}
}

/** Функция преобразует вертексы в перспективное пространство
*/
void InitCameraPercpective(float cameraDist)
{
	for (int i = 0; i < sizeof(Transform.Vertex) / sizeof(Transform.Vertex[0]); i++)
	{
		Transform.Vertex[i][0] = Transform.Vertex[i][0] * cameraDist / (Transform.Vertex[i][2] + cameraDist);
		Transform.Vertex[i][1] = Transform.Vertex[i][1] * cameraDist / (Transform.Vertex[i][2] + cameraDist);
	}
}

/** Функция отрисвоки линии на экране окна по Брезенхэйму
*/
void DrawLine()
{
	for (int i = 0; i < Transform.sizeSquare; i++)
	{
		SetPixel(window.contx, Transform.x0, Transform.y0, RGB(255, 0, 0)); /// отрисовки линии

		if (Transform.direction > -Transform.dy)
		{
			if (Transform.x0 != Transform.x1)
			{
				Transform.direction -= Transform.dy; /// изменение направления по y
				Transform.x0 += Transform.signX; /// применения сдвига пикселя на x
			}
		}
		if (Transform.direction < Transform.dx)
		{
			if (Transform.y0 != Transform.y1)
			{
				Transform.direction += Transform.dx; /// изменение направления по x
				Transform.y0 += Transform.signY; /// применения сдвига пикселя на y
			}
		}

	}
}

/** Алгоритм отрисовки квадрата
*/
void DrawSquare(int size)
{
	for (int i = 0; i < sizeof(Transform.Index) / sizeof(Transform.Index[0]); i++)
	{
		/// получаем индексы из буффера
		int Point0 = Transform.Index[i][0];
		int Point1 = Transform.Index[i][1];

		/// загружаем точки вертексов
		InitTransformData(
			Transform.Vertex[Point0 - 1][0] * size / 2,
			Transform.Vertex[Point0 - 1][1] * size / 2,
			Transform.Vertex[Point1 - 1][0] * size / 2,
			Transform.Vertex[Point1 - 1][1] * size / 2,
			size
		);

		/// отрисовываем грани квадрата по точкам
		DrawLine();
	}
}

/** загрузка модулей приложения
*/
void InitApp()
{
	//создание и иниализация контекста устройсва и девайс устройства
	window.dev_cont = GetDC(window.hWnd);
	window.contx = CreateCompatibleDC(window.dev_cont);
	SelectObject(window.contx, CreateCompatibleBitmap(window.dev_cont, window.width, window.height));

	InitAngleTransform(0, 0, 0); /// базовый поворот
	InitCameraPercpective(4); /// базовая перспектива
}

/** Обновление приложения
*/
void UpdateApp()
{
	InitAngleTransform(0, 0, 0); /// поворот за тик (можно использовать без таймера)
	DrawSquare(500);
}



/** обработка команд устройств ввода
*/
void UpdateKeyCode()
{
	//выход из приложения на ESC
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
	//отрисовка заднего фона
	ShowBitmap(window.contx, 0, 0, window.width, window.height, (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
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

	/// обработка соощений для окна
	while (window.gbool)
	{
		/// отбработка сообщений
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

		/// задержка обновления
		Sleep(16);
	}
	return 0;
}
