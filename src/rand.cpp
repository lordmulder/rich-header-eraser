/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "rand.h"
#include <limits.h>

#define RESEED_COUNT 999983U

static CRITICAL_SECTION g_mutex;
static DWORD g_state[6U] = { 0x4F5B7CF1, 0x599531DE, 0xBC360195, 0x2933231A, 0x62DFEC75, 0x959E2017 };
static SIZE_T g_reseed_counter = RESEED_COUNT;
static DWORD g_byte_buffer = MAXDWORD;
static SIZE_T g_byte_counter = sizeof(DWORD);

static void mix_function(DWORD &a, DWORD &b, DWORD &c)
{
	a = a - b; a = a - c; a = a ^ (c >> 13);
	b = b - c; b = b - a; b = b ^ (a <<  8);
	c = c - a; c = c - b; c = c ^ (b >> 13);
	a = a - b; a = a - c; a = a ^ (c >> 12);
	b = b - c; b = b - a; b = b ^ (a << 16);
	c = c - a; c = c - b; c = c ^ (b >>  5);
	a = a - b; a = a - c; a = a ^ (c >>  3);
	b = b - c; b = b - a; b = b ^ (a << 10);
	c = c - a; c = c - b; c = c ^ (b >> 15);
}

static DWORD combine(const DWORD a, const DWORD b)
{
	return a ^ (b + 0x9E3779B9 + (a << 6U) + (a >> 2U));
}

static DWORD get_entropy(const DWORD seed)
{
	LARGE_INTEGER perf;
	FILETIME time;
	DWORD a = 0x7869F30D + GetCurrentProcessId();
	DWORD b = 0xD8D35B4B + GetCurrentThreadId();
	DWORD c = seed + ((DWORD)((ULONG_PTR)&seed));
	mix_function(a, b, c);
	for (UINT i = 0U; i < 97U; i++)
	{
		a += GetTickCount();
		GetSystemTimeAsFileTime(&time);
		b += combine(time.dwHighDateTime, time.dwLowDateTime);
		QueryPerformanceCounter(&perf);
		c += combine(perf.HighPart, perf.LowPart);
		mix_function(a, b, c);
	}
	return c;
}

static void rnd_seed(void)
{
	for (UINT i = 0U; i < 13U; ++i)
	{
		g_state[0U] = get_entropy(g_state[0U]);
		g_state[3U] = get_entropy(g_state[3U]);
		g_state[1U] = get_entropy(g_state[1U]);
		g_state[4U] = get_entropy(g_state[4U]);
		g_state[2U] = get_entropy(g_state[2U]);
		g_state[5U] = get_entropy(g_state[5U]);
	}
}

static void rnd_update(void)
{
	g_state[0U] += 1U;
	mix_function(g_state[2U], g_state[1U], g_state[0U]);
	g_state[3U] -= 1U;
	mix_function(g_state[5U], g_state[4U], g_state[3U]);
}

static DWORD _rnd_next(void)
{
	if (++g_reseed_counter >= RESEED_COUNT)
	{
		rnd_seed();
		g_reseed_counter = 0U;
	}
	rnd_update();
	return combine(g_state[0U], g_state[3U]);
}

static BYTE _rnd_byte(void)
{
	if (++g_byte_counter >= sizeof(DWORD))
	{
		g_byte_buffer = _rnd_next();
		g_byte_counter = 0U;
		return (BYTE) g_byte_buffer;
	}
	else
	{
		g_byte_buffer >>= CHAR_BIT;
		return (BYTE) g_byte_buffer;
	}
}

DWORD rnd_next(void)
{
	DWORD retval;
	EnterCriticalSection(&g_mutex);
	retval = _rnd_next();
	LeaveCriticalSection(&g_mutex);
	return retval;
}

BYTE rnd_byte(void)
{
	BYTE retval;
	EnterCriticalSection(&g_mutex);
	retval = _rnd_byte();
	LeaveCriticalSection(&g_mutex);
	return retval;
}

void rnd_init(void)
{
	InitializeCriticalSection(&g_mutex);
}

void rnd_exit(void)
{
	DeleteCriticalSection(&g_mutex);
}
