/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "rand.h"

static const DWORD BUFFSIZE = 32U;
typedef BOOLEAN (__stdcall *gen_random_t)(PVOID buffer, ULONG len);

static DWORD g_rand_seed = 0;
static HMODULE g_advapi32 = NULL;
static gen_random_t g_gen_rand = NULL;
static CRITICAL_SECTION g_mutex;
static BYTE g_buffer[BUFFSIZE];
static DWORD g_offset = BUFFSIZE;

static DWORD lcg(void)
{
	static const DWORD RAND_MAX = (1U << 31) - 1U;
	return g_rand_seed = (g_rand_seed * 1103515245U + 12345U) & RAND_MAX;
}

static BYTE _rnd_next(void)
{
	if(g_gen_rand)
	{
		if(g_offset >= BUFFSIZE)
		{
			g_gen_rand(g_buffer, BUFFSIZE);
			g_offset = 0U;
		}
		return g_buffer[g_offset++];
	}
	else
	{
		return (BYTE)lcg(); /*fallback*/
	}
}

BYTE rnd_next(void)
{
	BYTE retval;
	EnterCriticalSection(&g_mutex);
	retval = _rnd_next();
	LeaveCriticalSection(&g_mutex);
	return retval;
}

void rnd_init(void)
{
	if(g_advapi32 = LoadLibraryW(L"advapi32.dll"))
	{
		g_gen_rand = (gen_random_t) GetProcAddress(g_advapi32, "SystemFunction036");
	}
	InitializeCriticalSection(&g_mutex);
}

void rnd_exit(void)
{
	if(g_advapi32)
	{
		FreeLibrary(g_advapi32);
	}
	g_advapi32 = NULL;
	g_gen_rand = NULL;
}
