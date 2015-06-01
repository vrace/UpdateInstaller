#include <Windows.h>

#include "SupportDir.h"
#include "HudContent.h"
#include "AudioDevice.h"
#include "resource.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 640

static const TCHAR AppName[] = TEXT("BMS 4 Update Installer");
static HINSTANCE AppInstance;
static HFONT MainFont;
static int FontHeight;
static const int LineMessage = 100;
static const int VStart = 420;
static HudContent Hud;
static RECT ButtonRect[] =
{
	{ 532, 503, 532 + 24, 503 + 23 },  // 0
	{ 407, 422, 407 + 25, 421 + 24 },  // 1
	{ 449, 422, 449 + 25, 422 + 24 },  // 2
	{ 491, 422, 491 + 25, 422 + 24 },  // 3
	{ 408, 462, 408 + 25, 462 + 24 },  // 4
	{ 449, 462, 449 + 25, 462 + 24 },  // 5
	{ 491, 462, 491 + 25, 462 + 24 },  // 6
	{ 409, 502, 409 + 25, 502 + 24 },  // 7
	{ 450, 502, 450 + 25, 502 + 24 },  // 8
	{ 490, 502, 490 + 25, 502 + 24 },  // 9
	{ 409, 539, 409 + 19, 539 + 17 },  // UP
	{ 409, 556, 409 + 19, 556 + 17 },  // DOWN
	{ 452, 550, 452 + 20, 550 + 14 },  // RTN
	{ 539, 423, 539 + 27, 423 + 25 },  // RCL
	{ 539, 466, 539 + 26, 466 + 25 },  // ENTR
};
static TCHAR InputChar[] = TEXT("0123456789-=/<>");

void InitMainFont(HWND hwnd)
{
	HDC hdc;
	LOGFONT lf = {0};
	RECT rc;
	HFONT lastFont;

	hdc = GetDC(hwnd);
	wsprintf(lf.lfFaceName, TEXT("Courier New"));
	lf.lfWeight = FW_NORMAL;
	lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	MainFont = CreateFontIndirect(&lf);
	lastFont = (HFONT)SelectObject(hdc, MainFont);
	DrawText(hdc, TEXT("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG"), -1, &rc, DT_SINGLELINE | DT_CALCRECT);
	FontHeight = rc.bottom - rc.top;
	SelectObject(hdc, lastFont);

	ReleaseDC(hwnd, hdc);
}

void PaintHud(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;
	HFONT lastFont;
	const HudMenu *menu;
	const HudText *text;
	TCHAR prompt[100];

	hdc = BeginPaint(hwnd, &ps);
	GetClientRect(hwnd, &rc);

	lastFont = (HFONT)SelectObject(hdc, MainFont);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(145, 250, 70));

	/* draw title */

	rc.top += LineMessage;
	DrawText(hdc, Hud.title(), -1, &rc, DT_SINGLELINE | DT_CENTER);

	/* draw our menu if any */

	rc.top = LineMessage + FontHeight * 3;
	rc.left = VStart;

	menu = Hud.menu();
	if (menu)
	{
		TCHAR msg[100];

		for (int i = 0; i < menu->count && menu->text[i]; ++i)
		{
			wsprintf(msg, TEXT("%s%s"), i == menu->sel ? TEXT(">   ") : TEXT("    "), menu->text[i]);
			DrawText(hdc, msg, -1, &rc, DT_SINGLELINE | DT_LEFT);
			rc.top += FontHeight;
		}
	}

	/* draw text if any */

	rc.top = LineMessage + FontHeight * 3;
	rc.left = 0;

	text = Hud.text();
	if (text)
	{
		for (int i = 0; i < text->count && text->text[i]; ++i)
		{
			DrawText(hdc, text->text[i], -1, &rc, DT_SINGLELINE | DT_CENTER);
			rc.top += FontHeight;
		}
	}

	/* draw prompt if any */

	rc.top = LineMessage + FontHeight * 10;
	rc.left = 0;
	wsprintf(prompt, TEXT("%s %s"), Hud.prompt(), Hud.input());
	DrawText(hdc, prompt, -1, &rc, DT_SINGLELINE | DT_CENTER);

	SelectObject(hdc, lastFont);

	EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
		InitMainFont(hwnd);
		return 0;

	case WM_PAINT:
		PaintHud(hwnd);
		return 0;

	case WM_LBUTTONDOWN:
		{
			POINT pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);

			for (int i = 0; i < lstrlen(InputChar); ++i)
			{
				if (PtInRect(&ButtonRect[i], pt))
				{
					AudioDevice::sharedDevice()->buttonPress();
					Hud.process(InputChar[i]);
					InvalidateRect(hwnd, NULL, TRUE);
				}
			}

			if (Hud.shoudExit())
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
		}
		return 0;

	case WM_DESTROY:
		DeleteObject(MainFont);
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

HWND CreateMain(HINSTANCE hInstance)
{
	RECT rc;
	int sx, sy;
	int cx, cy;
	DWORD style;

	sx = GetSystemMetrics(SM_CXSCREEN);
	sy = GetSystemMetrics(SM_CYSCREEN);

	style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX;
	SetRect(&rc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	AdjustWindowRect(&rc, style, FALSE);

	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top;

	rc.left = (sx - cx) / 2;
	rc.top = (sy - cy) / 2;

	return CreateWindow(AppName, AppName, style, rc.left, rc.top, cx, cy, NULL, NULL, hInstance, NULL);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASS wc;
	HWND hwnd;
	MSG msg;
	HBITMAP bmp;

	AppInstance = hInstance;
	SupportDir::sharedSupportDir();
	AudioDevice::sharedDevice()->music();

	bmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BACKGROUND));

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance     = hInstance;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreatePatternBrush(bmp);
	wc.lpszClassName = AppName;
	wc.lpszMenuName  = NULL;

	RegisterClass(&wc);

	hwnd = CreateMain(hInstance);
	Hud.setHost(hwnd);

	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	AudioDevice::sharedDevice()->release();
	SupportDir::sharedSupportDir()->release();

	return 0;
}
