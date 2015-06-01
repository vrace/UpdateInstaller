#include "ExtApp.h"
#include "SupportDir.h"

static TCHAR BMSInstallPath[MAX_PATH];

BOOL LaunchAppAndWait(const TCHAR *app, const TCHAR *args, BOOL hidden)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	BOOL result;
	TCHAR params[1000];

	lstrcpy(params, args);
	GetStartupInfo(&si);

	if (hidden)
	{
		si.dwFlags = si.dwFlags | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}

	result = CreateProcess(app, params, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	
	if (result)
	{
		WaitForSingleObject(pi.hThread, INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else
	{
		DWORD err = GetLastError();
	}

	return result;
}

void KillDirectory(const TCHAR *path)
{
	WIN32_FIND_DATA wfd;
	HANDLE hf;
	TCHAR findwhat[MAX_PATH];

	wsprintf(findwhat, TEXT("%s\\*"), path);

	hf = FindFirstFile(findwhat, &wfd);
	while (hf != INVALID_HANDLE_VALUE)
	{
		if (wfd.cFileName[0] != TEXT('.'))
		{
			wsprintf(findwhat, TEXT("%s\\%s"), path, wfd.cFileName);
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				KillDirectory(findwhat);
				RemoveDirectory(findwhat);
			}
			else
			{
				SetFileAttributes(findwhat, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(findwhat);
			}
		}

		if (!FindNextFile(hf, &wfd))
		{
			FindClose(hf);
			hf = INVALID_HANDLE_VALUE;
		}
	}

	RemoveDirectory(path);
}

BOOL ExpandUpdate(const TCHAR *filename)
{
	TCHAR cmd[1000];

	wsprintf(cmd, TEXT("\"%s\" /S /D="), filename);
	SupportDir::sharedSupportDir()->pathName(TEXT(""), cmd + lstrlen(cmd));
	cmd[lstrlen(cmd) - 1] = TEXT('\0');

	return LaunchAppAndWait(NULL, cmd, TRUE);
}

BOOL Expand7Zip(const TCHAR *filename, const TCHAR *dest)
{
	TCHAR cmd[1000];
	TCHAR exeName[MAX_PATH];

	SupportDir::sharedSupportDir()->pathName(TEXT("7za.exe"), exeName);

	wsprintf(cmd, TEXT("\"%s\" x -y \"-p7agcVV(/A&3f7f/CV/°ÏGR8gJAF/f7a&°Ïr8g)9afg/°Ïergt7HFA)hauadhfH/(°ÏRt8a8937\" \"-o%s\" \"%s\""),
		exeName, dest, filename);

	return LaunchAppAndWait(NULL, cmd, TRUE);
}

BOOL ExpandRAR(const TCHAR *filename, const TCHAR *dest)
{
	TCHAR cmd[1000];
	TCHAR exeName[MAX_PATH];

	SupportDir::sharedSupportDir()->pathName(TEXT("unrar.exe"), exeName);

	wsprintf(cmd, TEXT("\"%s\" x -y -p\"7agcVV(/A&3f7f/CV/°ÏGR8gJAF/f7a&°Ïr8g)9afg/°Ïergt7HFA)hauadhfH/(°ÏRt8a8937\" \"%s\" \"%s\""),
		exeName, filename, dest);

	return LaunchAppAndWait(NULL, cmd, TRUE);
}

LPCTSTR BMSGetInstallPath(void)
{
	static LPCTSTR path = NULL;

	if (!path)
	{
		HKEY key;
		DWORD size;

		path = BMSInstallPath;
		size = MAX_PATH;

		RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Benchmark Sims\\Falcon BMS 4.32"), 0, KEY_READ, &key);
		RegQueryValueEx(key, TEXT("baseDir"), 0, NULL, (LPBYTE)path, &size);

		RegCloseKey(key);
	}

	return path;
}
