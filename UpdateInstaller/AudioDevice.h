#include <Windows.h>
#include "bass.h"

class AudioDevice
{
public:

	static AudioDevice* sharedDevice();
	void release();

	void music();
	void buttonPress();
	void caution();
	void bingo();

private:

	AudioDevice();
	~AudioDevice();

private:

	HMODULE bass_;
	HSTREAM music_;

	static AudioDevice *device_;
};
