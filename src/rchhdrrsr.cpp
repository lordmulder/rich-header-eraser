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
#include "glob.h"
#include <Shlwapi.h>

static const WORD VERSION_MAJOR = VER_RCHHDRRSR_MAJOR;
static const WORD VERSION_MINOR = (10U * VER_RCHHDRRSR_MINOR_HI) + VER_RCHHDRRSR_MINOR_LO;
static const WORD VERSION_PATCH = VER_RCHHDRRSR_PATCH;

static const DWORD DOS_HEADER_LEN = 0x40;

static const int EXIT_NO_RICH_HEADER = 1;
static const int EXIT_INVALID_FILE   = 2;
static const int EXIT_FILE_IO_ERROR  = 3;
static const int EXIT_INVALID_ARGS   = 4;

#define LSHIFT(X,Y) ((((DWORD)(X)) & 0xFF) << (Y))

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
	*pe_offset_out = LSHIFT(data[0x3C], 0) | LSHIFT(data[0x3D], 8) | LSHIFT(data[0x3E], 16) | LSHIFT(data[0x3F], 24);
	if ((*pe_offset_out >= DOS_HEADER_LEN) && (*pe_offset_out < size - 3U))
	{
		return (!memcmp(data + (*pe_offset_out), "PE\0\0", 4));
	}
	return FALSE;
}

static BOOL locate_rich_footer(const BYTE *const data, const DWORD pe_offset, DWORD *const footer_offset_out)
{
	const DWORD limit = pe_offset - 7U;
	BOOL found = FALSE;
	for (DWORD off = DOS_HEADER_LEN; off < limit; ++off)
	{
		if (!memcmp(data + off, "Rich", 4))
		{
			*footer_offset_out = off;
			found = TRUE;
		}
	}
	return found;
}

static BOOL locate_rich_header(const BYTE *const data, const DWORD foor_offset, DWORD *const header_offset_out)
{
	BOOL found = FALSE;
	for (DWORD off = foor_offset - 4U; off >= DOS_HEADER_LEN; --off)
	{
		if (((data[off + 0U] ^ data[foor_offset + 4U]) == 'D') && ((data[off + 1U] ^ data[foor_offset + 5U]) == 'a') &&
			((data[off + 2U] ^ data[foor_offset + 6U]) == 'n') && ((data[off + 3U] ^ data[foor_offset + 7U]) == 'S'))
		{
			*header_offset_out = off;
			found = TRUE;
			break;
		}
	}
	return found;
}

static int update_retval(const int retval, const int retval_new)
{
	return (retval < 0) ? retval_new :
		((retval_new > EXIT_NO_RICH_HEADER) ? max(retval, retval_new) :
			((retval <= EXIT_NO_RICH_HEADER) ? min(retval, retval_new) : retval));
}

static int process_file(const WCHAR *const file_name, const BOOL zero)
{
	con_printf(L">> %s\n\n", file_name);
	con_puts(L"Mapping binary file into memory... ");

	DWORD error = 0U;
	const HANDLE file = open_file(file_name, GENERIC_READ | GENERIC_WRITE, OPEN_EXISTING, &error);
	if (file == INVALID_HANDLE_VALUE)
	{
		con_puts(L"Failed!\n\nSpecified file could *not* be opened for reading/writing:\n");
		if(WCHAR *const messsage = get_error_message(error))
		{
			con_printf(L"%s\n\n", messsage);
			LocalFree((HLOCAL)messsage);
		}
		else
		{
			con_puts(L"An unknown error occurred!\n\n");
		}
		return EXIT_FILE_IO_ERROR;
	}

	mapping_t mapping;
	if(!create_mapping(file, &mapping, 4096U))
	{
		con_puts(L"Failed!\n\nError: Failed to map file into memory!\n\n");
		CloseHandle(file);
		return EXIT_FILE_IO_ERROR;
	}

	con_puts(L"OK\nScanning for MZ/PE headers... ");

	if (!check_hdr_mz(mapping.view, mapping.size))
	{
		con_puts(L"Failed!\n\nFile does *not* look like a valid EXE or DLL file!\n\n");
		CLEANUP_RETURN(EXIT_INVALID_FILE)
	}

	DWORD pe_offset = 0U;
	if (!check_hdr_pe(mapping.view, mapping.size, &pe_offset))
	{
		con_puts(L"Failed!\n\nFile does *not* contain a proper PE format header!\n\n");
		CLEANUP_RETURN(EXIT_INVALID_FILE)
	}

	con_puts(L"OK\nScanning for Rich (DanS) header... ");

	DWORD rich_footer = 0U;
	if (!locate_rich_footer(mapping.view, pe_offset, &rich_footer))
	{
		con_puts(L"Failed!\n\nFile does *not* contain Rich (DanS) header. Already erased or not created by M$?\n\n");
		CLEANUP_RETURN(EXIT_NO_RICH_HEADER)
	}

	DWORD rich_header = 0U;
	if (!locate_rich_header(mapping.view, rich_footer, &rich_header))
	{
		con_puts(L"Failed!\n\nIncomplete or corrupted Rich (DanS) header. Signature could *not* be decoded!\n\n");
		CLEANUP_RETURN(EXIT_NO_RICH_HEADER)
	}

	con_printf(L"OK\nErasing 0x%04X to 0x%04X... ", rich_header, rich_footer + 7U);

	for (DWORD off = rich_header; off < rich_footer + 8U; ++off)
	{
		mapping.view[off] = zero ? 0x00 : rnd_byte();
	}

	FlushViewOfFile(mapping.view, mapping.size);
	destroy_mapping(&mapping);

	con_puts(L"OK\n\n");

	FlushFileBuffers(file);
	CloseHandle(file);

	return 0;
}

static int rchhdrrsr(const int argc, const WCHAR *const argv[])
{
	con_printf(L"Rich-Header Eraser v%u.%02u-%u [%S]\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, __DATE__);
	con_puts(L"Created by LoRd_MuldeR <https://github.com/lordmulder>\n\n");

	if (argc < 2)
	{
		con_puts(L"-----------------------------------------------------------\n");
		con_puts(L"This work is licensed under the CC0 1.0 Universal License.\n");
		con_puts(L"To view a copy of the license, visit:\n");
		con_puts(L"https://creativecommons.org/publicdomain/zero/1.0/legalcode\n");
		con_puts(L"-----------------------------------------------------------\n\n");
		con_puts(L"Usage:\n");
		con_puts(L"  rchhdrrsr.exe [--zero] <file_1> [<file_2> ... <file_N>]\n\n");
		return EXIT_INVALID_ARGS;
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
			return EXIT_INVALID_ARGS;
		}
		break;
	}

	if ((arg_idx >= argc) || (!argv[arg_idx][0U]))
	{
		con_puts(L"Error: Target file name has *not* been specified!\n\n");
		return EXIT_INVALID_ARGS;
	}

	int retval = -1;
	for(; arg_idx < argc; ++arg_idx)
	{
		if(StrPBrkW(argv[arg_idx], L"*?"))
		{
			glob_ctx_t glob_data;
			if(WCHAR *file_name = glob_find(argv[arg_idx], GLOB_FILTER_REG, &glob_data))
			{
				do
				{
					retval = update_retval(retval, process_file(file_name, zero));
					LocalFree((HLOCAL)file_name);
				}
				while(file_name = glob_next(&glob_data));
			}
		}
		else
		{
			retval = update_retval(retval, process_file(argv[arg_idx], zero));
		}
	}

	if(retval < EXIT_SUCCESS)
	{
		con_puts(L">> No matching file(s) found. Nothing to do!\n\n");
		return EXIT_FILE_IO_ERROR;
	}
	else if(retval > EXIT_SUCCESS)
	{
		con_puts((retval == EXIT_NO_RICH_HEADER) ? L">> Done: No headers have been found or erased.\n\n" : L">> Exiting with critical errors !!!\n\n");
		return retval;
	}
	else
	{
		con_puts(L">> Done: All header(s) have been erased succssefully.\n\n");
		return EXIT_SUCCESS;
	}
}

int wmain(const int argc, const WCHAR *const argv[])
{
	int retval = -1;
	con_init();
	rnd_init();
	retval = rchhdrrsr(argc, argv);
	rnd_exit();
	con_exit();
	return retval;
}
