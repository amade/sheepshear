/*
 *  sysdeps.h - System dependent definitions for BeOS
 *
 *  SheepShaver (C) 1997-2008 Christian Bauer and Marc Hellwig
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SYSDEPS_H
#define SYSDEPS_H

// Do we have std namespace?
#ifdef __POWERPC__
#define NO_STD_NAMESPACE
#endif

#include <assert.h>
#include <interface/GraphicsDefs.h>
#include <KernelKit.h>
#include <support/ByteOrder.h>
#include <string.h>
#include <sys/types.h>

#include "user_strings_beos.h"

// Are we using a PPC emulator or the real thing?
#ifdef __POWERPC__
#define EMULATED_PPC 0
#define WORDS_BIGENDIAN 1
#define SYSTEM_CLOBBERS_R2 1
#define REAL_ADDRESSING 1
#else
// Not PowerPC
#define EMULATED_PPC 1
#undef  WORDS_BIGENDIAN
#ifdef NATMEM_OFFSET
#define DIRECT_ADDRESSING 1
#else
#define REAL_ADDRESSING 1
#endif
#endif

// Define for external components
#define SHEEPSHAVER 1

// Always use Real Addressing mode on native architectures
// Otherwise, use Direct Addressing mode if NATMEM_OFFSET is set
#if !defined(EMULATED_PPC)
#include "ppc_asm.tmpl"
#elif defined(NATMEM_OFFSET)
#define DIRECT_ADDRESSING 1
#else
#define REAL_ADDRESSING 1
#endif


// High precision timing
#define PRECISE_TIMING 1
#define PRECISE_TIMING_BEOS 1

#define POWERPC_ROM 1

// Byte swap functions
#define bswap_16 B_SWAP_INT16
#define bswap_32 B_SWAP_INT32
#define bswap_64 B_SWAP_INT64

// Time data type for Time Manager emulation
typedef bigtime_t tm_time_t;

// 64 bit file offsets
typedef off_t loff_t;

// Data types
typedef uint32 uintptr;
typedef int32 intptr;

// Timing functions
extern void Delay_usec(uint32 usec);

// Macro for calling MacOS routines
#define CallMacOS(type, proc) (*(type)proc)()
#define CallMacOS1(type, proc, arg1) (*(type)proc)(arg1)
#define CallMacOS2(type, proc, arg1, arg2) (*(type)proc)(arg1, arg2)
#define CallMacOS3(type, proc, arg1, arg2, arg3) (*(type)proc)(arg1, arg2, arg3)
#define CallMacOS4(type, proc, arg1, arg2, arg3, arg4) (*(type)proc)(arg1, arg2, arg3, arg4)
#define CallMacOS5(type, proc, arg1, arg2, arg3, arg4, arg5) (*(type)proc)(arg1, arg2, arg3, arg4, arg5)
#define CallMacOS6(type, proc, arg1, arg2, arg3, arg4, arg5, arg6) (*(type)proc)(arg1, arg2, arg3, arg4, arg5, arg6)
#define CallMacOS7(type, proc, arg1, arg2, arg3, arg4, arg5, arg6, arg7) (*(type)proc)(arg1, arg2, arg3, arg4, arg5, arg6, arg7)

#endif
