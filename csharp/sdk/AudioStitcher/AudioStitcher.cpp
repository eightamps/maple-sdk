// AudioStitcher.cpp : Defines the exported functions for the DLL.

#include "pch.h"
#include "framework.h"
#include "AudioStitcher.h"


// This is an example of an exported variable
AUDIOSTITCHER_API int nAudioStitcher=0;

// This is an example of an exported function.
AUDIOSTITCHER_API int fnAudioStitcher(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
CAudioStitcher::CAudioStitcher()
{
    return;
}
