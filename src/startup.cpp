/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <ShellAPI.h>

int wmain(const int argc, const WCHAR *const *const argv);

extern "C" 
{
	int startup(void)
	{
		//SetErrorMode(SetErrorMode(0x0003) | 0x0003);
		int argc = 0, retval = -1;
		if(const WCHAR *const *const argv = CommandLineToArgvW(GetCommandLineW(), &argc))
		{
			retval = wmain(argc, argv);
			LocalFree((HLOCAL)argv);
		}
		else
		{
			static const WCHAR *const *const norags= { NULL };
			retval = wmain(0, norags);
		}
		ExitProcess(retval);
		return retval;
	}
}
