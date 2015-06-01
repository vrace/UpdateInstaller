#include "SupportDir.h"
#include "ExtApp.h"
#include "BassModule.h"
#include "7za.h"
#include "unrar.h"

SupportDir* SupportDir::supportDir_ = NULL;

SupportDir* SupportDir::sharedSupportDir()
{
	if (!supportDir_)
		supportDir_ = new SupportDir();

	return supportDir_;
}

void SupportDir::release()
{
	if (supportDir_)
		delete supportDir_;

	supportDir_ = NULL;
}

TCHAR* SupportDir::pathName(const TCHAR *name, TCHAR *buffer)
{
	wsprintf(buffer, TEXT("%s\\%s"), path_, name);
	return buffer;
}

void SupportDir::extractFile(const TCHAR *name, unsigned char *buffer, size_t size)
{
	TCHAR path[MAX_PATH];
	HANDLE file;
	DWORD written;

	file = CreateFile(pathName(name, path), FILE_GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		WriteFile(file, buffer, size, &written, NULL);
		CloseHandle(file);
	}
}

SupportDir::SupportDir()
{
	TCHAR tmpdir[MAX_PATH];

	GetTempPath(MAX_PATH, tmpdir);
	memset(path_, 0, sizeof(TCHAR) * MAX_PATH);
	GetTempFileName(tmpdir, TEXT("bms"), 0, path_);

	DeleteFile(path_);
	CreateDirectory(path_, NULL);

	extractFile(TEXT("bass.dll"), BassModule, sizeof(BassModule));
	extractFile(TEXT("7za.exe"), SevenZip, sizeof(SevenZip));
	extractFile(TEXT("unrar.exe"), UnRAR, sizeof(UnRAR));
}

SupportDir::~SupportDir()
{
	KillDirectory(path_);
}
