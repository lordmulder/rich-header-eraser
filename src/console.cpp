/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "console.h"
#include <Shlwapi.h>

#define BUFFSIZE_UTF16 4096U
#define BUFFSIZE_UTF8 16384U

static UINT cp_original;
static CRITICAL_SECTION g_mutex;
static HANDLE g_stdout;
static WCHAR g_buffer_utf16[BUFFSIZE_UTF16];
static char g_buffer_utf8[BUFFSIZE_UTF8];

static DWORD utf16_to_utf8(const WCHAR *const input, char *const output, const DWORD out_size)
{
	const DWORD result = WideCharToMultiByte(CP_UTF8, 0U, input, -1, output, out_size, NULL, NULL);
	return ((result > 0U) && (result <= out_size)) ? result - 1U : 0U;
}

static int _puts(const WCHAR *const text)
{
	const DWORD len = utf16_to_utf8(text, g_buffer_utf8, BUFFSIZE_UTF8);
	if (len > 0U)
	{
		DWORD written;
		if (WriteFile(g_stdout, g_buffer_utf8, len, &written, NULL))
		{
			FlushFileBuffers(g_stdout);
			return len;
		}
	}
	return 0U;
}

static int _printf(const WCHAR *const format, const va_list args)
{
	const int result = wvnsprintfW(g_buffer_utf16, BUFFSIZE_UTF16, format, args);
	if (result > 0)
	{
		return _puts(g_buffer_utf16);
	}
	return 0U;
}

int puts(const WCHAR *const text)
{
	int retval;
	EnterCriticalSection(&g_mutex);
	retval = _puts(text);
	LeaveCriticalSection(&g_mutex);
	return retval;
}

int printf(const WCHAR *const format, ...)
{
	va_list ap;
	int retval;
	EnterCriticalSection(&g_mutex);
	va_start(ap, format);
	retval = _printf(format, ap);
	va_end(ap);
	LeaveCriticalSection(&g_mutex);
	return retval;
}

void con_init(void)
{
	cp_original = GetConsoleCP();
	SetConsoleCP(CP_UTF8);
	g_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	InitializeCriticalSection(&g_mutex);
}

void con_exit(void)
{
	if(cp_original != 0U)
	{
		SetConsoleCP(cp_original);
	}
}
