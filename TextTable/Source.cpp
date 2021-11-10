#include <windows.h>
#include <wingdi.h>
#include <vector>
#include <string>
#include <fstream>
#include <regex>

using namespace std;

#define WIDTH 800
#define HEIGHT 325

int col;
int row;
vector<vector<string>> textMatrix{};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DrawVBorder(HDC hdc, int cWidth, int cHeight, int wndHeight);
void DrawHBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, RECT win);
int readFile();


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (readFile()) {
		const wchar_t* const WND_CREATE_ERROR = L"Cannot read file";
		MessageBoxW(NULL, WND_CREATE_ERROR, NULL, MB_OK);
		return 0;
	}

	MSG msg;
	HWND hWnd;
	WNDCLASSEX wc{ sizeof(WNDCLASSEX) };

	LPCWSTR const className = L"MyAppClass";
	LPCWSTR windowName = L"Text Table";

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;
	wc.style = CS_DBLCLKS;

	if (!RegisterClassEx(&wc)) {
		return EXIT_FAILURE;
	}

	hWnd = CreateWindowEx(
		0, className,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WIDTH, HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) {
		const wchar_t* const WND_CREATE_ERROR = L"Cannot create window";
		MessageBoxW(NULL, WND_CREATE_ERROR, NULL, MB_OK);
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {

	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		TEXTMETRIC textMetric;
		GetTextMetrics(hdc, &textMetric);
		int cellHeight = 2 * textMetric.tmHeight;
		RECT windowRect;

		GetClientRect(hWnd, &windowRect);
		int cellWidth = (windowRect.right - windowRect.left) / col;
		int totalHeight = cellHeight / 2;
		for (int i = 0; i < row; i++) {
			int maxRowHeight = cellHeight;
			for (int j = 0; j < col; ++j) {
				RECT cell;
				cell.left = j * cellWidth;
				cell.right = cell.left + cellWidth;
				cell.top = totalHeight;
				cell.bottom = windowRect.bottom;
				wstring line = wstring(textMatrix[i][j].begin(), textMatrix[i][j].end());
				int curCellHeight = DrawText(hdc, line.c_str(),
					-1, &cell, DT_WORDBREAK | DT_CENTER | DT_END_ELLIPSIS);
				maxRowHeight = curCellHeight > maxRowHeight ? curCellHeight : maxRowHeight;
			}

			DrawHBorder(hdc, cellWidth, cellHeight, totalHeight, windowRect);

			totalHeight += maxRowHeight + cellHeight / 2;
		}

		DrawHBorder(hdc, cellWidth, cellHeight, totalHeight, windowRect);

		DrawVBorder(hdc, cellWidth, cellHeight, totalHeight);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZE: {
		InvalidateRect(hWnd, nullptr, TRUE);
	}
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void DrawVBorder(HDC hdc, int cWidth, int cHeight, int wndHeight) {
	for (int i = 0; i <= col; i++) {
		MoveToEx(hdc, cWidth * i, 0, nullptr);
		LineTo(hdc, cWidth * i, wndHeight - cHeight / 2);
	}
}

void DrawHBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, RECT win) {
	MoveToEx(hdc, 0, wndHeight - cHeight / 2, nullptr);
	LineTo(hdc, win.right, wndHeight - cHeight / 2);
}

int readFile() {
	std::ifstream file("../tableData.txt");
	if (!file) {
		return -1;
	}
	string line;
	while (std::getline(file, line)) {
		std::regex regex{ R"([,][\s]+)" };
		std::sregex_token_iterator iterator{ line.begin(), line.end(), regex, -1 };
		textMatrix.emplace_back(vector<string>{iterator, {}});
	}
	row = textMatrix.size();
	col = textMatrix[0].size();

	file.close();
	return 0;
}
