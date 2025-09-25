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
	float dx, dy, dz, CenterX, CenterY;
	float firstX, firstY, firstZ ,secondX, secondY, secondZ;
	int width, height, depth;
	float angleX, angleY, angleZ;
	int timer;
	int cameraDist;
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
	//Таймер
	Transform.timer++;

	//растояние камеры до центра xyz
	Transform.cameraDist = 10;

	//размер итоговой фигуры квадрата
	Transform.width  = 100;
	Transform.height = 100;
	Transform.depth  = 0;

	//центр экрана
	Transform.CenterX = window.width  / 2;
	Transform.CenterY = window.height / 2;

	//угол поворота квадрата
	Transform.angleX = 0 * (3.14 / 180.);
	Transform.angleY = Transform.timer * (3.14 / 180.);
	Transform.angleZ = 0 * (3.14 / 180.);

	//определяем положения вершин в декартовой системе
	float Vector3[8][3] = {
		{-1,	-1,   0},
		{-1,	 1,   0},
		{ 1,	 1,   0},
		{ 1,	-1,   0},

		{-1,	-1,  -2},
		{-1,	 1,  -2},
		{ 1,	 1,  -2},
		{ 1,	-1,  -2},
	};

	//определяем начало и конец рисования вершин
	int Index[6][2] = {
		{1,2}, 
		{2,3}, 
		{3,1}, 

		/*{1,3},
		{3,4},
		{4,1},*/

		/*{5,6}, {5,7},
		{6,7}, {7,8},
		{7,5}, {8,5},

		{1,5}, {1,6},
		{5,6}, {6,2},
		{6,1}, {2,1},

		{2,6}, {2,7},
		{6,7}, {7,3},
		{7,2}, {3,2},

		{3,7}, {3,8},
		{7,8}, {8,4},
		{8,3}, {4,3},

		{4,8}, {4,5},
		{8,5}, {5,1},
		{5,4}, {1,4},*/
	};


	//преобразование вершин по 3 углам поворота
	for (int i = 0; i < sizeof(Vector3)/sizeof(Vector3[0]); i++)
	{
		//переменные для умножения вершин на матрицу поворота
		float x = Vector3[i][0];
		float y = Vector3[i][1];
		float z = Vector3[i][2];

		//преобразование по X
		Vector3[i][1] = y * cos(Transform.angleX) + z * -sin(Transform.angleX);
		Vector3[i][2] = y * sin(Transform.angleX) + z * cos(Transform.angleX);
		
		x = Vector3[i][0];
		z = Vector3[i][2];

		//преобразование по Y
		Vector3[i][0] = x *  cos(Transform.angleY) + z * sin(Transform.angleY);
		Vector3[i][2] = x * -sin(Transform.angleY) + z * cos(Transform.angleY);

		x = Vector3[i][0];
		y = Vector3[i][1];

		//преобразование по Z
		Vector3[i][0] = x * cos(Transform.angleZ) + y * -sin(Transform.angleZ) ;
		Vector3[i][1] = x * sin(Transform.angleZ) + y *  cos(Transform.angleZ) ;

	}

	float printx1 = Vector3[1 - 1][0] / (1 - Vector3[1 - 1][2] / Transform.cameraDist);
	float printx2 = Vector3[2 - 1][0] / (1 - Vector3[2 - 1][2] / Transform.cameraDist);
	float printx3 = Vector3[3 - 1][0] / (1 - Vector3[3 - 1][2] / Transform.cameraDist);

	float printy1 = Vector3[1 - 1][1] / (1 - Vector3[1 - 1][2] / Transform.cameraDist);
	float printy2 = Vector3[2 - 1][1] / (1 - Vector3[2 - 1][2] / Transform.cameraDist);
	float printy3 = Vector3[3 - 1][1] / (1 - Vector3[3 - 1][2] / Transform.cameraDist);

	float Ay; //высокая вершина
	float By; //средняя 
	float Cy; //нижняя

	float Ax;
	float Bx;
	float Cx;


	//цикл открисовки по размеру количества наших индексов
	for (int i = 0; i < sizeof(Index) / sizeof(Index[0]); i++)
	{
		//перспективное преобразование вершин квадрата
		float firstPerX  = Vector3[Index[i][0] - 1][0] / (1 - Vector3[Index[i][0] - 1][2] / Transform.cameraDist);
		float firstPerY  = Vector3[Index[i][0] - 1][1] / (1 - Vector3[Index[i][0] - 1][2] / Transform.cameraDist);
		float firstPerZ  = Vector3[Index[i][0] - 1][2] / (1 - Vector3[Index[i][0] - 1][2] / Transform.cameraDist);

		float secondPerX = Vector3[Index[i][1] - 1][0] / (1 - Vector3[Index[i][1] - 1][2] / Transform.cameraDist);
		float secondPerY = Vector3[Index[i][1] - 1][1] / (1 - Vector3[Index[i][1] - 1][2] / Transform.cameraDist);
		float secondPerZ = Vector3[Index[i][1] - 1][2] / (1 - Vector3[Index[i][1] - 1][2] / Transform.cameraDist);

		//начало рисования линии
		Transform.firstX = firstPerX * Transform.width  / 2;
		Transform.firstY = firstPerY * Transform.height / 2;
		Transform.firstZ = firstPerZ * Transform.depth  / 2;

		//конец рисования линии
		Transform.secondX = secondPerX * Transform.width  / 2;
		Transform.secondY = secondPerY * Transform.height / 2;
		Transform.secondZ = secondPerZ * Transform.depth  / 2;

		//вычисляем дельту между вершинами
		Transform.dx = Transform.secondX - Transform.firstX;
		Transform.dy = Transform.secondY - Transform.firstY;
		Transform.dz = Transform.secondZ - Transform.firstZ;

		//заливка цветом

		//сортировка 3 вершин
		if (printy1 >= printy2)
		{
			if (printy1 >= printy3)
			{
				Ay = printy2;
				Ax = printx2;

				By = printy3;
				Bx = printx3;

				Cy = printy1;
				Cx = printx1;
			}
			else
			{
				Ay = printy2;
				Ax = printx2;

				By = printy1;
				Bx = printx1;

				Cy = printy3;
				Cx = printx3;
			}
		}
		else
		{
			if (printy2 >= printy3)
			{
				Ay = printy1;
				Ax = printx1;

				By = printy3;
				Bx = printx3;

				Cy = printy2;
				Cx = printx2;
			}
			else
			{
				Ay = printy1;
				Ax = printx1;

				By = printy2;
				Bx = printx2;

				Cy = printy3;
				Cx = printx3;
			}
		}

		Ax *= Transform.width / 2;
		Ay *= Transform.height / 2;

		Bx *= Transform.width / 2;
		By *= Transform.height / 2;

		Cx *= Transform.width / 2;
		Cy *= Transform.height / 2;




		/*for (int sy = Ay; sy <= Cy; sy++) 
		{
			int x1 = Ax + (sy - Ay) * (Cx - Ax) / (Cy - Ay);
			int x2;

			if (sy < By)
			{
				x2 = Ax + (sy - Ay) * (Bx - Ax) / (By - Ay);
			}
			else 
			{
				if (Cy == By)
				x2 = Bx;
				else
				x2 = Bx + (sy - By) * (Cx - Bx) / (Cy - By);
			}
			if (x1 > x2) 
			{ 
				int tmp = x1; 
				x1 = x2; 
				x2 = tmp;
			};

			int dx12 = x2 - x1;
			int length2 = sqrt(pow(dx12, 2) + pow(Transform.dy, 2) + pow(Transform.dz, 2));
			for (int i = 0; i < length2; i++)
			{
				int PixelPointX = dx12 * i / length2 + Transform.firstX + Transform.CenterX;
				int PixelPointY = Transform.dy * i / length2 + Transform.firstY + Transform.CenterY;
				SetPixel(window.contx, PixelPointX, PixelPointY, RGB(255, 0, 0));
			}
		}*/

		//определение длины гипотенузы по катитам x, y, z по теореме пифагора
		int length = sqrt(pow(Transform.dx, 2) + pow(Transform.dy, 2) + pow(Transform.dz, 2));

		for (int j = 0; j < length; j++)
		{
			//Вычисления шага отрисовки пикселей при помощи алгоритма Брезенхэма
			int PixelPointX = Transform.dx * j / length + Transform.firstX + Transform.CenterX;
			int PixelPointY = Transform.dy * j / length + Transform.firstY + Transform.CenterY;

			//отрисовка пикселей на экране окна
			SetPixel(window.contx, PixelPointX, PixelPointY, RGB(255, 255, 255));
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
		Sleep(3);
	}
	return 0;
}