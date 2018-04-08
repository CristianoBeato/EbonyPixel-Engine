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
#ifndef _SYS_SDL_CPU_H_
#define	_SYS_SDL_CPU_H_

#include <SDL_cpuinfo.h>

class btCpuInfo
{
public:
	const char* 	GetProcessorString(void);
	cpuid_t			GetCPUId(void);
	void			CPUCount(int& numLogicalCPUCores, int& numPhysicalCPUCores, int& numCPUPackages);

private:
	static void	CpuId(int func, unsigned regs[4]);
	static int	HasCPUID(void);
	static bool IsINTEL(void);
	static bool	IsAMD(void);
	static bool	HasHTT(void);
	static bool	HasCMOV(void);
	static bool	HasDAZ(void);
	static unsigned char LogicalProcPerCPU(void); // Return the number of logical processors per physical processors.
};

class btFPUInfo
{
public:
	bool			StackIsEmpty(void);						// returns true if the FPU stack is empty
	void			ClearStack(void);						// empties the FPU stack
	const char* 	GetState(void);						// returns the FPU state as a string
	void			EnableExceptions(int exceptions);	// enables the given FPU exceptions
	void			SetPrecision(int precision);		// sets the FPU precision
	void			SetRounding(int rounding);			// sets the FPU rounding mode
	void			SetFTZ(bool enable);				// sets Flush-To-Zero mode (only available when CPUID_FTZ is set)
	void			SetDAZ(bool enable);				// sets Denormals-Are-Zero mode (only available when CPUID_DAZ is set)

};

#endif // !_SYS_SDL_CPU_H_
