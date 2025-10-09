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
	float AX, AY, BX, BY, CX, CY, AZ, BZ, CZ;
	int cameraDist;
	int asix   = 0;
	int asiy   = 0;
	int tangag = 0;

	int DefultVertexBuffer[8][3] =
	{
				{-1,	-1 ,   1},
				{-1,	 1 ,   1},
				{ 1,	 1 ,   1},
				{ 1,	-1 ,   1},

				{-1,	-1 ,  -1},
				{-1,	 1 ,  -1},
				{ 1,	 1 ,  -1},
				{ 1,	-1 ,  -1},
	};

	/// точки вертексов квадрата
	float Vertex[8][3] =
	{
				{-1,	-1 ,   1},
				{-1,	 1 ,   1},
				{ 1,	 1 ,   1},
				{ 1,	-1 ,   1},

				{-1,	-1 ,  -1},
				{-1,	 1 ,  -1},
				{ 1,	 1 ,  -1},
				{ 1,	-1 ,  -1},
	};

	/// последовательность отрисовки квадрата
	int Index[36][2] =
	{
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

	/// массив полигонов
	int Poligon[12][4] =
	{
		//front //red
		{1,2,3 ,1},
		{1,4,3 ,1},

		//back //green
		{5,6,7 ,2},
		{5,8,7 ,2},

		//left //blue
		{1,5,6 ,3},
		{1,2,6 ,3},

		//right //red green
		{4,8,7 ,4},
		{4,3,7 ,4},

		//bottom //green blue
		{2,6,7 ,5},
		{2,3,7 ,5},

		//top //red blue
		{1,5,8 ,6},
		{1,4,8 ,6},
	};

	/// массив цветов
	int Color[1][3] =
	{
		{0,0,0},
	};

	/// zBuffer для всего окна
	float ZBuffer[3000][3000];

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
	Transform.cameraDist = cameraDist;

	for (int i = 0; i < sizeof(Transform.Vertex) / sizeof(Transform.Vertex[0]); i++)
	{
		Transform.Vertex[i][0] *= Transform.cameraDist / (Transform.Vertex[i][2] + Transform.cameraDist);
		Transform.Vertex[i][1] *= Transform.cameraDist / (Transform.Vertex[i][2] + Transform.cameraDist);
	}
}

void Swap(int& value1, int& value2)
{
	int temp = value1;
	value1 = value2;
	value2 = temp;
};


/** Функция отрисвоки линии на экране окна по Брезенхэйму
*/
void DrawLine()
{
	bool StatusX = false;
	bool StatusY = false;

	while (true)
	{
		SetPixel(window.contx, Transform.x0, Transform.y0, RGB(255, 255, 255)); /// отрисовки линии

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

		/// проверка на завершение отрисовки линии
		if (Transform.x0 == Transform.x1)
			StatusX = true;
		if (Transform.y0 == Transform.y1)
			StatusY = true;

		if (StatusX && StatusY)
			break;
	}
}

/** Алгоритм отрисовки квадрата
*/
void DrawSquare(int size, bool drawlines)
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
		if (drawlines == true)
		{
			DrawLine();
		}
	}
}

/** Поиск цвета
*/
void FindColor(int NumColor)
{
	if (NumColor == 1)
	{
		Transform.Color[0][0] = 255;
		Transform.Color[0][1] = 0;
		Transform.Color[0][2] = 0;
	}
	if (NumColor == 2)
	{
		Transform.Color[0][0] = 0;
		Transform.Color[0][1] = 255;
		Transform.Color[0][2] = 0;
	}
	if (NumColor == 3)
	{
		Transform.Color[0][0] = 0;
		Transform.Color[0][1] = 0;
		Transform.Color[0][2] = 255;
	}
	if (NumColor == 4)
	{
		Transform.Color[0][0] = 255;
		Transform.Color[0][1] = 255;
		Transform.Color[0][2] = 0;
	}
	if (NumColor == 5)
	{
		Transform.Color[0][0] = 255;
		Transform.Color[0][1] = 0;
		Transform.Color[0][2] = 255;
	}
	if (NumColor == 6)
	{
		Transform.Color[0][0] = 0;
		Transform.Color[0][1] = 255;
		Transform.Color[0][2] = 255;
	}
}

/** Получаем данные полигона
*/
void InitPointTriangle(int NumPoligon)
{
	Transform.AX = Transform.Vertex[Transform.Poligon[NumPoligon][0] - 1][0] * (Transform.sizeSquare / 2) + Transform.CenterX;
	Transform.AY = Transform.Vertex[Transform.Poligon[NumPoligon][0] - 1][1] * (Transform.sizeSquare / 2) + Transform.CenterY;
	Transform.AZ = Transform.Vertex[Transform.Poligon[NumPoligon][0] - 1][2] * (Transform.sizeSquare / 2) + Transform.cameraDist;
	
	Transform.BX = Transform.Vertex[Transform.Poligon[NumPoligon][1] - 1][0] * (Transform.sizeSquare / 2) + Transform.CenterX;
	Transform.BY = Transform.Vertex[Transform.Poligon[NumPoligon][1] - 1][1] * (Transform.sizeSquare / 2) + Transform.CenterY;
	Transform.BZ = Transform.Vertex[Transform.Poligon[NumPoligon][1] - 1][2] * (Transform.sizeSquare / 2) + Transform.cameraDist;
	
	Transform.CX = Transform.Vertex[Transform.Poligon[NumPoligon][2] - 1][0] * (Transform.sizeSquare / 2) + Transform.CenterX;
	Transform.CY = Transform.Vertex[Transform.Poligon[NumPoligon][2] - 1][1] * (Transform.sizeSquare / 2) + Transform.CenterY;
	Transform.CZ = Transform.Vertex[Transform.Poligon[NumPoligon][2] - 1][2] * (Transform.sizeSquare / 2) + Transform.cameraDist;
}

/** Интерполяция Z координаты (билинейная)
*/
float FindZPoint(float vertices[3][3], float x, float y)
{
		float x1 = vertices[0][0], x2 = vertices[1][0], x3 = vertices[2][0];
		float y1 = vertices[0][1], y2 = vertices[1][1], y3 = vertices[2][1];
		float z1 = vertices[0][2], z2 = vertices[1][2], z3 = vertices[2][2];

		//барицентрические координаты
		float bar = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);

		float w1 = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / bar;
		float w2 = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / bar;
		float w3 = 1 - w1 - w2;

		/// интерполяция Z
		float z = w1 * z1 + w2 * z2 + w3 * z3;

		/// проверка то то находится ли точка внутри триугольника
		if (0 <= w1 <= 1 && 0 <= w2 <= 1 && 0 <= w3 <= 1)
		return z;
}

/** Рисуем пиксели с учетом zbuffer
*/
void DrawPixel(int x, int y, float z)
{
	if (x > 0 && y > 0 && x <= window.width && y <= window.height)
	{
		if (z < Transform.ZBuffer[x][y]) {
			Transform.ZBuffer[x][y] = z;
			SetPixel(window.contx, x, y, RGB(Transform.Color[0][0], Transform.Color[0][1], Transform.Color[0][2]));
		}
	}
}

/** Метод для замены переменных местами
*/
void Swap(float& value1, float& value2)
{
	float temp = value1;
	value1 = value2;
	value2 = temp;
}

/** Алгоритм поиска точкек x1 и x2
*/
void FindXinterpolation(float vertices[3][3], int y ,int &x1,int &x2)
{
	x1 = vertices[0][0] + (y - vertices[0][1]) * (vertices[2][0] - vertices[0][0]) / (vertices[2][1] - vertices[0][1]);
	if (y < (int)vertices[1][1])
	{
		x2 = vertices[0][0] + (y - vertices[0][1]) * (vertices[1][0] - vertices[0][0]) / (vertices[1][1] - vertices[0][1]);
	}
	else
	{
		if ((int)vertices[2][1] == (int)vertices[1][1])
			x2 = vertices[1][0];
		else
			x2 = vertices[1][0] + (y - vertices[1][1]) * (vertices[2][0] - vertices[1][0]) / (vertices[2][1] - vertices[1][1]);
	}

	if (x1 > x2)
		Swap(x1, x2);
}

/** Функция которая загружает данные пиксилей в ZBuffer
*/
void Rasterization()
{
	for (int i = 0; i < sizeof(Transform.Poligon) / sizeof(Transform.Poligon[0]); i++)
	{
		InitPointTriangle(i);
		FindColor(Transform.Poligon[i][3]);

		// вершины
		float vertices[3][3] = {
			{Transform.AX, Transform.AY, Transform.AZ},
			{Transform.BX, Transform.BY, Transform.BZ},
			{Transform.CX, Transform.CY, Transform.CZ}
		};

		// сортируем вершины
		for (int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) - 1; i++)
		{
			for (int j = sizeof(vertices) / sizeof(vertices[0]) - 1; j > i; j--)
			{
				if (vertices[j - 1][1] > vertices[j][1])
				{
					Swap(vertices[j - 1][0], vertices[j][0]);
					Swap(vertices[j - 1][1], vertices[j][1]);
					Swap(vertices[j - 1][2], vertices[j][2]);
				}
			}
		}

		int x1;
		int x2;
		if (vertices[0][1] != vertices[2][1])
		{
			for (int y = vertices[0][1]; y <= vertices[2][1]; y++)
			{

				FindXinterpolation(vertices, y, x1, x2); /// ищем точки x1, x2

				for (int x = x1; x <= x2; x++)
				{
					float z = FindZPoint(vertices,x,y);

					DrawPixel(x, y, z); /// рисуем точку с учетом глибны (Zbuffer)
				}
			}
		}
	}
}

/** очищает zbuffer и ZBufferColor до стандартный значений
*/
void ClearZBuffer()
{
	for (int i = 0; i < sizeof(Transform.ZBuffer) / sizeof(Transform.ZBuffer[0]); i++)
	{
		for (int j = 0; j < sizeof(Transform.ZBuffer[0]) / sizeof(Transform.ZBuffer[0][0]); j++)
		{
			Transform.ZBuffer[i][j] = 1000;
		}
	}
}

/** очищает Vertex buffer до стандартный значений
*/
void ClearVertexBuffer()
{
	for (int i = 0; i < sizeof(Transform.Vertex) / sizeof(Transform.Vertex[0]); i++)
	{
		Transform.Vertex[i][0] = Transform.DefultVertexBuffer[i][0];
		Transform.Vertex[i][1] = Transform.DefultVertexBuffer[i][1];
		Transform.Vertex[i][2] = Transform.DefultVertexBuffer[i][2];
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
}

void Information(float valueX,float valueY, float valueZ)
{
	//SetBkMode(window.context, TRANSPARENT); //аааааааааааа
	auto hFont = CreateFont(30, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI"); //ааааа
	auto hTmp = (HFONT)SelectObject(window.contx, hFont);

	char txt[32];
	SetTextColor(window.contx, RGB(255, 0, 0));
	_itoa_s(valueX, txt, 10); // аа аааа ааааааааа аааааа
	TextOutA(window.contx, window.width - 500, window.height - 1000, (LPCSTR)txt, strlen(txt));// аааа ааааа аааааа

	SetTextColor(window.contx, RGB(0, 255, 50));
	_itoa_s(valueY, txt, 10); // аа аааа ааааааааа аааааа
	TextOutA(window.contx, window.width - 450, window.height - 1000, (LPCSTR)txt, strlen(txt));// аааа ааааа аааааа

	SetTextColor(window.contx, RGB(0, 0, 255));
	_itoa_s(valueZ, txt, 10); // аа аааа ааааааааа аааааа
	TextOutA(window.contx, window.width - 400, window.height - 1000, (LPCSTR)txt, strlen(txt));// аааа ааааа аааааа
}
/** Обновление приложения
*/
void UpdateApp()
{
	ClearVertexBuffer();
	ClearZBuffer();
	InitAngleTransform(Transform.asiy, Transform.asix, Transform.tangag); /// поворот за тик
	InitCameraPercpective(40);   /// перспектива  
	Rasterization();
	DrawSquare(100, false);


	Information(Transform.AX, Transform.AY, Transform.AZ);

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

	// управление кубом
	if (GetAsyncKeyState(VK_LEFT))
	{
		Transform.asix += 5;
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		Transform.asix -= 5;
	}
	if (GetAsyncKeyState(VK_UP))
	{
		Transform.asiy -= 5;
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		Transform.asiy += 5;
	}
	if (GetAsyncKeyState(0x51))
	{
		Transform.tangag -= 1;
	}
	if (GetAsyncKeyState(0x45))
	{
		Transform.tangag += 1;
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