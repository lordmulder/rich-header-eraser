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

WCHAR *glob_find(const WCHAR *const pattern, ULONG_PTR *const ctx);
WCHAR *glob_next(ULONG_PTR *const ctx);
void   glob_free(ULONG_PTR *const ctx);
