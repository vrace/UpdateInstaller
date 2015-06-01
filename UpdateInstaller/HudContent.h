#pragma once

#include <Windows.h>

struct HudMenu
{
	int count;
	int sel;
	const TCHAR *text[5];
};

struct HudText
{
	int count;
	const TCHAR *text[5];
};

struct LaunchUpdateParams
{
	int updateId;
	TCHAR filename[MAX_PATH];
};

class HudContent
{
	HudMenu *menu_;
	HudText *text_;
	const TCHAR *title_;
	const TCHAR *prompt_;
	TCHAR input_[10];
	int state_;
	LaunchUpdateParams updateParams_;
	HWND hwnd_;

public:

	HudContent();

	const HudMenu* menu() const;
	const HudText* text() const;
	const TCHAR* title() const;
	const TCHAR* prompt() const;
	const TCHAR* input() const;

	void process(TCHAR ch);
	BOOL shoudExit() const;
	void setHost(HWND hwnd);

private:

	void setMainMenu();
	void setFailed();
	static void updateThread(void *param);
};
