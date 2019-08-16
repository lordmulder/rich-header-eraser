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

HANDLE open_file(const WCHAR *const file_name, const DWORD access_flags, const DWORD creation_flags, DWORD *const error);
WCHAR *get_error_message(const DWORD error);
