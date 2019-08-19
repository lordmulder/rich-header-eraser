/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "glob.h"
#include <Shlwapi.h>

static BOOL dot_or_dotdot(const WCHAR *const file_name)
{
	return (file_name[0U] == L'.') && ((file_name[1U] == L'\0') || ((file_name[1U] == L'.') && (file_name[2U] == L'\0')));
}

static WCHAR *concat_path(const WCHAR *const prefix, const WCHAR *const file_name)
{
	if(prefix && prefix[0U])
	{
		WCHAR *const buffer = (WCHAR*) LocalAlloc(LPTR, sizeof(WCHAR) * (lstrlenW(prefix) + lstrlen(file_name) + 1U));
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

static void clean_up(glob_t *const ctx)
{
	if(ctx->handle != INVALID_HANDLE_VALUE)
	{
		FindClose(ctx->handle);
		ctx->handle = INVALID_HANDLE_VALUE;
	}
	if(ctx->path_prefix)
	{
		LocalFree((HLOCAL)ctx->path_prefix);
		ctx->path_prefix = NULL;
	}
}

WCHAR *glob_find(const WCHAR *const pattern, glob_t *const ctx)
{
	SecureZeroMemory(ctx, sizeof(glob_t));

	SIZE_T prefix_len = 0U;
	for(SIZE_T i = 0U; pattern[i]; ++i)
	{
		if((pattern[i] == '\\') || (pattern[i] == '/'))
		{
			prefix_len = i + 1U;
		}
	}

	WIN32_FIND_DATAW find_data;
	if((ctx->handle = FindFirstFileExW(pattern, FindExInfoBasic, &find_data, FindExSearchNameMatch, NULL, 0U)) == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	if(prefix_len > 0)
	{
		if(!(ctx->path_prefix = (WCHAR*) LocalAlloc(LPTR, sizeof(WCHAR) * (prefix_len + 1U))))
		{
			clean_up(ctx);
			return NULL;
		}
		lstrcpynW(ctx->path_prefix, pattern, prefix_len + 1U);
	}
	
	if(dot_or_dotdot(find_data.cFileName))
	{
		return glob_next(ctx);
	}

	WCHAR *const file_path = concat_path(ctx->path_prefix, find_data.cFileName);
	if(!file_path)
	{
		clean_up(ctx);
	}

	return file_path;
}

WCHAR *glob_next(glob_t *const ctx)
{
	WIN32_FIND_DATAW find_data;
	do
	{
		if(!FindNextFileW(ctx->handle, &find_data))
		{
			clean_up(ctx);
			return NULL;
		}
	}
	while(dot_or_dotdot(find_data.cFileName));

	WCHAR *const file_path = concat_path(ctx->path_prefix, find_data.cFileName);
	if(!file_path)
	{
		clean_up(ctx);
	}

	return file_path;
}
