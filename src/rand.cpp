/*
 * Rich-Header Eraser
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#include "rand.h"

#define N 624U
#define M 397U

static DWORD g_vector[N], g_rbuff;
static SIZE_T g_idx = N + 1U, g_shift = 4U;
static CRITICAL_SECTION g_mutex;

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

static DWORD create_seed(void)
{
	DWORD seed = 0x4F5B7CF1;
	FILETIME time;
	LARGE_INTEGER perf;
	for (SIZE_T i = 0; i < 97U; i++)
	{
		GetSystemTimeAsFileTime(&time);
		seed = mix_function(time.dwHighDateTime, time.dwLowDateTime, seed);
		QueryPerformanceCounter(&perf);
		seed = mix_function(perf.HighPart, perf.LowPart, seed);
	}
	return seed;
}

static void MT_init(DWORD *const p, const SIZE_T len, DWORD seed)
{
	static const DWORD mult = 1812433253UL;
	for (SIZE_T i = 0; i < len; i++)
	{
		p[i] = seed;
		seed = mult * (seed ^ (seed >> 30)) + (i+1U);
	}
}

static void MT_update(DWORD *const p)
{
	static const DWORD A[2U] = { 0U, 0x9908B0DF };
	SIZE_T i = 0;
	for (; i < N-M; ++i)
	{
		p[i] = p[i+(M)] ^ (((p[i] & 0x80000000) | (p[i+1U] & 0x7FFFFFFF)) >> 1U) ^ A[p[i+1U] & 1U];
	}
	for (; i < N-1; ++i)
	{
		p[i] = p[i+(M-N)] ^ (((p[i] & 0x80000000) | (p[i+1U] & 0x7FFFFFFF)) >> 1) ^ A[p[i+1U] & 1U];
	}
	p[N-1U] = p[M-1U] ^ (((p[N-1U] & 0x80000000) | (p[0U] & 0x7FFFFFFF)) >> 1) ^ A[p[0U] & 1U];
}

static DWORD MT_next()
{
	if (g_idx >= N)
	{
		if(g_idx > N)
		{
			MT_init(g_vector, N, create_seed());
		}
		MT_update(g_vector);
		g_idx = 0U;
	}
	DWORD e  = g_vector[g_idx++];
	e ^= (e >> 11U);
	e ^= (e <<  7U) & 0x9D2C5680;
	e ^= (e << 15U) & 0xEFC60000;
	e ^= (e >> 18U);
	return e;
}

static BYTE _next_byte(void)
{
	if(g_shift >= 4U)
	{
		g_rbuff = MT_next();
		g_shift = 0U;
	}
	return (BYTE)(g_rbuff >> ((g_shift++) * 8U));
}

BYTE rnd_next_byte(void)
{
	BYTE retval;
	EnterCriticalSection(&g_mutex);
	retval = _next_byte();
	LeaveCriticalSection(&g_mutex);
	return retval;
}

DWORD rnd_next_word(void)
{
	DWORD retval;
	EnterCriticalSection(&g_mutex);
	retval = MT_next();
	LeaveCriticalSection(&g_mutex);
	return retval;
}

void rnd_init(void)
{
	InitializeCriticalSection(&g_mutex);
}
