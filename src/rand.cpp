/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "rand.h"

static CRITICAL_SECTION g_mutex;
static BOOL g_seeded = FALSE;
static DWORD g_state[3U] = { 0x4F5B7CF1, 0x599531DE, 0xBC360195 };
static DWORD g_byte_buffer;
static SIZE_T g_byte_counter = 4U;

static DWORD mix_function(DWORD a, DWORD b, DWORD c)
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
	return c;
}

static DWORD get_entropy(DWORD seed)
{
	LARGE_INTEGER perf;
	FILETIME time;
	for (SIZE_T i = 0U; i < 31U; i++)
	{
		GetSystemTimeAsFileTime(&time);
		seed = mix_function(time.dwHighDateTime, time.dwLowDateTime, seed);
		QueryPerformanceCounter(&perf);
		seed = mix_function(perf.HighPart, perf.LowPart, seed);
	}
	return seed;
}

static void rnd_seed(void)
{
	for (SIZE_T i = 0U; i < 31U; i++)
	{
		g_state[0U] = get_entropy(g_state[0U]);
		g_state[1U] = get_entropy(g_state[1U]);
		g_state[2U] = get_entropy(g_state[2U]);
	}
}

static void rnd_update(void)
{
	g_state[0U] = mix_function(g_state[1U], g_state[2U], g_state[0U] + 1U);
	g_state[1U] = mix_function(g_state[2U], g_state[0U], g_state[1U] + 1U);
	g_state[2U] = mix_function(g_state[0U], g_state[1U], g_state[2U] + 1U);
}

static DWORD _rnd_next(void)
{
	if (!g_seeded)
	{
		rnd_seed();
		g_seeded = TRUE;
	}
	rnd_update();
	return g_state[0U];
}

static BYTE _rnd_byte(void)
{
	if (++g_byte_counter > 3U)
	{
		g_byte_buffer = _rnd_next();
		g_byte_counter = 0U;
		return (BYTE) g_byte_buffer;
	}
	g_byte_buffer >>= 8U;
	return (BYTE) g_byte_buffer;
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
