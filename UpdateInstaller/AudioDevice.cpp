#include "AudioDevice.h"
#include "bass.h"
#include "SupportDir.h"
#include "ExtApp.h"

typedef BOOL (WINAPI *INIT_FUNC)(int, DWORD, DWORD, HWND, GUID*);
typedef void (WINAPI *FREE_FUNC)(void);
typedef HSTREAM (WINAPI *STREAM_CREATE_FUNC)(BOOL, LPCTSTR, QWORD, QWORD, DWORD);
typedef BOOL (WINAPI *PLAY_FUNC)(HSTREAM, BOOL);

namespace BASS
{
	INIT_FUNC init;
	FREE_FUNC release;
	STREAM_CREATE_FUNC createStream;
	PLAY_FUNC play;
};

AudioDevice* AudioDevice::device_ = NULL;

AudioDevice::AudioDevice()
{
	TCHAR path[MAX_PATH];
	
	bass_ = LoadLibrary(SupportDir::sharedSupportDir()->pathName(TEXT("bass.dll"), path));
	if (bass_)
	{
		BASS::init = (INIT_FUNC)GetProcAddress(bass_, "BASS_Init");
		BASS::release = (FREE_FUNC)GetProcAddress(bass_, "BASS_Free");
		BASS::createStream = (STREAM_CREATE_FUNC)GetProcAddress(bass_, "BASS_StreamCreateFile");
		BASS::play = (PLAY_FUNC)GetProcAddress(bass_, "BASS_ChannelPlay");

		BASS::init(-1, 44100, 0, NULL, NULL);

		wsprintf(path, TEXT("%s\\Data\\Music\\ui1.ogg"), BMSGetInstallPath());
		music_ = BASS::createStream(FALSE, path, 0, 0, BASS_SAMPLE_LOOP | BASS_UNICODE);
	}
}

AudioDevice::~AudioDevice()
{
	BASS::release();

	FreeLibrary(bass_);
	bass_ = NULL;
}

AudioDevice* AudioDevice::sharedDevice()
{
	if (!device_)
	{
		device_ = new AudioDevice;
	}

	return device_;
}

void AudioDevice::release()
{
	if (device_)
	{
		delete device_;
		device_ = NULL;
	}
}

void AudioDevice::music()
{
	BASS::play(music_, TRUE);
}

void AudioDevice::buttonPress()
{
	TCHAR path[MAX_PATH];
	HSTREAM hs;

	wsprintf(path, TEXT("%s\\Data\\Sounds\\cockpit\\pushbutton.ogg"), BMSGetInstallPath());
	hs = BASS::createStream(FALSE, path, 0, 0, BASS_STREAM_AUTOFREE | BASS_UNICODE);
	BASS::play(hs, TRUE);
}

void AudioDevice::caution()
{
	TCHAR path[MAX_PATH];
	HSTREAM hs;

	wsprintf(path, TEXT("%s\\Data\\Sounds\\caution.ogg"), BMSGetInstallPath());
	hs = BASS::createStream(FALSE, path, 0, 0, BASS_STREAM_AUTOFREE | BASS_UNICODE);
	BASS::play(hs, TRUE);
}

void AudioDevice::bingo()
{
	TCHAR path[MAX_PATH];
	HSTREAM hs;

	wsprintf(path, TEXT("%s\\Data\\Sounds\\bingo.ogg"), BMSGetInstallPath());
	hs = BASS::createStream(FALSE, path, 0, 0, BASS_STREAM_AUTOFREE | BASS_UNICODE);
	BASS::play(hs, TRUE);
}
