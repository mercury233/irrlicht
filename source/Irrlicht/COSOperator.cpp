// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COSOperator.h"

#ifdef _IRR_WINDOWS_API_
#ifndef _IRR_XBOX_PLATFORM_
#include <windows.h>
#endif
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef _IRR_OSX_PLATFORM_
#include <sys/sysctl.h>
#endif
#endif

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#include "CIrrDeviceLinux.h"
#endif
#if defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
#import <Cocoa/Cocoa.h>
#endif

#include "fast_atof.h"

namespace irr
{

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
// constructor  linux
	COSOperator::COSOperator(const core::stringc& osVersion, CIrrDeviceLinux* device)
: OperatingSystem(osVersion), IrrDeviceLinux(device)
{
}
#endif

// constructor
COSOperator::COSOperator(const core::stringc& osVersion) : OperatingSystem(osVersion)
{
	#ifdef _DEBUG
	setDebugName("COSOperator");
	#endif
}


//! returns the current operating system version as string.
const core::stringc& COSOperator::getOperatingSystemVersion() const
{
	return OperatingSystem;
}


//! copies text to the clipboard
//! text must be encoded in the system multibyte format (UTF-8 on Linux/macOS)
void COSOperator::copyToClipboard(const c8* text) const
{
	if (!text || text[0] == '\0')
		return;

// Windows version
#if defined(_IRR_XBOX_PLATFORM_)
#elif defined(_IRR_WINDOWS_API_)
	// Convert UTF-8 to UTF-16 for the Windows clipboard
	int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	if (wlen == 0)
		return;

	if (!OpenClipboard(NULL))
		return;

	EmptyClipboard();

	HGLOBAL clipbuffer = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * wlen);
	if (!clipbuffer)
	{
		CloseClipboard();
		return;
	}

	wchar_t* buffer = (wchar_t*)GlobalLock(clipbuffer);
	if (!buffer)
	{
		GlobalFree(clipbuffer);
		CloseClipboard();
		return;
	}

	if (MultiByteToWideChar(CP_UTF8, 0, text, -1, buffer, wlen) == 0)
	{
		GlobalUnlock(clipbuffer);
		GlobalFree(clipbuffer);
		CloseClipboard();
		return;
	}

	GlobalUnlock(clipbuffer);
	if (!SetClipboardData(CF_UNICODETEXT, clipbuffer))
		GlobalFree(clipbuffer);
	CloseClipboard();

#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
    if ((text != NULL) && (strlen(text) > 0))
    {
        NSString* str = [NSString stringWithUTF8String:text];
        if (str)
        {
            NSPasteboard* board = [NSPasteboard generalPasteboard];
            [board declareTypes:@[NSPasteboardTypeString] owner:NSApp];
            [board setString:str forType:NSPasteboardTypeString];
        }
    }

#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
    if ( IrrDeviceLinux )
        IrrDeviceLinux->copyToClipboard(text);
#else

#endif
}


//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
//! Result is encoded in the system multibyte format (UTF-8 on Linux/macOS)
const c8* COSOperator::getTextFromClipboard() const
{
#if defined(_IRR_XBOX_PLATFORM_)
		return 0;
#elif defined(_IRR_WINDOWS_API_)
	if (!OpenClipboard(NULL))
		return 0;

	ClipboardBuffer = "";
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	if (hData)
	{
		wchar_t* wbuffer = (wchar_t*)GlobalLock(hData);
		if (wbuffer)
		{
			// Convert UTF-16 clipboard data to UTF-8 and store in member buffer
			// so the returned pointer remains valid after CloseClipboard()
			int len = WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, NULL, 0, NULL, NULL);
			if (len > 0)
			{
				c8* tmp = new c8[len];
				WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, tmp, len, NULL, NULL);
				ClipboardBuffer = tmp;
				delete[] tmp;
			}
			GlobalUnlock(hData);
		}
	}
	CloseClipboard();
	return ClipboardBuffer.c_str();

#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
    NSPasteboard* board = [NSPasteboard generalPasteboard];
    NSString* str = [board stringForType:NSPasteboardTypeString];
    const char* result = (str != nil) ? [str UTF8String] : nullptr;
    ClipboardBuffer = result ? result : "";
    return ClipboardBuffer.c_str();

#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
    ClipboardBuffer = IrrDeviceLinux ? IrrDeviceLinux->getTextFromClipboard() : "";
    return ClipboardBuffer.c_str();

#else

	return 0;
#endif
}


bool COSOperator::getProcessorSpeedMHz(u32* MHz) const
{
	if (MHz)
		*MHz=0;
#if defined(_IRR_WINDOWS_API_) && !defined(_WIN32_WCE ) && !defined (_IRR_XBOX_PLATFORM_)
	LONG Error;

	HKEY Key;
	Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			__TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
			0, KEY_READ, &Key);

	if(Error != ERROR_SUCCESS)
		return false;

	DWORD Speed = 0;
	DWORD Size = sizeof(Speed);
	Error = RegQueryValueEx(Key, __TEXT("~MHz"), NULL, NULL, (LPBYTE)&Speed, &Size);

	RegCloseKey(Key);

	if (Error != ERROR_SUCCESS)
		return false;
	else if (MHz)
		*MHz = Speed;
	return true;

#elif defined(_IRR_OSX_PLATFORM_)
	struct clockinfo CpuClock;
	size_t Size = sizeof(clockinfo);

	if (!sysctlbyname("kern.clockrate", &CpuClock, &Size, NULL, 0))
		return false;
	else if (MHz)
		*MHz = CpuClock.hz;
	return true;
#else
	// read from "/proc/cpuinfo"
	FILE* file = fopen("/proc/cpuinfo", "r");
	if (file)
	{
		char buffer[1024];
		size_t r = fread(buffer, 1, 1023, file);
		buffer[r] = 0;
		buffer[1023]=0;
		core::stringc str(buffer);
		s32 pos = str.find("cpu MHz");
		if (pos != -1)
		{
			pos = str.findNext(':', pos);
			if (pos != -1)
			{
				while ( str[++pos] == ' ' );
				*MHz = core::fast_atof(str.c_str()+pos);
			}
		}
		fclose(file);
	}
	return (MHz && *MHz != 0);
#endif
}

bool COSOperator::getSystemMemory(u32* Total, u32* Avail) const
{
#if defined(_IRR_WINDOWS_API_) && !defined (_IRR_XBOX_PLATFORM_)

    #if (_WIN32_WINNT >= 0x0500)
	MEMORYSTATUSEX MemoryStatusEx;
 	MemoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);

	// cannot fail
	GlobalMemoryStatusEx(&MemoryStatusEx);

	if (Total)
		*Total = (u32)(MemoryStatusEx.ullTotalPhys>>10);
	if (Avail)
		*Avail = (u32)(MemoryStatusEx.ullAvailPhys>>10);
	return true;
	#else
	MEMORYSTATUS MemoryStatus;
	MemoryStatus.dwLength = sizeof(MEMORYSTATUS);

 	// cannot fail
	GlobalMemoryStatus(&MemoryStatus);

 	if (Total)
		*Total = (u32)(MemoryStatus.dwTotalPhys>>10);
 	if (Avail)
		*Avail = (u32)(MemoryStatus.dwAvailPhys>>10);
    return true;
	#endif

#elif defined(_IRR_POSIX_API_) && defined(_SC_PHYS_PAGES) && defined(_SC_AVPHYS_PAGES)
	long ps = sysconf(_SC_PAGESIZE);
	long pp = sysconf(_SC_PHYS_PAGES);
	long ap = sysconf(_SC_AVPHYS_PAGES);

 	if (ps == -1 || (Total && pp == -1) || (Avail && ap == -1))
		return false;

	if (Total)
		*Total = (u32)((pp>>10)*ps);
	if (Avail)
		*Avail = (u32)((ap>>10)*ps);
	return true;
#elif defined(_IRR_OSX_PLATFORM_)
	int mib[2];
	int64_t physical_memory;
	size_t length;

	// Get the Physical memory size
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	length = sizeof(int64_t);
	sysctl(mib, 2, &physical_memory, &length, NULL, 0);

	if (Total)
		*Total = (u32)(physical_memory>>10);
	if (Avail)
		*Avail = (u32)(physical_memory>>10); // we don't know better
	return true;
#else
	// TODO: implement for others
	return false;
#endif
}


} // end namespace

