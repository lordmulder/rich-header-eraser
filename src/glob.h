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

typedef ULONG_PTR glob_ctx_t;

WCHAR *glob_find(const WCHAR *const pattern, glob_ctx_t *const ctx);
WCHAR *glob_next(glob_ctx_t *const ctx);
void   glob_free(glob_ctx_t *const ctx);
