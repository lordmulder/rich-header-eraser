/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "utils.h"

WCHAR *get_error_message(const DWORD error)
{
	WCHAR *buffer = NULL;
	DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (WCHAR*)&buffer, 0, NULL);
	if (len > 0U)
	{
		while((len > 0) && (buffer[len - 1U] < 0x20))
		{
			buffer[--len] = L'\0';
		}
		return buffer;
	}
	return NULL;
}

HANDLE open_file(const WCHAR *const file_name, DWORD *const error)
{
	HANDLE handle = NULL;
	*error = 0U;
	for (DWORD retry = 0; retry < 80; ++retry)
	{
		if (retry > 0U)
		{
			Sleep(retry);
		}
		handle = CreateFileW(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0U, NULL);
		*error = GetLastError();
		if ((handle != INVALID_HANDLE_VALUE) || ((retry > 0U) && ((*error == 0x2) || (*error == 0x3))))
		{
			break;
		}
	}
	return handle;
}
