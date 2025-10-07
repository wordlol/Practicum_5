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
	float BoxLeftX, BoxLeftY, BoxRightX, BoxRightY;
	int AX, AY, BX, BY, CX, CY;
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

	int Poligon[12][5] =
	{
		////front //red
		{1,2,3 ,1},
		{1,3,4 ,1},

		//back //green
		{5,6,7 ,2},
		{5,7,8 ,2},

		//left //blue
		{1,5,6 ,3},
		{1,6,2 ,3},

		//right //red green
		{4,8,7 ,4},
		{4,7,3 ,4},

		//top //red blue
		{2,6,7 ,5},
		{2,7,3 ,5},

		//bottom //green blue
		{1,5,8 ,6}, 
		{1,8,4 ,6},
	};

	int Color[1][3] =
	{
		{0,0,0},
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
		Transform.Vertex[i][0] *= cameraDist / (Transform.Vertex[i][2] + cameraDist);
		Transform.Vertex[i][1] *= cameraDist / (Transform.Vertex[i][2] + cameraDist);
	}
}

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
												
	Transform.BX = Transform.Vertex[Transform.Poligon[NumPoligon][1] - 1][0] * (Transform.sizeSquare / 2) + Transform.CenterX;
	Transform.BY = Transform.Vertex[Transform.Poligon[NumPoligon][1] - 1][1] * (Transform.sizeSquare / 2) + Transform.CenterY;
												
	Transform.CX = Transform.Vertex[Transform.Poligon[NumPoligon][2] - 1][0] * (Transform.sizeSquare / 2) + Transform.CenterX;
	Transform.CY = Transform.Vertex[Transform.Poligon[NumPoligon][2] - 1][1] * (Transform.sizeSquare / 2) + Transform.CenterY;

	FindColor(Transform.Poligon[NumPoligon][3]);
}

/** Найти описывающий прямоугольник
*/
void FindBoundBox()
{
	int temp;
	temp = max(Transform.AX, Transform.CX);
	Transform.BoxRightX = max(temp, Transform.BX);

	temp = min(Transform.AX, Transform.CX);
	Transform.BoxLeftX = min(temp, Transform.BX);

	temp = max(Transform.AY, Transform.CY);
	Transform.BoxRightY = max(temp, Transform.BY);

	temp = min(Transform.AY, Transform.CY);
	Transform.BoxLeftY = min(temp, Transform.BY);
}

/** Уровнение прямой проходящее через 2 точки на плоскости
*/
int FindPoint(int x0,int x1,int y0,int y1 ,int px, int py)
{
	return (x0 - px) * (y1 - y0) - (x1 - x0) * (y0 - py);
}

/** Проверка является ли точка в треугольнике
*/
bool InTriangle(int PointX, int PointY)
{
	int a = FindPoint(Transform.AX, Transform.BX, Transform.AY, Transform.BY,PointX, PointY);
	int b = FindPoint(Transform.BX, Transform.CX, Transform.BY, Transform.CY, PointX, PointY);
	int c = FindPoint(Transform.CX, Transform.AX, Transform.CY, Transform.AY, PointX, PointY);

	if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0))
		return true;
	else
		return false;
}

/** Функция закрашивания полигона одним цветом
*/
void Rasterisation()
{
	for (int i = 0; i < sizeof(Transform.Poligon)/sizeof(Transform.Poligon[0]); i++)
	{
		InitPointTriangle(i);
		FindBoundBox();

		for (int y = Transform.BoxLeftY; y < Transform.BoxRightY; y++)
		{
			for (int x = Transform.BoxLeftX; x < Transform.BoxRightX; x++)
			{
				if(InTriangle(x, y))
				SetPixel(window.contx, x , y , RGB(Transform.Color[0][0], Transform.Color[0][1], Transform.Color[0][2]));
			}
		}
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
	InitCameraPercpective(40);   /// базовая перспектива  
}

/** Обновление приложения
*/
void UpdateApp()
{
	InitAngleTransform(2, 2, 2); /// поворот за тик (можно использовать без таймера)
	Rasterisation();
	DrawSquare(100);
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


////обновление приложения
//void UpdateApp()
//{  
// ************** алгоритм художника
//	//вычисление среднего z для z-buffer
//	for (int i = 0; i < sizeof(Poligon) / sizeof(Poligon[0]); i++)
//	{
//		float Z1 = Vector3[(int)Poligon[i][0] - 1][2];
//		float Z2 = Vector3[(int)Poligon[i][1] - 1][2];
//		float Z3 = Vector3[(int)Poligon[i][2] - 1][2];
//
//		float Zmidle = (float)(Z1 + Z2 + Z3) / (float)3;
//		Poligon[i][4] = Zmidle;
//	}
// ************** пузырек
//	//пузырьковая сортировка полигонов z-buffer
//	bool sort = false;
//	while (!sort)
//	{
//		sort = true;
//		for (int i = 1; i < sizeof(Poligon) / sizeof(Poligon[0]); i++)
//		{
//			float prev = Poligon[i - 1][4];
//			float current = Poligon[i][4];
//
//			if (prev > current)
//			{
//				float temp[5];
//				for (int j = 0; j < sizeof(Poligon[0]) / sizeof(Poligon[0][0]); j++)
//				{
//					temp[j] = Poligon[i - 1][j];
//					Poligon[i - 1][j] = Poligon[i][j];
//					Poligon[i][j] = temp[j];
//					sort = false;
//				}
//			}
//		}
//	}
// 
//void DrawTriangle(float X1, float X2, float X3, float Y1, float Y2, float Y3, int Color)
//{
//	int R = 0;
//	int G = 0;
//	int B = 0;
//	//выборка цвета
//	if (Color == 1)
//	{
//		R = 255; G = 0; B = 0;
//	}
//	if (Color == 2)
//	{
//		R = 0; G = 255; B = 0;
//	}
//	if (Color == 3)
//	{
//		R = 0; G = 0; B = 255;
//	}
//	if (Color == 4)
//	{
//		R = 255; G = 255; B = 0;
//	}
//	if (Color == 5)
//	{
//		R = 255; G = 0; B = 255;
//	}
//	if (Color == 6)
//	{
//		R = 0; G = 255; B = 255;
//	}
//
//	int AY = Y1 * Transform.height / 2 + Transform.CenterY;
//	int BY = Y2 * Transform.height / 2 + Transform.CenterY;
//	int CY = Y3 * Transform.height / 2 + Transform.CenterY;
//
//	int AX = X1 * Transform.width / 2 + Transform.CenterX;
//	int BX = X2 * Transform.width / 2 + Transform.CenterX;
//	int CX = X3 * Transform.width / 2 + Transform.CenterX;
//
//	int temp;
//	//сортировка 3 вершин по minY и maxY
//	if (AY > BY)
//	{
//		temp = AY;
//		AY = BY;
//		BY = temp;
//	}
//	if (AY > CY)
//	{
//		temp = AY;
//		AY = CY;
//		CY = temp;
//	}
//	if (BY > CY)
//	{
//		temp = BY;
//		BY = CY;
//		CY = temp;
//	}
//
//	//алгоритм заливки триугольника однотонным цветом
//	for (int i = AY; i < CY; i++)
//	{
//		int x1 = AX + (i - AY) * (CX - AX) / (CY - AY);
//		int x2;
//		if (i < BY)
//		{
//			x2 = AX + (i - AY) * (BX - AX) / (BY - AY);
//		}
//		else
//		{
//			if (CY == BY)
//			{
//				x2 = BX;
//			}
//			else
//			{
//				x2 = BX + (i - BY) * (CX - BX) / (CY - BY);
//			}
//		}
//		if (x1 > x2)
//		{
//			temp = x1;
//			x1 = x2;
//			x2 = temp;
//		}
//		for (int j = x1; j < x2; j++)
//		{
//			SetPixel(window.contx, j, i, RGB(R, G, B));
//		}
//	}
//}