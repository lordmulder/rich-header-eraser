/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#pragma warning(disable: 4200)

#include "glob.h"
#include <Shlwapi.h>

#define CTX ((glob_private_t*)(*ctx))
#define IS_IGNORED_ENTRY(X) (((X).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || dot_or_dotdot((X).cFileName))

typedef struct _glob_private_t
{
	HANDLE handle;
	WCHAR path_prefix[];
}
glob_private_t;

static BOOL dot_or_dotdot(const WCHAR *const file_name)
{
	return (file_name[0U] == L'.') && ((file_name[1U] == L'\0') || ((file_name[1U] == L'.') && (file_name[2U] == L'\0')));
}

static SIZE_T get_prefix_len(const WCHAR *const pattern)
{
	SIZE_T prefix_len = 0U;
	for(SIZE_T i = 0U; pattern[i]; ++i)
	{
		if((pattern[i] == L'\\') || (pattern[i] == L'/'))
		{
			prefix_len = i + 1U;
		}
	}
	return prefix_len;
}

static WCHAR *concat_path(const WCHAR *const prefix, const WCHAR *const file_name)
{
	if(prefix[0U])
	{
		WCHAR *const buffer = (WCHAR*) LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlenW(prefix) + lstrlenW(file_name) + 1U));
		if(buffer)
		{
			lstrcpyW(buffer, prefix);
			lstrcatW(buffer, file_name);
		}
		return buffer;
	}
	else
	{
		return StrDupW(file_name);
	}
}

WCHAR *glob_find(const WCHAR *const pattern, glob_ctx_t *const ctx)
{
	*ctx = NULL;

	WIN32_FIND_DATAW find_data;
	const HANDLE handle = FindFirstFileExW(pattern, FindExInfoBasic, &find_data, FindExSearchNameMatch, NULL, 0U);
	if(handle == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	const SIZE_T prefix_len = get_prefix_len(pattern);
	if(!(*ctx = (ULONG_PTR) LocalAlloc(LPTR, sizeof(glob_ctx_t) + (sizeof(WCHAR) * (prefix_len + 1U)))))
	{
		FindClose(handle);
		return NULL;
	}

	CTX->handle = handle;
	if(prefix_len)
	{
		lstrcpynW(CTX->path_prefix, pattern, prefix_len + 1U);
	}
	
	if(IS_IGNORED_ENTRY(find_data))
	{
		return glob_next(ctx);
	}

	WCHAR *const file_path = concat_path(CTX->path_prefix, find_data.cFileName);
	if(!file_path)
	{
		glob_free(ctx);
	}

	return file_path;
}

WCHAR *glob_next(glob_ctx_t *const ctx)
{
	if(!(*ctx))
	{
		return NULL;
	}

	WIN32_FIND_DATAW find_data;
	do
	{
		if(!FindNextFileW(CTX->handle, &find_data))
		{
			glob_free(ctx);
			return NULL;
		}
	}
	while(IS_IGNORED_ENTRY(find_data));

	WCHAR *const file_path = concat_path(CTX->path_prefix, find_data.cFileName);
	if(!file_path)
	{
		glob_free(ctx);
	}

	return file_path;
}

void glob_free(glob_ctx_t *const ctx)
{
	if(*ctx)
	{
		if(CTX->handle != INVALID_HANDLE_VALUE)
		{
			FindClose(CTX->handle);
		}
		LocalFree((HLOCAL)(*ctx));
		*ctx = NULL;
	}
}
