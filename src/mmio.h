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

typedef struct _mapping_t
{
	DWORD size;
	HANDLE handle;
	BYTE *view;
}
mapping_t;

BOOL create_mapping(const HANDLE file, mapping_t *const mapping_out, const DWORD max_size);
void destroy_mapping(mapping_t *const mapping_out);
