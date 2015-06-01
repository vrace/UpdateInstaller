#pragma once

#include <Windows.h>

class SupportDir
{
public:
	static SupportDir* sharedSupportDir();
	void release();

	TCHAR* pathName(const TCHAR *name, TCHAR *buffer);

private:
	SupportDir();
	~SupportDir();

	void extractFile(const TCHAR *name, unsigned char *buffer, size_t size);

private:
	TCHAR path_[MAX_PATH];
	static SupportDir *supportDir_;
};
