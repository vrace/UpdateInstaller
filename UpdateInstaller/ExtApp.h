#include <Windows.h>

LPCTSTR BMSGetInstallPath(void);
BOOL LaunchAppAndWait(const TCHAR *app, const TCHAR *args, BOOL hidden);
void KillDirectory(const TCHAR *path);
BOOL ExpandUpdate(const TCHAR *filename);
BOOL Expand7Zip(const TCHAR *filename, const TCHAR *dest);
BOOL ExpandRAR(const TCHAR *filename, const TCHAR *dest);
