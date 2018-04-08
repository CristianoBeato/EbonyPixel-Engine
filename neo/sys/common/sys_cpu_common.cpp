/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2016-2018 Cristiano Beato.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "sys_cpu_common.h"

#include <sdl.h>
#include <xmmintrin.h>
#include <pmmintrin.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include "sys/win32/win_local.h"
#endif

/*
==============================================================
CPU
==============================================================
*/
#define _REG_EAX		0
#define _REG_EBX		1
#define _REG_ECX		2
#define _REG_EDX		3

#define HT_BIT				0x10000000	// Bit 28 of EDX indicates Hyper-Threading Technology support
#define CMOV_BIT			0x00008000	// bit 15 of EDX denotes CMOV support
#define NUM_LOGICAL_BITS	0x00FF0000	// Register EBX bits 23 through 16 indicate the number of logical processors per package
#define DAZ_BIT				0X00000040	// Bit 6 of MXCSR is daz flag
#define FTZ_BIT				0x00008000	// Bit 15 of MXCSR is FTZ flag
#define ROUND_BIT			0x00006000

const char * btCpuInfo::GetProcessorString(void)
{
	return nullptr;
}

cpuid_t btCpuInfo::GetCPUId(void)
{
	int flags;
	
	// verify we're at least a Pentium or 486 with CPUID support
	if (!HasCPUID())
		return CPUID_UNSUPPORTED;

	// check for an AMD
	if (IsAMD())
		flags = CPUID_AMD;
	else if(IsINTEL())
		flags = CPUID_INTEL;
	else
		flags = CPUID_GENERIC;

	// check for Multi Media Extensions
	if (SDL_HasMMX())
		flags |= CPUID_MMX;

	// check for 3DNow!
	if (SDL_Has3DNow())
		flags |= CPUID_3DNOW;

	// check for Streaming SIMD Extensions
	if (SDL_HasSSE())
		flags |= CPUID_SSE | CPUID_FTZ;

	// check for Streaming SIMD Extensions 2
	if (SDL_HasSSE2())
		flags |= CPUID_SSE2;

	// check for Streaming SIMD Extensions 3 aka Prescott's New Instructions
	if (SDL_HasSSE3())
		flags |= CPUID_SSE3;

	// check for Hyper-Threading Technology
	if( HasHTT() )
		flags |= CPUID_HTT;

	// check for Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
	if( HasCMOV() )
		flags |= CPUID_CMOV;

	// check for Denormals-Are-Zero mode
	if( HasDAZ() )
		flags |= CPUID_DAZ;

	return (cpuid_t)flags;
}

/*
========================
btCpuInfo::CPUCount
numLogicalCPUCores	- the number of logical CPU per core
numPhysicalCPUCores	- the total number of cores per package
numCPUPackages		- the total number of packages (physical processors)
========================
*/
void btCpuInfo::CPUCount(int & numLogicalCPUCores, int & numPhysicalCPUCores, int & numCPUPackages)
{
#if defined(_WIN32) || defined(_WIN64)
	cpuInfo_t cpuInfo;
	Win_GetCPUInfo(cpuInfo);

	numPhysicalCPUCores = cpuInfo.processorCoreCount;
	numLogicalCPUCores = cpuInfo.logicalProcessorCount;
	numCPUPackages = cpuInfo.processorPackageCount;
#elif defined(__linux__)
	static bool		init = false;
	static double	ret;

	static int		s_numLogicalCPUCores;
	static int		s_numPhysicalCPUCores;
	static int		s_numCPUPackages;

	int		fd, len, pos, end;
	char	buf[4096];
	char	number[100];

	if (init)
	{
		numPhysicalCPUCores = s_numPhysicalCPUCores;
		numLogicalCPUCores = s_numLogicalCPUCores;
		numCPUPackages = s_numCPUPackages;
	}

	s_numPhysicalCPUCores = 1;
	s_numLogicalCPUCores = 1;
	s_numCPUPackages = 1;

	fd = open("/proc/cpuinfo", O_RDONLY);
	if (fd != -1)
	{
		len = read(fd, buf, 4096);
		close(fd);
		pos = 0;
		while (pos < len)
		{
			if (!idStr::Cmpn(buf + pos, "processor", 9))
			{
				pos = strchr(buf + pos, ':') - buf + 2;
				end = strchr(buf + pos, '\n') - buf;
				if (pos < len && end < len)
				{
					idStr::Copynz(number, buf + pos, sizeof(number));
					assert((end - pos) > 0 && (end - pos) < sizeof(number));
					number[end - pos] = '\0';

					int processor = atoi(number);

					if ((processor + 1) > s_numPhysicalCPUCores)
					{
						s_numPhysicalCPUCores = processor + 1;
					}
				}
				else
				{
					common->Printf("failed parsing /proc/cpuinfo\n");
					break;
				}
			}
			else if (!idStr::Cmpn(buf + pos, "core id", 7))
			{
				pos = strchr(buf + pos, ':') - buf + 2;
				end = strchr(buf + pos, '\n') - buf;
				if (pos < len && end < len)
				{
					idStr::Copynz(number, buf + pos, sizeof(number));
					assert((end - pos) > 0 && (end - pos) < sizeof(number));
					number[end - pos] = '\0';

					int coreId = atoi(number);

					if ((coreId + 1) > s_numLogicalCPUCores)
					{
						s_numLogicalCPUCores = coreId + 1;
					}
				}
				else
				{
					common->Printf("failed parsing /proc/cpuinfo\n");
					break;
				}
			}

			pos = strchr(buf + pos, '\n') - buf + 1;
}
	}

	common->Printf("/proc/cpuinfo CPU processors: %d\n", s_numPhysicalCPUCores);
	common->Printf("/proc/cpuinfo CPU logical cores: %d\n", s_numLogicalCPUCores);

	numPhysicalCPUCores = s_numPhysicalCPUCores;
	numLogicalCPUCores = s_numLogicalCPUCores;
	numCPUPackages = s_numCPUPackages;
#elif defined( MACOS_X )
	static bool		init = false;

	static int		s_numLogicalCPUCores;
	static int		s_numPhysicalCPUCores;
	static int		s_numCPUPackages;

	size_t len = sizeof(s_numPhysicalCPUCores);

	if (init)
	{
		numPhysicalCPUCores = s_numPhysicalCPUCores;
		numLogicalCPUCores = s_numLogicalCPUCores;
		numCPUPackages = s_numCPUPackages;
	}

	s_numPhysicalCPUCores = 1;
	s_numLogicalCPUCores = 1;
	s_numCPUPackages = 1;


	sysctlbyname("hw.physicalcpu", &s_numPhysicalCPUCores, &len, NULL, 0);
	sysctlbyname("hw.logicalcpu", &s_numLogicalCPUCores, &len, NULL, 0);

	common->Printf("CPU processors: %d\n", s_numPhysicalCPUCores);
	common->Printf("CPU logical cores: %d\n", s_numLogicalCPUCores);

	numPhysicalCPUCores = s_numPhysicalCPUCores;
	numLogicalCPUCores = s_numLogicalCPUCores;
	numCPUPackages = s_numCPUPackages;
#else
	numPhysicalCPUCores = 1;
	numLogicalCPUCores = SDL_GetCPUCount();
	numCPUPackages = 1;
#endif
}

void btCpuInfo::CpuId(int func, unsigned regs[4])
{
	unsigned regEAX, regEBX, regECX, regEDX;
#if defined(__GNUC__) && defined(i386) || defined(__MINGW32__)
	__asm__ __volatile__(\
		"        pushl %%ebx        \n" \
		"        xorl %%ecx,%%ecx   \n" \
		"        cpuid              \n" \
		"        movl %%ebx, %%esi  \n" \
		"        popl %%ebx         \n" : \
		"=a" (regEAX), "=S" (regEBX), "=c" (regECX), "=d" (regEDX) : "a" (func))
#elif defined(__GNUC__) && defined(__x86_64__) || defined(__MINGW64__)
	__asm__ __volatile__(
		"        pushq %%rbx        \n"
		"        xorq %%rcx,%%rcx   \n"
		"        cpuid              \n"
		"        movq %%rbx, %%rsi  \n"
		"        popq %%rbx         \n" :
		"=a" (regEAX), "=S" (regEBX), "=c" (regECX), "=d" (regEDX) : "a" (func))
#elif (defined(_MSC_VER) && defined(_M_IX86))
	__asm { 
	__asm mov eax, func 
	__asm xor ecx, ecx 
	__asm cpuid 
	__asm mov regEAX, eax
	__asm mov regEBX, ebx
	__asm mov regECX, ecx
	__asm mov regEDX, edx
	}
#elif defined(_MSC_VER) && defined(_M_X64)
		__cpuid(regs, func);
#else
	regEAX = regEBX = regECX = regEDX = 0;
#endif
	regs[_REG_EAX] = regEAX;
	regs[_REG_EBX] = regEBX;
	regs[_REG_ECX] = regECX;
	regs[_REG_EDX] = regEDX;
}

int btCpuInfo::HasCPUID(void)
{
	int has_CPUID = 0;
#if defined(__GNUC__) && defined(i386)
	__asm__(
		"        pushfl                      # Get original EFLAGS             \n"
		"        popl    %%eax                                                 \n"
		"        movl    %%eax,%%ecx                                           \n"
		"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
		"        pushl   %%eax               # Save new EFLAGS value on stack  \n"
		"        popfl                       # Replace current EFLAGS value    \n"
		"        pushfl                      # Get new EFLAGS                  \n"
		"        popl    %%eax               # Store new EFLAGS in EAX         \n"
		"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
		"        jz      1f                  # Processor=80486                 \n"
		"        movl    $1,%0               # We have CPUID support           \n"
		"1:                                                                    \n"
		: "=m" (has_CPUID)
		:
		: "%eax", "%ecx"
	);
#elif defined(__GNUC__) && defined(__x86_64__)
	/* Technically, if this is being compiled under __x86_64__ then it has
	CPUid by definition.  But it's nice to be able to prove it.  :)      */
	__asm__(
		"        pushfq                      # Get original EFLAGS             \n"
		"        popq    %%rax                                                 \n"
		"        movq    %%rax,%%rcx                                           \n"
		"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
		"        pushq   %%rax               # Save new EFLAGS value on stack  \n"
		"        popfq                       # Replace current EFLAGS value    \n"
		"        pushfq                      # Get new EFLAGS                  \n"
		"        popq    %%rax               # Store new EFLAGS in EAX         \n"
		"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
		"        jz      1f                  # Processor=80486                 \n"
		"        movl    $1,%0               # We have CPUID support           \n"
		"1:                                                                    \n"
		: "=m" (has_CPUID)
		:
		: "%rax", "%rcx"
	);
#elif (defined(_MSC_VER) && defined(_M_IX86))
	__asm {
		pushfd; Get original EFLAGS
		pop     eax
		mov     ecx, eax
		xor     eax, 200000h; Flip ID bit in EFLAGS
		push    eax; Save new EFLAGS value on stack
		popfd; Replace current EFLAGS value
		pushfd; Get new EFLAGS
		pop     eax; Store new EFLAGS in EAX
		xor     eax, ecx; Can not toggle ID bit,
		jz      done; Processor = 80486
		mov     has_CPUID, 1; We have CPUID support
		done :
	}
#elif defined(_MSC_VER) && defined(_M_X64)
	has_CPUID = 1;
#endif
	return has_CPUID;
}

bool btCpuInfo::IsINTEL(void)
{
	return false;
}

bool btCpuInfo::IsAMD(void)
{
	char pstring[16];
	char processorString[13];

	// get name of processor
	CpuId(0, (unsigned int *)pstring);
	processorString[0] = pstring[4];
	processorString[1] = pstring[5];
	processorString[2] = pstring[6];
	processorString[3] = pstring[7];
	processorString[4] = pstring[12];
	processorString[5] = pstring[13];
	processorString[6] = pstring[14];
	processorString[7] = pstring[15];
	processorString[8] = pstring[8];
	processorString[9] = pstring[9];
	processorString[10] = pstring[10];
	processorString[11] = pstring[11];
	processorString[12] = 0;

	if (strcmp(processorString, "AuthenticAMD") == 0) 
		return true;
	
	return false;
}

bool btCpuInfo::HasHTT(void)
{
	unsigned regs[4];

	// get CPU feature bits
	CpuId(1, regs);

	// bit 28 of EDX denotes HTT existence
	if (regs[_REG_EDX] & (HT_BIT))
		return true;

	return false;
}

bool btCpuInfo::HasCMOV(void)
{
	unsigned regs[4];

	// get CPU feature bits
	CpuId(1, regs);

	// bit 15 of EDX denotes CMOV existence
	if (regs[_REG_EDX] & (CMOV_BIT))
		return true;

	return false;
}

bool btCpuInfo::HasDAZ(void)
{
	return false;
}

unsigned char btCpuInfo::LogicalProcPerCPU(void)
{
	if (HasHTT())
	{
		unsigned regs[4];
	
		// get CPU feature bits
		CpuId(1, regs);
	
		return (unsigned char)((regs[_REG_EBX] & NUM_LOGICAL_BITS) >> 16);
	}
	else
		return (unsigned char)1;
}

static void set_MXCSR(int bitNum, int bitFlag)
{
#if defined(_MSC_VER) ||  defined(__GNUC__)
	_mm_setcsr((_mm_getcsr() & ~bitNum) | (bitFlag));
#endif
}

/*
===============================================================================
FPU
===============================================================================
*/
static void set_MXCSR_on(int bitNum)
{
	set_MXCSR(bitNum, bitNum);
}

static void set_MXCSR_off(int bitNum)
{
	set_MXCSR(bitNum, bitNum);
}

bool btFPUInfo::StackIsEmpty(void)
{
	return true;
}

void btFPUInfo::ClearStack(void)
{
}

const char * btFPUInfo::GetState(void)
{
	return "TODO Sys_FPU_GetState()";
}

void btFPUInfo::EnableExceptions(int exceptions)
{
}

void btFPUInfo::SetPrecision(int precision)
{
}
#define ID_USE_INSTRINSEC

void btFPUInfo::SetRounding(int rounding)
{
	short roundingBitTable[4] = { 0, 1, 2, 3 };
	short roundingBits = roundingBitTable[rounding & 3] << 10;

#ifdef ID_USE_INSTRINSEC
	_MM_SET_ROUNDING_MODE(roundingBits);
#else
	set_MXCSR(ROUND_BIT, roundingBits);
#endif
}

void btFPUInfo::SetFTZ(bool enable)
{
#ifdef ID_USE_INSTRINSEC
	int mode = _MM_GET_FLUSH_ZERO_MODE();
	if (enable && mode != _MM_FLUSH_ZERO_ON)
		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	else if (!enable && mode == _MM_FLUSH_ZERO_ON)
		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
#else	
	if (enable)
		set_MXCSR_on(FTZ_BIT);
	else
		set_MXCSR_off(FTZ_BIT);
#endif // !ID_USE_INSTRINSEC
}

void btFPUInfo::SetDAZ(bool enable)
{
#ifdef ID_USE_INSTRINSEC
	int mode = _MM_GET_DENORMALS_ZERO_MODE();
	if (enable && mode != _MM_DENORMALS_ZERO_ON)
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
	else if (!enable && mode == _MM_DENORMALS_ZERO_ON)
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
#else	
	if (enable)
		set_MXCSR_on(DAZ_BIT);
	else
		set_MXCSR_off(DAZ_BIT);
#endif // !ID_USE_INSTRINSEC
}
