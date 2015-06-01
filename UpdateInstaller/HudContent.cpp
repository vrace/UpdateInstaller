#include "HudContent.h"

#include <tchar.h>
#include <io.h>
#include <process.h>

#include "ExtApp.h"
#include "SupportDir.h"
#include "AudioDevice.h"

static HudMenu MainMenu =
{
	3, 0,
	{
		TEXT("INSTALL UPDATE"),
		TEXT("ABOUT INSTALLER"),
		TEXT("EXIT"),
		NULL, NULL
	}
};

static HudText AboutInstaller =
{
	5,
	{
		TEXT("BROUGHT TO YOU BY"),
		TEXT("YANKEE SIERRA AIR BASE"),
		TEXT("YS-122"),
		TEXT(""),
		TEXT("YSAB.AEROFORUMS.COM")
	}
};

static HudText InstallInstruction =
{
	3,
	{
		TEXT(""),
		TEXT("ENTER THE UPDATE ID"),
		TEXT("YOU WANT TO APPLY")
	}
};

static HudText InstallWait =
{
	4,
	{
		TEXT(""),
		TEXT("THE UPDATE IS BEING APPLIED"),
		TEXT(""),
		TEXT("PLEASE WAIT")
	}
};

static HudText UpdateFailed =
{
	4,
	{
		TEXT(""),
		TEXT("CAUTION"),
		TEXT(""),
		TEXT("FAILED TO APPLY THE UPDATE")
	}
};

static HudText BmsNotFound =
{
	4,
	{
		TEXT(""),
		TEXT("BMS 4 WAS NOT FOUND"),
		TEXT(""),
		TEXT("PLEASE INSTALL BMS 4 FIRST")
	}
};

static HudText UpdateDone =
{
	3,
	{
		TEXT(""),
		TEXT(""),
		TEXT("THE UPDATE HAS BEEN APPLIED"),
	}
};

#define HUD_MAIN_MENU   0
#define HUD_INSTALL     1
#define HUD_ABOUT       2
#define HUD_EXIT        3
#define HUD_UPDATING    4
#define HUD_BAD_UPDATE  5
#define HUD_UPDATE_DONE 6
#define HUD_NO_BMS      7

HudContent::HudContent()
{
	setMainMenu();
	if (lstrlen(BMSGetInstallPath()) == 0)
	{
		menu_ = NULL;
		text_ = &BmsNotFound;
		state_ = HUD_NO_BMS;
	}
}

const HudMenu* HudContent::menu() const
{
	return menu_;
}

const HudText* HudContent::text() const
{
	return text_;
}

const TCHAR* HudContent::title() const
{
	return title_;
}

const TCHAR* HudContent::prompt() const
{
	return prompt_;
}

const TCHAR* HudContent::input() const
{
	return input_;
}

void HudContent::process(TCHAR ch)
{
	if (state_ == HUD_MAIN_MENU)
	{
		if (ch == TEXT('-'))
		{
			if (menu_->sel > 0)
				--menu_->sel;
		}
		else if (ch == TEXT('='))
		{
			if (++menu_->sel >= menu_->count)
				menu_->sel = menu_->count - 1;
		}
		else if (ch == TEXT('>'))
		{
			if (menu_->sel == 0)
			{
				state_ = HUD_INSTALL;
				menu_ = NULL;
				text_ = &InstallInstruction;
				title_ = TEXT("BMS 4 UPDATE INSTALLER");
				prompt_ = TEXT("UPDATE ID");
				input_[0] = TEXT('\0');
			}
			else if (menu_->sel == 1)
			{
				state_ = HUD_ABOUT;
				menu_ = NULL;
				text_ = &AboutInstaller;
				title_ = TEXT("BMS 4 UPDATE INSTALLER");
				prompt_ = TEXT("");
				input_[0] = TEXT('\0');
			}
			else if (menu_->sel == 2)
			{
				state_ = HUD_EXIT;
			}
		}
	}
	else if (state_ == HUD_INSTALL)
	{
		if (ch == TEXT('>'))
		{
			updateParams_.updateId = _ttoi(input_);
			wsprintf(updateParams_.filename, TEXT("Falcon_BMS_4.32_Update_%d.exe"), updateParams_.updateId);
			if (_taccess(updateParams_.filename, 0) != 0)
			{
				setFailed();
			}
			else
			{
				state_ = HUD_UPDATING;
				menu_ = NULL;
				text_ = &InstallWait;
				title_ = TEXT("BMS 4 UPDATE INSTALLER");
				prompt_ = TEXT("");
				input_[0] = TEXT('\0');

				_beginthread(updateThread, 0, this);
			}
		}
		else if (ch == TEXT('<'))
		{
			if (input_[0])
				input_[lstrlen(input_) - 1] = TEXT('\0');
		}
		else if (ch >= TEXT('0') && ch <= TEXT('9'))
		{
			int len;
			len = lstrlen(input_);

			if (len < 8)
			{
				input_[len++] = ch;
				input_[len] = TEXT('\0');
			}
		}
		else if (ch == TEXT('/'))
		{
			setMainMenu();
		}
	}
	else if (state_ == HUD_ABOUT || state_ == HUD_BAD_UPDATE || state_ == HUD_UPDATE_DONE)
	{
		if (ch == TEXT('>'))
		{
			setMainMenu();
		}
	}
	else if (state_ == HUD_UPDATING)
	{
		if (ch == TEXT('/'))
		{
			setMainMenu();
		}
	}
	else if (state_ == HUD_NO_BMS)
	{
		if (ch == TEXT('>'))
		{
			state_ = HUD_EXIT;
		}
	}
}

BOOL HudContent::shoudExit() const
{
	return state_ == HUD_EXIT;
}

void HudContent::setMainMenu()
{
	state_ = HUD_MAIN_MENU;
	menu_ = &MainMenu;
	text_ = NULL;
	title_ = TEXT("BMS 4 UPDATE INSTALLER");
	prompt_ = TEXT("");
	input_[0] = TEXT('\0');
}

void HudContent::setFailed()
{
	state_ = HUD_BAD_UPDATE;
	menu_ = NULL;
	text_ = &UpdateFailed;
	title_ = TEXT("BMS 4 UPDATE INSTALLER");
	prompt_ = TEXT("");
	input_[0] = TEXT('\0');

	AudioDevice::sharedDevice()->caution();
}

void HudContent::setHost(HWND hwnd)
{
	hwnd_ = hwnd;
}

void HudContent::updateThread(void *param)
{
	TCHAR path[MAX_PATH];
	HudContent *self = (HudContent*)param;
	SupportDir *support = SupportDir::sharedSupportDir();

	ExpandUpdate(self->updateParams_.filename);
	if (_taccess(support->pathName(TEXT("data\\local.ver"), path), 0) == 0)
	{
		/* apply all patch items */

		int part = 1;
		HANDLE fp;
		DWORD readsize;
		TCHAR shortName[MAX_PATH];
		TCHAR fullName[MAX_PATH];
		unsigned char buf[4];

		while (1)
		{
			wsprintf(shortName, TEXT("data\\4.32_Update_%d_Part_%d.bsf"),
				self->updateParams_.updateId, part++);
			
			support->pathName(shortName, fullName);

			fp = CreateFile(fullName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fp != INVALID_HANDLE_VALUE)
			{
				memset(buf, 0, sizeof(buf));
				ReadFile(fp, buf, sizeof(buf), &readsize, NULL);

				if (buf[0] == 0x37 && buf[1] == 0x7a)  // 7z
				{
					Expand7Zip(fullName, BMSGetInstallPath());
				}
				else if (buf[0] == 0x52 && buf[1] == 0x61 && buf[2] == 0x72)  // rar
				{
					ExpandRAR(fullName, BMSGetInstallPath());
				}
				else  // should be exe
				{
					LaunchAppAndWait(fullName, TEXT(""), TRUE);
				}
			}
			else
			{
				/* it's finished */

				self->state_ = HUD_UPDATE_DONE;
				self->text_ = &UpdateDone;
				
				AudioDevice::sharedDevice()->bingo();

				InvalidateRect(self->hwnd_, NULL, TRUE);

				break;
			}
		}
	}
	else
	{
		/* maybe not an update? */
		self->setFailed();
	}

	/* all done */
	support->pathName(TEXT("data"), path);
	KillDirectory(path);
}
