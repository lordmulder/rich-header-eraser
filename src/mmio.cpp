/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "mmio.h"

static LONGLONG get_file_size(const HANDLE file)
{
	LARGE_INTEGER size;
	if (GetFileSizeEx(file, &size))
	{
		return size.QuadPart;
	}
	return 0U;
}

BOOL create_mapping(const HANDLE file, mapping_t *const mapping_out, const DWORD max_size)
{
	SecureZeroMemory(mapping_out, sizeof(mapping_t));

	const LONGLONG total_size = get_file_size(file);
	if ((total_size < 1U) || (total_size > MAXDWORD))
	{
		return FALSE;
	}

	mapping_out->size = min((DWORD)total_size, max_size);
	if (!(mapping_out->handle = CreateFileMappingW(file, NULL, PAGE_READWRITE, 0U, mapping_out->size, NULL)))
	{
		mapping_out->size = 0U;
		return FALSE;
	}

	mapping_out->view = (BYTE*) MapViewOfFile(mapping_out->handle, FILE_MAP_ALL_ACCESS, 0U, 0U, mapping_out->size);
	if (!mapping_out->view)
	{
		CloseHandle(mapping_out->handle);
		mapping_out->size = 0U;
		mapping_out->handle = NULL;
		return FALSE;
	}

	return TRUE;
}

void destroy_mapping(mapping_t *const mapping_out)
{
	UnmapViewOfFile((LPCVOID)mapping_out->view);
	CloseHandle(mapping_out->handle);
	SecureZeroMemory(mapping_out, sizeof(mapping_t));
}
