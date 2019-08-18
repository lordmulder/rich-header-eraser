/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "console.h"
#include "mmio.h"
#include "rand.h"
#include "utils.h"
#include "version.h"

static const WORD VERSION_MAJOR = VER_RCHHDRRSR_MAJOR;
static const WORD VERSION_MINOR = (10U * VER_RCHHDRRSR_MINOR_HI) + VER_RCHHDRRSR_MINOR_LO;
static const WORD VERSION_PATCH = VER_RCHHDRRSR_PATCH;

static const DWORD DOS_STUB_LEN = 0x80;

#define CLEANUP_RETURN(X) \
	destroy_mapping(&mapping); \
	CloseHandle(file); \
	return (X);

static BOOL check_hdr_mz(const BYTE *const data, const DWORD size)
{
	return (size >= 0x100) && (data[0x0] == 'M') && (data[0x1] == 'Z');
}

static BOOL check_hdr_pe(const BYTE *const data, const DWORD size, DWORD *const pe_offset_out)
{
	*pe_offset_out = data[0x3C] | (data[0x3D] << 8) | (data[0x3E] << 16) | (data[0x3E] << 24);
	return (*pe_offset_out > DOS_STUB_LEN) && (*pe_offset_out < size - 3U) && (!memcmp(data + (*pe_offset_out), "PE\0\0", 4));
}

static BOOL locate_rich_footer(const BYTE *const data, const DWORD pe_offset, DWORD *const footer_offset_out)
{
	BOOL found = false;
	for (DWORD off = DOS_STUB_LEN; off < pe_offset - 7U; ++off)
	{
		if (!memcmp(data + off, "Rich", 4))
		{
			*footer_offset_out = off;
			found = true;
		}
	}
	return found;
}

static BOOL locate_rich_header(const BYTE *const data, const DWORD foor_offset, DWORD *const header_offset_out)
{
	BOOL found = false;
	for (DWORD off = foor_offset - 4U; off >= DOS_STUB_LEN; --off)
	{
		if (((data[off + 0U] ^ data[foor_offset + 4U]) == 'D') && ((data[off + 1U] ^ data[foor_offset + 5U]) == 'a') &&
			((data[off + 2U] ^ data[foor_offset + 6U]) == 'n') && ((data[off + 3U] ^ data[foor_offset + 7U]) == 'S'))
		{
			*header_offset_out = off;
			found = true;
			break;
		}
	}
	return found;
}

static int rchhdrrsr(const int argc, const WCHAR *const *const argv)
{
	con_printf(L"Rich-Header Eraser v%u.%02u-%u [%S]\n\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, __DATE__);

	if (argc < 2)
	{
		con_puts(L"Usage:\n  rchhdrrsr.exe [--zero] <path_to_binary>\n\n");
		return 1;
	}

	int arg_idx = 1;
	BOOL zero = FALSE;

	for(; arg_idx < argc; ++arg_idx)
	{
		if((argv[arg_idx][0U] == L'-') && (argv[arg_idx][1U] == L'-'))
		{
			if(argv[arg_idx][2U] == L'\0')
			{
				++arg_idx;
				break;
			}
			if(!lstrcmpiW(argv[arg_idx], L"--zero"))
			{
				zero = TRUE;
				continue;
			}
			con_printf(L"Error: Option \"%s\" is unknown!\n\n", argv[arg_idx]);
			return 2;
		}
		break;
	}

	if ((arg_idx >= argc) || (!argv[arg_idx][0U]))
	{
		con_puts(L"Error: Target file name has *not* been specified!\n\n");
		return 3;
	}
	else if (arg_idx < argc - 1)
	{
		con_puts(L"Warning: Excess command-line argument was ignored!\n\n");
	}

	con_puts(L"Mapping binary file into memory... ");

	DWORD error = 0U;
	const HANDLE file = open_file(argv[arg_idx], GENERIC_READ | GENERIC_WRITE, OPEN_EXISTING, &error);
	if (file == INVALID_HANDLE_VALUE)
	{
		con_printf(L"Failed!\n\nSpecified file could *not* be opened for reading/writing:\n%s\n\n", argv[arg_idx]);
		if(WCHAR *const messsage = get_error_message(error))
		{
			con_printf(L"%s\n\n", messsage);
			LocalFree((HLOCAL)messsage);
		}
		return 4;
	}

	mapping_t mapping;
	if(!create_mapping(file, &mapping, 4096U))
	{
		con_puts(L"Failed!\n\nError: Failed to map file into memory!\n\n");
		CloseHandle(file);
		return 5;
	}

	con_puts(L"OK\nScanning for PE header... ");

	if (!check_hdr_mz(mapping.view, mapping.size))
	{
		con_puts(L"Failed!\n\nFile does *not* look like a valid EXE or DLL file!\n\n");
		CLEANUP_RETURN(6)
	}

	DWORD pe_offset = 0U;
	if (!check_hdr_pe(mapping.view, mapping.size, &pe_offset))
	{
		con_puts(L"Failed!\n\nFile does *not* contain a proper PE format header!\n\n");
		CLEANUP_RETURN(7)
	}

	con_puts(L"OK\nScanning for Rich (DanS) header... ");

	DWORD rich_footer = 0U;
	if (!locate_rich_footer(mapping.view, pe_offset, &rich_footer))
	{
		con_puts(L"Failed!\n\nFile does *not* seem to contain Rich (DanS) header. Already erased or not created by M$?\n\n");
		CLEANUP_RETURN(8)
	}

	DWORD rich_header = 0U;
	if (!locate_rich_header(mapping.view, rich_footer, &rich_header))
	{
		con_puts(L"Failed!\n\nFound a Rich header, but 'DanS' signature could *not* be decoded!\n\n");
		CLEANUP_RETURN(9)
	}

	con_printf(L"OK\nErasing 0x%04X to 0x%04X... ", rich_header, rich_footer + 7U);

	for (DWORD off = rich_header; off < rich_footer + 8U; ++off)
	{
		mapping.view[off] = zero ? 0x00 : rnd_next_byte();
	}

	FlushViewOfFile(mapping.view, mapping.size);
	destroy_mapping(&mapping);

	con_puts(L"OK\n\n");

	FlushFileBuffers(file);
	CloseHandle(file);

	con_puts(L"Completed.\n\n");
	return 0;
}

int wmain(const int argc, const WCHAR *const *const argv)
{
	int retval = -1;
	con_init();
	rnd_init();
	retval = rchhdrrsr(argc, argv);
	con_exit();
	return retval;
}
