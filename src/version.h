/*
 * FS utility functions
 * Created by LoRd_MuldeR <mulder2@gmx.de>.
 * 
 * This work is licensed under the CC0 1.0 Universal License.
 * To view a copy of the license, visit:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode
 */

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Version Info
///////////////////////////////////////////////////////////////////////////////

#define VER_RCHHDRRSR_MAJOR      1
#define VER_RCHHDRRSR_MINOR_HI   0
#define VER_RCHHDRRSR_MINOR_LO   1
#define VER_RCHHDRRSR_PATCH      3

///////////////////////////////////////////////////////////////////////////////
// Helper macros (aka: having fun with the C pre-processor)
///////////////////////////////////////////////////////////////////////////////

#define ___VER_RCHHDRRSR_STR___(X)       #X
#define __VER_RCHHDRRSR_STR__(W,X,Y,Z)   ___VER_RCHHDRRSR_STR___(v##W.X##Y-Z)
#define _VER_RCHHDRRSR_STR_(W,X,Y,Z)     __VER_RCHHDRRSR_STR__(W,X,Y,Z)
#define VER_RCHHDRRSR_STR                _VER_RCHHDRRSR_STR_(VER_RCHHDRRSR_MAJOR,VER_RCHHDRRSR_MINOR_HI,VER_RCHHDRRSR_MINOR_LO,VER_RCHHDRRSR_PATCH)
